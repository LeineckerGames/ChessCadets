#include "BishopTutorial.h"
#include "TutorialBoard.h"
#include "ChessPiece.h"
#include "Engine/World.h"
#include "TimerManager.h"

ABishopTutorial::ABishopTutorial()
{
	TaughtPiece = EChessPieceType::Bishop;
}

// The bishop starts on a "light" square: (File + Rank) odd — same parity everywhere it can reach
bool ABishopTutorial::IsOwnColor(int32 File, int32 Rank) const
{
	return (File + Rank) % 2 != 0;
}

void ABishopTutorial::ApplyColorLock()
{
	if (!Board) return;

	for (int32 F = 0; F < Board->NumFiles; ++F)
	{
		for (int32 R = 0; R < Board->NumRanks; ++R)
		{
			Board->TintSquare(F, R, IsOwnColor(F, R) ? OwnColor : OffColor);
		}
	}
}

void ABishopTutorial::SetupPhase(ETutorialPhase Phase)
{
	Super::SetupPhase(Phase);

	for (AChessPiece* Decoy : DecoyActors)
		if (IsValid(Decoy)) Decoy->Destroy();
	DecoyActors.Empty();

	PaintedSquares.Empty();
	PaintedCount = 0;

	// Repaint the whole board each phase (also clears boss paint on reset)
	ApplyColorLock();

	switch (Phase)
	{
	case ETutorialPhase::Phase1: SetupPhase1(); break;
	case ETutorialPhase::Phase2: SetupPhase2(); break;
	case ETutorialPhase::Phase3: SetupPhase3(); break;
	case ETutorialPhase::Boss:   SetupBoss();   break;
	case ETutorialPhase::Complete: break;
	}

	UpdateHighlights();
}

// Phase 1: Targets on your own color — pure diagonal movement
void ABishopTutorial::SetupPhase1()
{
	SpawnPlayerPiece(1, 0);

	SpawnTargetAt(3, 2);
	SpawnTargetAt(5, 4);
	SpawnTargetAt(0, 3);

	ShowGuideMessage(TEXT("Bishops only move diagonally! Stay on your color and grab the orbs!"));
}

// Phase 2: Decoy targets on the wrong color — visibly out of reach
void ABishopTutorial::SetupPhase2()
{
	SpawnPlayerPiece(1, 0);

	SpawnTargetAt(0, 1);
	SpawnTargetAt(4, 1);
	SpawnTargetAt(2, 5);

	// Unreachable decoys on the purple squares
	SpawnDecoyAt(2, 2);
	SpawnDecoyAt(4, 4);

	ShowGuideMessage(TEXT("Some orbs are on purple squares — I can't reach those, they're not my color!"));
}

// Phase 3: Walls on the diagonals — plan a longer route
void ABishopTutorial::SetupPhase3()
{
	SpawnPlayerPiece(1, 0);

	SpawnTargetAt(2, 5);
	SpawnTargetAt(5, 4);

	SpawnWallAt(3, 2);
	SpawnWallAt(4, 1);

	ShowGuideMessage(TEXT("A wall on your diagonal! Find another diagonal around it!"));
}

// Boss: Paint the Board — cover green squares in a limited number of moves
void ABishopTutorial::SetupBoss()
{
	if (!Board) return;

	SpawnPlayerPiece(1, 0);
	PrevFile = 1;
	PrevRank = 0;
	PaintSquare(1, 0);

	ShowGuideMessage(FString::Printf(
		TEXT("BOSS: Paint the board! Slide across squares to paint them — you have %d moves!"), BossMoveLimit));
}

void ABishopTutorial::SpawnDecoyAt(int32 File, int32 Rank)
{
	if (!Board) return;

	FVector Location = Board->GridToWorldLocation(File, Rank, 5.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AChessPiece* Decoy = GetWorld()->SpawnActor<AChessPiece>(AChessPiece::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Decoy)
	{
		Decoy->Init(EChessPieceType::Pawn, EChessColor::Black, File, Rank, nullptr);
		Decoy->SetActorScale3D(FVector(0.6f));
		DecoyActors.Add(Decoy);
	}
}

void ABishopTutorial::PaintSquare(int32 File, int32 Rank)
{
	if (!Board) return;
	const int32 Key = PackPos(File, Rank);
	if (PaintedSquares.Contains(Key)) return;

	PaintedSquares.Add(Key);
	PaintedCount = PaintedSquares.Num();
	Board->TintSquare(File, Rank, PaintColor);
}

void ABishopTutorial::AfterPlayerMove(int32 File, int32 Rank)
{
	if (CurrentPhase != ETutorialPhase::Boss) return;
	if (!Board) return;

	// Paint every square along the slide, including the landing square
	const int32 FileStep = (File > PrevFile) ? 1 : -1;
	const int32 RankStep = (Rank > PrevRank) ? 1 : -1;
	int32 CF = PrevFile, CR = PrevRank;
	while (CF != File || CR != Rank)
	{
		CF += FileStep;
		CR += RankStep;
		PaintSquare(CF, CR);
	}
	PrevFile = File;
	PrevRank = Rank;

	const int32 TotalOwnSquares = (Board->NumFiles * Board->NumRanks) / 2;

	if (MoveCount >= BossMoveLimit)
	{
		ShowGuideMessage(FString::Printf(
			TEXT("Amazing! You painted %d of %d green squares! Diagonals are your domain!"),
			PaintedCount, TotalOwnSquares));
		CurrentPhase = ETutorialPhase::Complete;
		OnLevelComplete.Broadcast();
		return;
	}

	ShowGuideMessage(FString::Printf(TEXT("%d squares painted — %d moves left!"),
		PaintedCount, BossMoveLimit - MoveCount));
	UpdateHighlights();
}

void ABishopTutorial::OnTargetCollected(int32 File, int32 Rank)
{
	if (TargetPositions.Num() > 0)
		ShowGuideMessage(FString::Printf(TEXT("Got it! %d to go!"), TargetPositions.Num()));
}

void ABishopTutorial::OnPhaseComplete()
{
	if (CurrentPhase == ETutorialPhase::Boss)
	{
		// Boss end is handled in AfterPlayerMove — don't auto-advance
		return;
	}

	if (CurrentPhase == ETutorialPhase::Phase2)
	{
		ShowGuideMessage(TEXT("See? Bishops stay on one color forever. That's their secret!"));
	}
	else if (CurrentPhase == ETutorialPhase::Phase3)
	{
		ShowGuideMessage(TEXT("You found another diagonal! Clever!"));
	}

	Super::OnPhaseComplete();
}
