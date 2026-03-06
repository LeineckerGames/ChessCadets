#include "ChessManager.h"
#include "ChessBoard.h"
#include "ChessPiece.h"
#include "Async/Async.h"

THIRD_PARTY_INCLUDES_START
#include "position.h"
#include "search.h"
#include "notation.h"
#include "movegenerator.h"
#include "movelist.h"
#include "protocol.h"
#include "model/move.h"
#include "model/piece.h"
#include "model/piecetype.h"
#include "model/square.h"
#include "model/color.h"
#include "model/file.h"
#include "model/rank.h"
#include "model/movetype.h"
THIRD_PARTY_INCLUDES_END

struct AChessManager::FEngineImpl : public pulse::Protocol
{
	pulse::Position Position;
	pulse::Search   Search;

	TFunction<void(int)> BestMoveCallback;

	FEngineImpl()
		: Position(pulse::notation::toPosition(pulse::notation::STANDARDPOSITION))
		, Search(*this)
	{}

	void sendBestMove(int bestMove, int ponderMove) override
	{
		if (BestMoveCallback)
			BestMoveCallback(bestMove);
	}

	void sendStatus(int, int, uint64_t, int, int) override {}
	void sendStatus(bool, int, int, uint64_t, int, int) override {}
	void sendMove(pulse::RootEntry, int, int, uint64_t) override {}
	void sendInfo(const std::string&) override {}
	void sendDebug(const std::string&) override {}
};

int AChessManager::SquareFromString(const FString& Str)
{
	if (Str.Len() < 2) return -1;

	const char FileChar = (char)FChar::ToLower(Str[0]);
	const char RankChar = (char)Str[1];

	if (FileChar < 'a' || FileChar > 'h') return -1;
	if (RankChar < '1' || RankChar > '8') return -1;

	const int File = FileChar - 'a';
	const int Rank = RankChar - '1';
	return pulse::square::valueOf(File, Rank);
}

FString AChessManager::SquareToString(int Square)
{
	if (!pulse::square::isValid(Square)) return TEXT("??");
	const int File = pulse::square::getFile(Square);
	const int Rank = pulse::square::getRank(Square);
	return FString::Printf(TEXT("%c%c"), 'a' + File, '1' + Rank);
}

static const TCHAR* PieceToChar(int Piece)
{
	switch (Piece)
	{
	case pulse::piece::WHITE_PAWN:   return TEXT("P");
	case pulse::piece::WHITE_KNIGHT: return TEXT("N");
	case pulse::piece::WHITE_BISHOP: return TEXT("B");
	case pulse::piece::WHITE_ROOK:   return TEXT("R");
	case pulse::piece::WHITE_QUEEN:  return TEXT("Q");
	case pulse::piece::WHITE_KING:   return TEXT("K");
	case pulse::piece::BLACK_PAWN:   return TEXT("p");
	case pulse::piece::BLACK_KNIGHT: return TEXT("n");
	case pulse::piece::BLACK_BISHOP: return TEXT("b");
	case pulse::piece::BLACK_ROOK:   return TEXT("r");
	case pulse::piece::BLACK_QUEEN:  return TEXT("q");
	case pulse::piece::BLACK_KING:   return TEXT("k");
	default:                          return TEXT("");
	}
}

AChessManager::AChessManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AChessManager::BeginPlay()
{
	Super::BeginPlay();
	NewGame();
	SpawnAllPieces();

	OnMoveMade.AddDynamic(this, &AChessManager::HandleMoveMade);
}

void AChessManager::BeginDestroy()
{
	if (Engine)
	{
		Engine->Search.quit();
		delete Engine;
		Engine = nullptr;
	}
	Super::BeginDestroy();
}

void AChessManager::NewGame()
{
	if (Engine)
	{
		Engine->Search.quit();
		delete Engine;
	}

	Engine = new FEngineImpl();

	Engine->BestMoveCallback = [this](int BestMove)
	{
		AsyncTask(ENamedThreads::GameThread, [this, BestMove]()
		{
			if (!IsValid(this)) return;
			OnBestMoveFound(BestMove);
		});
	};
}

void AChessManager::SetPositionFromFEN(const FString& FEN)
{
	if (!Engine) NewGame();

	const std::string FENStd(TCHAR_TO_UTF8(*FEN));
	Engine->Position = pulse::notation::toPosition(FENStd);
}

