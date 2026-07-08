#include "PawnTutorial.h"
#include "TutorialBoard.h"
#include "ChessPiece.h"
#include "Engine/World.h"
#include "TimerManager.h"

APawnTutorial::APawnTutorial()
{
	TaughtPiece = EChessPieceType::Pawn;
}

bool APawnTutorial::IsSquareOccupied(int32 File, int32 Rank) const
{
	if (File == PlayerFile && Rank == PlayerRank) return true;
	if (File == AIPawnFile && Rank == AIPawnRank && AIPawn) return true;
	return EnemyActors.Contains(PackPos(File, Rank)) || TargetActors.Contains(PackPos(File, Rank));
}

void APawnTutorial::SetupPhase(ETutorialPhase Phase)
{
	Super::SetupPhase(Phase);

	if (AIPawn) { AIPawn->Destroy(); AIPawn = nullptr; }

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

// Phase 1: Empty path — just move forward one square at a time
void APawnTutorial::SetupPhase1()
{
	SpawnPlayerPiece(1, 0); // start at bottom, column 1 (of 4)

	// Place a target at the end of the path
	SpawnTargetAt(1, 7);

	ShowGuideMessage(TEXT("Pawns only go forward! Click the glowing square to move."));
}

// Phase 2: First move lets you go two squares
void APawnTutorial::SetupPhase2()
{
	SpawnPlayerPiece(2, 0);

	// Targets spread further apart to encourage the two-square first move
	SpawnTargetAt(2, 2);
	SpawnTargetAt(2, 5);
	SpawnTargetAt(2, 7);

	ShowGuideMessage(TEXT("On your first step, you can sprint two squares!"));
}

// Phase 3: Diagonal capture gauntlet — zigzag 6 captures
// If player goes forward instead, soft-lock resets with hint
//   Col: 0  1  2  3
//   R0:  .  .  P  .    <- player starts at (2,0)
//   R1:  .  E  .  .    <- capture diag-left → (1,1)
//   R2:  .  .  E  .    <- capture diag-right → (2,2)
//   R3:  .  E  .  .    <- capture diag-left → (1,3)
//   R4:  .  .  E  .    <- capture diag-right → (2,4)
//   R5:  .  E  .  .    <- capture diag-left → (1,5)
//   R6:  .  .  E  .    <- capture diag-right → (2,6) → done!
void APawnTutorial::SetupPhase3()
{
	SpawnPlayerPiece(2, 0);

	SpawnEnemyAt(1, 1);
	SpawnEnemyAt(2, 2);
	SpawnEnemyAt(1, 3);
	SpawnEnemyAt(2, 4);
	SpawnEnemyAt(1, 5);
	SpawnEnemyAt(2, 6);

	ShowGuideMessage(TEXT("Pawns capture DIAGONALLY! Zigzag through all the enemies!"));
}

// Boss: Pawn Race — player and AI pawn race to the other side
void APawnTutorial::SetupBoss()
{
	if (!Board) return;

	SpawnPlayerPiece(1, 0);

	// AI pawn on adjacent column, starts mid-board so it's visible
	AIPawnFile = 2;
	AIPawnRank = 5;

	FVector AILocation = Board->GridToWorldLocation(AIPawnFile, AIPawnRank, 5.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AIPawn = GetWorld()->SpawnActor<AChessPiece>(AChessPiece::StaticClass(), AILocation, FRotator::ZeroRotator, Params);
	if (AIPawn)
	{
		AIPawn->Init(EChessPieceType::Pawn, EChessColor::Black, AIPawnFile, AIPawnRank, nullptr);
		AIPawn->SetActorScale3D(FVector(1.2f));
	}

	BossMoveLimit = 20;

	// Target at finish line — collect to win, also prevents base-class auto-complete
	SpawnTargetAt(1, 7);

	ShowGuideMessage(TEXT("BOSS: Race to the top! The enemy pawn is coming toward you — dodge or capture it!"));
}

bool APawnTutorial::IsMoveAllowed(int32 FromFile, int32 FromRank, int32 ToFile, int32 ToRank) const
{
	if (CurrentPhase == ETutorialPhase::Boss && AIPawn)
	{
		const int32 FileDiff = FMath::Abs(ToFile - FromFile);
		const int32 RankDiff = ToRank - FromRank;

		// Diagonal capture of the AI pawn
		if (FileDiff == 1 && RankDiff == 1 && ToFile == AIPawnFile && ToRank == AIPawnRank)
			return true;

		// The AI pawn blocks forward movement — can't land on it or jump over it
		if (FileDiff == 0 && ToFile == AIPawnFile)
		{
			if (ToRank == AIPawnRank) return false;
			if (RankDiff == 2 && FromRank + 1 == AIPawnRank) return false;
		}
	}

	return Super::IsMoveAllowed(FromFile, FromRank, ToFile, ToRank);
}

void APawnTutorial::AfterPlayerMove(int32 File, int32 Rank)
{
	if (CurrentPhase != ETutorialPhase::Boss) return;
	if (!Board) return;

	// Check if player captured the AI pawn (diagonal capture onto its square)
	if (AIPawn && File == AIPawnFile && Rank == AIPawnRank)
	{
		AIPawn->Destroy();
		AIPawn = nullptr;
		ShowGuideMessage(TEXT("You captured the enemy pawn! You win the race!"));
		CurrentPhase = ETutorialPhase::Complete;
		OnLevelComplete.Broadcast();
		return;
	}

	// Check if player reached the finish line
	if (Rank >= Board->NumRanks - 1)
	{
		if (AIPawn) { AIPawn->Destroy(); AIPawn = nullptr; }
		ShowGuideMessage(TEXT("You won the race! You've mastered the pawn!"));
		CurrentPhase = ETutorialPhase::Complete;
		OnLevelComplete.Broadcast();
		return;
	}

	// AI takes its turn
	MoveAIPawn();

	// After AI moves, update highlights so player sees new legal moves
	UpdateHighlights();
}

void APawnTutorial::MoveAIPawn()
{
	if (!AIPawn || !Board) return;

	// AI pawn moves down one rank (toward rank 0)
	int32 NewRank = AIPawnRank - 1;

	// If blocked, try diagonal
	if (NewRank < 0) return; // already at the end

	if (IsSquareOccupied(AIPawnFile, NewRank))
	{
		// Try moving diagonally if player is there (capture)
		if (AIPawnFile > 0 && PlayerFile == AIPawnFile - 1 && PlayerRank == NewRank)
		{
			// Don't capture — just stay (AI is simple, not aggressive)
		}
		else if (AIPawnFile < Board->NumFiles - 1 && !IsSquareOccupied(AIPawnFile + 1, NewRank))
		{
			// Side-step if possible (simplified AI)
		}
		return; // blocked, skip turn
	}

	AIPawnRank = NewRank;
	Board->GlideActorToSquare(AIPawn, AIPawnFile, AIPawnRank, 5.f);
	AIPawn->BoardRank = AIPawnRank;

	// Check if AI reached the end
	if (AIPawnRank <= 0)
	{
		ShowGuideMessage(TEXT("The enemy pawn reached the end! Try again — you can do it!"));
		// Freeze input and restart the boss through the full phase reset
		bPhaseTransitionPending = true;
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ATutorialManager::ResetPhase, 2.0f, false);
	}
}

void APawnTutorial::OnTargetCollected(int32 File, int32 Rank)
{
	if (CurrentPhase == ETutorialPhase::Phase1)
		ShowGuideMessage(TEXT("You did it! Pawns march forward, one step at a time!"));
	else if (CurrentPhase == ETutorialPhase::Phase2)
		ShowGuideMessage(FString::Printf(TEXT("Got one! %d to go!"), TargetPositions.Num()));
}

void APawnTutorial::OnPhaseComplete()
{
	if (CurrentPhase == ETutorialPhase::Boss)
	{
		// Boss win is handled in AfterPlayerMove — don't auto-advance
		return;
	}

	if (CurrentPhase == ETutorialPhase::Phase3)
	{
		ShowGuideMessage(TEXT("You captured and conquered! Diagonal captures are your secret weapon!"));
	}

	Super::OnPhaseComplete();
}
