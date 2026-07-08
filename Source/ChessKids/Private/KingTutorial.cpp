#include "KingTutorial.h"
#include "TutorialBoard.h"
#include "ChessPiece.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "ChessKidsGameInstance.h"

AKingTutorial::AKingTutorial()
{
	TaughtPiece = EChessPieceType::King;
}

void AKingTutorial::SetupPhase(ETutorialPhase Phase)
{
	Super::SetupPhase(Phase);

	GetWorldTimerManager().ClearTimer(RecapTimer);
	if (RecapPiece) { RecapPiece->Destroy(); RecapPiece = nullptr; }
	DangerSquares.Empty();
	TaughtPiece = EChessPieceType::King; // recap changes it temporarily — always restore

	// Reset board colors
	if (Board)
	{
		for (int32 F = 0; F < Board->NumFiles; ++F)
			for (int32 R = 0; R < Board->NumRanks; ++R)
				Board->TintSquare(F, R, ((F + R) % 2 != 0) ? Board->LightSquareColor : Board->DarkSquareColor);
	}

	switch (Phase)
	{
	case ETutorialPhase::Phase1: SetupMaze();  break;
	case ETutorialPhase::Phase2: SetupRecap(); break;
	case ETutorialPhase::Phase3:
		// This level has no phase 3 — go straight to the boss
		CurrentPhase = ETutorialPhase::Boss;
		SetupBoss();
		break;
	case ETutorialPhase::Boss:   SetupBoss();  break;
	case ETutorialPhase::Complete: break;
	}

	UpdateHighlights();
}

void AKingTutorial::AddDangerAt(int32 File, int32 Rank)
{
	if (!Board || !Board->IsValidSquare(File, Rank)) return;
	DangerSquares.Add(PackPos(File, Rank));
	Board->TintSquare(File, Rank, DangerColor);
}

// Phase 1: King Maze — one step at a time, red squares are lava.
// Serpentine walls generated from the board size, so any grid (6x6, 10x10, 11x11...) works:
// every other column is a danger wall with a single gap, alternating top/bottom.
void AKingTutorial::SetupMaze()
{
	SpawnPlayerPiece(0, 0);

	const int32 NF = Board ? Board->NumFiles : 6;
	const int32 NR = Board ? Board->NumRanks : 6;

	int32 WallIndex = 0;
	for (int32 C = 1; C < NF - 1; C += 2, ++WallIndex)
	{
		const int32 GapRank = (WallIndex % 2 == 0) ? NR - 1 : 0;
		for (int32 R = 0; R < NR; ++R)
		{
			if (R == GapRank) continue;
			AddDangerAt(C, R);
		}
	}

	SpawnTargetAt(NF - 1, NR - 1);

	ShowGuideMessage(TEXT("The king moves ONE square, any direction. Red squares are danger — find the safe path!"));
}

// Phase 2: Recap — each friend shows their move one last time
void AKingTutorial::SetupRecap()
{
	RecapIndex = 0;
	bPhaseTransitionPending = true; // no clicking during the parade
	ShowGuideMessage(TEXT("Your friends are here! One last look at how everyone moves..."));
	GetWorldTimerManager().SetTimer(RecapTimer, this, &AKingTutorial::ShowNextRecapPiece, RecapIntroSeconds, false);
}

void AKingTutorial::ShowNextRecapPiece()
{
	if (!Board) return;

	static const EChessPieceType RecapOrder[] = {
		EChessPieceType::Pawn, EChessPieceType::Rook, EChessPieceType::Bishop,
		EChessPieceType::Knight, EChessPieceType::Queen
	};
	static const TCHAR* RecapLines[] = {
		TEXT("Marco the Pawn: I march forward and strike diagonally!"),
		TEXT("Tara the Rook: straight lines, as far as I want!"),
		TEXT("Zig the Bishop: diagonals only — I never leave my color!"),
		TEXT("Luna the Knight: L-hops! I jump over ANYTHING!"),
		TEXT("Nova the Queen: straight AND diagonal. All of it. Mine.")
	};
	constexpr int32 RecapCount = 5;

	if (RecapPiece) { RecapPiece->Destroy(); RecapPiece = nullptr; }

	if (RecapIndex >= RecapCount)
	{
		// Parade over — restore the king and head to the boss
		TaughtPiece = EChessPieceType::King;
		Board->ClearHighlights();
		OnPhaseComplete();
		return;
	}

	const int32 CenterF = Board->NumFiles / 2;
	const int32 CenterR = Board->NumRanks / 2;

	FVector Location = Board->GridToWorldLocation(CenterF, CenterR, 5.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	RecapPiece = GetWorld()->SpawnActor<AChessPiece>(AChessPiece::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (RecapPiece)
		RecapPiece->Init(RecapOrder[RecapIndex], EChessColor::White, CenterF, CenterR, nullptr);

	// Show this piece's movement pattern from the center of the board
	TaughtPiece = RecapOrder[RecapIndex];
	PlayerFile = CenterF;
	PlayerRank = CenterR;
	UpdateHighlights();

	ShowGuideMessage(RecapLines[RecapIndex]);

	RecapIndex++;
	GetWorldTimerManager().SetTimer(RecapTimer, this, &AKingTutorial::ShowNextRecapPiece, RecapSecondsPerPiece, false);
}

// Boss: AXIOM — hand off to the real chess match
void AKingTutorial::SetupBoss()
{
	SpawnPlayerPiece(2, 0);

	ShowGuideMessage(TEXT("This is it. AXIOM awaits. Your friends become your army — go free Neo City!"));

	bPhaseTransitionPending = true;
	GetWorldTimerManager().SetTimer(RecapTimer, this, &AKingTutorial::StartAxiomBattle, 4.f, false);
}

void AKingTutorial::StartAxiomBattle()
{
	if (AxiomMapName != NAME_None)
	{
		// AXIOM plays on Easy and it's a solo game — the kid is meant to win
		if (UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			GI->Difficulty = 1;
			GI->bTwoPlayerMode = false;
		}
		UGameplayStatics::OpenLevel(this, AxiomMapName);
		return;
	}

	// No battle map wired up yet — end the tutorial gracefully
	ShowGuideMessage(TEXT("(AXIOM battle map not set — assign 'Axiom Map Name' on the KingTutorial actor.)"));
	CurrentPhase = ETutorialPhase::Complete;
	OnLevelComplete.Broadcast();
}

bool AKingTutorial::IsMoveAllowed(int32 FromFile, int32 FromRank, int32 ToFile, int32 ToRank) const
{
	// Danger squares are never legal — the maze shows safety by what glows
	if (CurrentPhase == ETutorialPhase::Phase1 && DangerSquares.Contains(PackPos(ToFile, ToRank)))
		return false;

	return Super::IsMoveAllowed(FromFile, FromRank, ToFile, ToRank);
}

void AKingTutorial::OnTargetCollected(int32 File, int32 Rank)
{
	if (CurrentPhase == ETutorialPhase::Phase1)
		ShowGuideMessage(TEXT("You made it through! Careful and steady — that's a king!"));
}

void AKingTutorial::OnPhaseComplete()
{
	if (CurrentPhase == ETutorialPhase::Boss)
		return;

	Super::OnPhaseComplete();
}