bool AChessManager::MakeMove(const FString& MoveStr)
{
	if (!Engine) return false;

	if (MoveStr.Len() < 4) return false;

	const FString OriginStr   = MoveStr.Mid(0, 2);
	const FString TargetStr   = MoveStr.Mid(2, 2);
	const FString PromoStr    = MoveStr.Len() >= 5 ? MoveStr.Mid(4, 1) : TEXT("");

	const int OriginSquare = SquareFromString(OriginStr);
	const int TargetSquare = SquareFromString(TargetStr);
	if (OriginSquare < 0 || TargetSquare < 0) return false;

	pulse::MoveGenerator Gen;
	auto& LegalMoves = Gen.getLegalMoves(
		Engine->Position, 1, Engine->Position.isCheck());

	for (int i = 0; i < LegalMoves.size; i++)
	{
		const int M = LegalMoves.entries[i]->move;
		if (pulse::move::getOriginSquare(M) != OriginSquare) continue;
		if (pulse::move::getTargetSquare(M) != TargetSquare) continue;

		if (!PromoStr.IsEmpty())
		{
			const char PromoChar = (char)FChar::ToLower(PromoStr[0]);
			int ExpectedPromo = pulse::piecetype::NOPIECETYPE;
			switch (PromoChar)
			{
			case 'q': ExpectedPromo = pulse::piecetype::QUEEN;  break;
			case 'r': ExpectedPromo = pulse::piecetype::ROOK;   break;
			case 'b': ExpectedPromo = pulse::piecetype::BISHOP; break;
			case 'n': ExpectedPromo = pulse::piecetype::KNIGHT; break;
			}
			if (pulse::move::getPromotion(M) != ExpectedPromo) continue;
		}

		Engine->Position.makeMove(M);

		const FString From = SquareToString(OriginSquare);
		const FString To   = SquareToString(TargetSquare);
		OnMoveMade.Broadcast(From, To);

		CheckGameOver();
		return true;
	}

	return false;
}

void AChessManager::RequestAIMove()
{
	if (!Engine) return;

	Engine->Search.newDepthSearch(Engine->Position, AISearchDepth);
	Engine->Search.start();
}

void AChessManager::StopSearch()
{
	if (Engine) Engine->Search.stop();
}

FString AChessManager::GetFEN() const
{
	if (!Engine) return TEXT("");
	return FString(pulse::notation::fromPosition(Engine->Position).c_str());
}

FString AChessManager::GetActiveColor() const
{
	if (!Engine) return TEXT("white");
	return Engine->Position.activeColor == pulse::color::WHITE
		? TEXT("white") : TEXT("black");
}

TArray<FString> AChessManager::GetLegalMovesFromSquare(const FString& SquareStr) const
{
	TArray<FString> Result;
	if (!Engine) return Result;

	const int OriginSquare = SquareFromString(SquareStr);
	if (OriginSquare < 0) return Result;

	pulse::MoveGenerator Gen;
	auto& LegalMoves = Gen.getLegalMoves(
		Engine->Position, 1, Engine->Position.isCheck());

	for (int i = 0; i < LegalMoves.size; i++)
	{
		const int M = LegalMoves.entries[i]->move;
		if (pulse::move::getOriginSquare(M) == OriginSquare)
			Result.AddUnique(SquareToString(pulse::move::getTargetSquare(M)));
	}

	return Result;
}

bool AChessManager::IsInCheck() const
{
	return Engine ? Engine->Position.isCheck() : false;
}

FString AChessManager::GetPieceOnSquare(const FString& SquareStr) const
{
	if (!Engine) return TEXT("");
	const int Sq = SquareFromString(SquareStr);
	if (Sq < 0) return TEXT("");
	return PieceToChar(Engine->Position.board[Sq]);
}

void AChessManager::OnBestMoveFound(int BestMove)
{
	if (!Engine) return;
	if (BestMove == pulse::move::NOMOVE) return;

	const FString From = SquareToString(pulse::move::getOriginSquare(BestMove));
	const FString To   = SquareToString(pulse::move::getTargetSquare(BestMove));

	Engine->Position.makeMove(BestMove);
	OnMoveMade.Broadcast(From, To);
	OnAIMoveReady.Broadcast(From, To);

	CheckGameOver();
}

void AChessManager::CheckGameOver()
{
	if (!Engine) return;

	pulse::MoveGenerator Gen;
	auto& LegalMoves = Gen.getLegalMoves(
		Engine->Position, 1, Engine->Position.isCheck());

	if (LegalMoves.size == 0)
	{
		if (Engine->Position.isCheck())
		{
			const FString Winner = Engine->Position.activeColor == pulse::color::WHITE
				? TEXT("black") : TEXT("white");
			OnGameOver.Broadcast(Winner);
		}
		else
		{
			OnGameOver.Broadcast(TEXT("draw"));
		}
		return;
	}

	if (Engine->Position.isRepetition() || Engine->Position.hasInsufficientMaterial())
		OnGameOver.Broadcast(TEXT("draw"));
}

