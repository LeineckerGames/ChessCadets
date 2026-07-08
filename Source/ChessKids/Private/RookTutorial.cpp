#include "RookTutorial.h"
#include "TutorialBoard.h"
#include "ChessPiece.h"
#include "Engine/World.h"
#include "TimerManager.h"

ARookTutorial::ARookTutorial()
{
	TaughtPiece = EChessPieceType::Rook;
}

void ARookTutorial::SetupPhase(ETutorialPhase Phase)
{
	Super::SetupPhase(Phase);

	if (Runner) { Runner->Destroy(); Runner = nullptr; }

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

// Phase 1: Orbs in a single row — slide sideways to collect
//   Col:  0  1  2  3  4  5
//   R2:   P  .  O  .  O  O
void ARookTutorial::SetupPhase1()
{
	SpawnPlayerPiece(0, 2);

	SpawnTargetAt(2, 2);
	SpawnTargetAt(4, 2);
	SpawnTargetAt(5, 2);

	ShowGuideMessage(TEXT("Rooks slide in straight lines! Slide across to grab the orbs!"));
}

// Phase 2: Orbs in rows AND columns — mix horizontal and vertical slides
//   R4:   O  .  .  O  .  .
//   ...
//   R0:   P  .  .  O  .  .
void ARookTutorial::SetupPhase2()
{
	SpawnPlayerPiece(0, 0);

	SpawnTargetAt(3, 0);
	SpawnTargetAt(3, 4);
	SpawnTargetAt(0, 4);

	ShowGuideMessage(TEXT("Up, down, left, right — as far as you want!"));
}

// Phase 3: Walls block the way — find a path around
//   R5:   .  .  .  .  .  O
//   R3:   .  .  .  .  .  W
//   R2:   .  .  W  .  .  .
//   R0:   P  .  .  W  .  O
void ARookTutorial::SetupPhase3()
{
	SpawnPlayerPiece(0, 0);

	SpawnTargetAt(5, 0);
	SpawnTargetAt(5, 5);

	SpawnWallAt(3, 0);
	SpawnWallAt(2, 2);
	SpawnWallAt(5, 3);

	ShowGuideMessage(TEXT("Rooks can't go through walls — find another way!"));
}

// Boss: Catch the Runner — a fleeing target moves one square per turn
void ARookTutorial::SetupBoss()
{
	if (!Board) return;

	SpawnPlayerPiece(0, 0);

	RunnerFile = Board->NumFiles - 1;
	RunnerRank = Board->NumRanks - 1;

	FVector Location = Board->GridToWorldLocation(RunnerFile, RunnerRank, 5.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Runner = GetWorld()->SpawnActor<AChessPiece>(AChessPiece::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Runner)
	{
		Runner->Init(EChessPieceType::Pawn, EChessColor::Black, RunnerFile, RunnerRank, nullptr);
		Runner->SetActorScale3D(FVector(0.8f));
	}

	ShowGuideMessage(FString::Printf(TEXT("BOSS: Catch the runner! Land on its square — you have %d moves!"), BossMoveLimit));
}

void ARookTutorial::AfterPlayerMove(int32 File, int32 Rank)
{
	if (CurrentPhase != ETutorialPhase::Boss) return;
	if (!Board) return;

	// Caught the runner?
	if (Runner && File == RunnerFile && Rank == RunnerRank)
	{
		Runner->Destroy();
		Runner = nullptr;
		ShowGuideMessage(TEXT("You caught the runner! Straight lines, unstoppable!"));
		CurrentPhase = ETutorialPhase::Complete;
		OnLevelComplete.Broadcast();
		return;
	}

	MoveRunner();

	// Out of moves?
	if (MoveCount >= BossMoveLimit)
	{
		ShowGuideMessage(TEXT("The runner escaped! Try again — trap it against the edge!"));
		bPhaseTransitionPending = true;
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ATutorialManager::ResetPhase, 2.0f, false);
		return;
	}

	ShowGuideMessage(FString::Printf(TEXT("%d moves left!"), BossMoveLimit - MoveCount));
	UpdateHighlights();
}

void ARookTutorial::MoveRunner()
{
	if (!Runner || !Board) return;

	// Try the 4 orthogonal steps; pick the one furthest from the player
	const int32 DF[4] = { 1, -1, 0, 0 };
	const int32 DR[4] = { 0, 0, 1, -1 };

	int32 BestFile = RunnerFile, BestRank = RunnerRank, BestDist = -1;
	for (int32 i = 0; i < 4; ++i)
	{
		const int32 NF = RunnerFile + DF[i];
		const int32 NR = RunnerRank + DR[i];
		if (!Board->IsValidSquare(NF, NR)) continue;
		if (IsSquareBlocked(NF, NR)) continue;
		if (NF == PlayerFile && NR == PlayerRank) continue;

		const int32 Dist = FMath::Abs(NF - PlayerFile) + FMath::Abs(NR - PlayerRank);
		if (Dist > BestDist)
		{
			BestDist = Dist;
			BestFile = NF;
			BestRank = NR;
		}
	}

	RunnerFile = BestFile;
	RunnerRank = BestRank;
	Board->GlideActorToSquare(Runner, RunnerFile, RunnerRank, 5.f);
	Runner->BoardFile = RunnerFile;
	Runner->BoardRank = RunnerRank;
}

void ARookTutorial::OnTargetCollected(int32 File, int32 Rank)
{
	if (TargetPositions.Num() > 0)
		ShowGuideMessage(FString::Printf(TEXT("Orb collected! %d to go!"), TargetPositions.Num()));
}

void ARookTutorial::OnPhaseComplete()
{
	if (CurrentPhase == ETutorialPhase::Boss)
	{
		// Boss win is handled in AfterPlayerMove — don't auto-advance
		return;
	}

	if (CurrentPhase == ETutorialPhase::Phase3)
	{
		ShowGuideMessage(TEXT("You found the way around! Nothing stops a clever rook!"));
	}

	Super::OnPhaseComplete();
}
