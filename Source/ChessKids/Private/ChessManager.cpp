// ChessManager.cpp

#include "ChessManager.h"
#include "Async/Async.h"

// ---------------------------------------------------------------
// Pulse engine headers (STL-heavy, kept out of the public header)
// ---------------------------------------------------------------
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

// ---------------------------------------------------------------
// PImpl — holds all engine state and implements pulse::Protocol
// ---------------------------------------------------------------
struct AChessManager::FEngineImpl : public pulse::Protocol
{
	pulse::Position Position;
	pulse::Search   Search;

	// Set by AChessManager so the callback can fire the delegate
	TFunction<void(int /*bestMove*/)> BestMoveCallback;

	FEngineImpl()
		: Position(pulse::notation::toPosition(pulse::notation::STANDARDPOSITION))
		, Search(*this)
	{}

	// pulse::Protocol overrides
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

// ---------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------

// "e2" -> engine square index (file 0-7, rank 0-7, square = file + rank*16)
int AChessManager::SquareFromString(const FString& Str)
{
	if (Str.Len() < 2) return -1;

	const char FileChar = (char)FChar::ToLower(Str[0]);
	const char RankChar = (char)Str[1];

	if (FileChar < 'a' || FileChar > 'h') return -1;
	if (RankChar < '1' || RankChar > '8') return -1;

	const int File = FileChar - 'a';   // 0-7
	const int Rank = RankChar - '1';   // 0-7
	return pulse::square::valueOf(File, Rank);
}

// Engine square index -> "e2"
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

// ---------------------------------------------------------------
// AChessManager
// ---------------------------------------------------------------

AChessManager::AChessManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AChessManager::BeginPlay()
{
	Super::BeginPlay();
	NewGame();
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

	// Wire up the best-move callback to marshal onto the game thread
	Engine->BestMoveCallback = [this](int BestMove)
	{
		// Search runs on its own thread — dispatch back to game thread
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

	// Parse the move string (e.g. "e2e4" or "e7e8q")
	if (MoveStr.Len() < 4) return false;

	const FString OriginStr   = MoveStr.Mid(0, 2);
	const FString TargetStr   = MoveStr.Mid(2, 2);
	const FString PromoStr    = MoveStr.Len() >= 5 ? MoveStr.Mid(4, 1) : TEXT("");

	const int OriginSquare = SquareFromString(OriginStr);
	const int TargetSquare = SquareFromString(TargetStr);
	if (OriginSquare < 0 || TargetSquare < 0) return false;

	// Find a matching legal move
	pulse::MoveGenerator Gen;
	auto& LegalMoves = Gen.getLegalMoves(
		Engine->Position, 1, Engine->Position.isCheck());

	for (int i = 0; i < LegalMoves.size; i++)
	{
		const int M = LegalMoves.entries[i]->move;
		if (pulse::move::getOriginSquare(M) != OriginSquare) continue;
		if (pulse::move::getTargetSquare(M) != TargetSquare) continue;

		// Check promotion piece if provided
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

		// Legal — apply it
		Engine->Position.makeMove(M);

		const FString From = SquareToString(OriginSquare);
		const FString To   = SquareToString(TargetSquare);
		OnMoveMade.Broadcast(From, To);

		CheckGameOver();
		return true;
	}

	return false; // Illegal move
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
		// No legal moves — checkmate or stalemate
		if (Engine->Position.isCheck())
		{
			// The side that just moved wins
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