EChessPieceType AChessManager::CharToPieceType(TCHAR C)
{
	switch (FChar::ToLower(C))
	{
	case 'p': return EChessPieceType::Pawn;
	case 'n': return EChessPieceType::Knight;
	case 'b': return EChessPieceType::Bishop;
	case 'r': return EChessPieceType::Rook;
	case 'q': return EChessPieceType::Queen;
	case 'k': return EChessPieceType::King;
	default:  return EChessPieceType::Pawn;
	}
}

void AChessManager::SpawnAllPieces()
{
	for (auto& Pair : PieceActors)
		if (IsValid(Pair.Value))
			Pair.Value->Destroy();
	PieceActors.Empty();

	if (!Board) return;

	for (int32 Rank = 0; Rank < 8; ++Rank)
	{
		for (int32 File = 0; File < 8; ++File)
		{
			FString SquareName = FString::Printf(TEXT("%c%c"), 'a' + File, '1' + Rank);
			FString PieceStr = GetPieceOnSquare(SquareName);
			if (PieceStr.IsEmpty()) continue;

			TCHAR PieceChar = PieceStr[0];
			EChessColor Color = FChar::IsUpper(PieceChar) ? EChessColor::White : EChessColor::Black;
			EChessPieceType Type = CharToPieceType(PieceChar);

			FVector SpawnLoc = Board->FileRankToWorldLocation(File, Rank, 0.f);
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AChessPiece* Piece = GetWorld()->SpawnActor<AChessPiece>(SpawnLoc, FRotator::ZeroRotator, Params);
			if (Piece)
			{
				Piece->Init(Type, Color, File, Rank);
				Board->SnapActorToSquare(Piece, File, Rank, 0.f);
				PieceActors.Add(SquareName, Piece);
			}
		}
	}
}

AChessPiece* AChessManager::FindPieceOnSquare(const FString& Square) const
{
	const auto* Found = PieceActors.Find(Square);
	return Found ? *Found : nullptr;
}

void AChessManager::HandleMoveMade(FString From, FString To)
{
	if (!Board) return;

	AChessPiece* Captured = FindPieceOnSquare(To);
	if (Captured)
	{
		Captured->Destroy();
		PieceActors.Remove(To);
	}

	AChessPiece* MovingPiece = FindPieceOnSquare(From);
	if (MovingPiece)
	{
		PieceActors.Remove(From);

		int32 File = To[0] - 'a';
		int32 Rank = To[1] - '1';
		Board->SnapActorToSquare(MovingPiece, File, Rank, 0.f);
		MovingPiece->BoardFile = File;
		MovingPiece->BoardRank = Rank;

		PieceActors.Add(To, MovingPiece);
	}

	if (MovingPiece && MovingPiece->PieceType == EChessPieceType::King)
	{
		int32 FromFile = From[0] - 'a';
		int32 ToFile = To[0] - 'a';
		int32 RankNum = To[1] - '1';

		if (FMath::Abs(ToFile - FromFile) == 2)
		{
			if (ToFile > FromFile)
			{
				FString RookFrom = FString::Printf(TEXT("h%c"), '1' + RankNum);
				FString RookTo   = FString::Printf(TEXT("f%c"), '1' + RankNum);
				AChessPiece* Rook = FindPieceOnSquare(RookFrom);
				if (Rook)
				{
					PieceActors.Remove(RookFrom);
					Board->SnapActorToSquare(Rook, 5, RankNum, 0.f);
					Rook->BoardFile = 5;
					Rook->BoardRank = RankNum;
					PieceActors.Add(RookTo, Rook);
				}
			}
			else
			{
				FString RookFrom = FString::Printf(TEXT("a%c"), '1' + RankNum);
				FString RookTo   = FString::Printf(TEXT("d%c"), '1' + RankNum);
				AChessPiece* Rook = FindPieceOnSquare(RookFrom);
				if (Rook)
				{
					PieceActors.Remove(RookFrom);
					Board->SnapActorToSquare(Rook, 3, RankNum, 0.f);
					Rook->BoardFile = 3;
					Rook->BoardRank = RankNum;
					PieceActors.Add(RookTo, Rook);
				}
			}
		}
	}
}
