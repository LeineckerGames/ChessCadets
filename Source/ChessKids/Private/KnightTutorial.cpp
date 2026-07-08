#include "KnightTutorial.h"
#include "TutorialBoard.h"
#include "ChessPiece.h"
#include "Engine/World.h"
#include "TimerManager.h"

AKnightTutorial::AKnightTutorial()
{
	TaughtPiece = EChessPieceType::Knight;
}

void AKnightTutorial::SetupPhase(ETutorialPhase Phase)
{
	Super::SetupPhase(Phase);

	if (Luna) { Luna->Destroy(); Luna = nullptr; }

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

// Phase 1: Stars one hop away, chained so each collected star reveals the next hop
void AKnightTutorial::SetupPhase1()
{
	SpawnPlayerPiece(2, 2);

	SpawnTargetAt(3, 4); // one L-hop from (2,2)
	SpawnTargetAt(5, 3); // one L-hop from (3,4)
	SpawnTargetAt(4, 1); // one L-hop from (5,3)

	ShowGuideMessage(TEXT("Knights move in an L — two squares one way, one to the side! Hop to the stars!"));
}

// Phase 2: Walls surround the player — but knights jump right over
void AKnightTutorial::SetupPhase2()
{
	SpawnPlayerPiece(0, 0);

	// Wall the player in
	SpawnWallAt(1, 0);
	SpawnWallAt(0, 1);
	SpawnWallAt(1, 1);

	SpawnTargetAt(1, 2);
	SpawnTargetAt(2, 1);
	SpawnTargetAt(4, 2);

	ShowGuideMessage(TEXT("You're walled in?! Not a problem — knights JUMP over anything!"));
}

// Phase 3: A trail of stars — connect-the-dots with L-hops
void AKnightTutorial::SetupPhase3()
{
	SpawnPlayerPiece(0, 0);

	// Each star is exactly one knight move from the previous one
	SpawnTargetAt(1, 2);
	SpawnTargetAt(2, 4);
	SpawnTargetAt(4, 3);
	SpawnTargetAt(5, 5);
	SpawnTargetAt(3, 4);

	// Scenery obstacles to hop over along the way
	SpawnWallAt(2, 2);
	SpawnWallAt(3, 3);

	ShowGuideMessage(TEXT("Follow the star trail! One L-hop at a time — like connect-the-dots!"));
}

// Boss: TAG — Luna hops away each turn; land on her square to tag her
void AKnightTutorial::SetupBoss()
{
	if (!Board) return;

	SpawnPlayerPiece(0, 0);

	LunaFile = Board->NumFiles - 1;
	LunaRank = Board->NumRanks - 1;

	FVector Location = Board->GridToWorldLocation(LunaFile, LunaRank, 5.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Luna = GetWorld()->SpawnActor<AChessPiece>(AChessPiece::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Luna)
	{
		Luna->Init(EChessPieceType::Knight, EChessColor::Black, LunaFile, LunaRank, nullptr);
		Luna->SetActorScale3D(FVector(0.8f));
	}

	LunaTurns = 0;
	bLunaResting = false;

	ShowGuideMessage(FString::Printf(
		TEXT("BOSS: Tag Luna! She hops like you do — land on her rooftop! You have %d hops!"), BossMoveLimit));
}

void AKnightTutorial::AfterPlayerMove(int32 File, int32 Rank)
{
	if (CurrentPhase != ETutorialPhase::Boss) return;
	if (!Board) return;

	// Tagged Luna?
	if (Luna && File == LunaFile && Rank == LunaRank)
	{
		Luna->Destroy();
		Luna = nullptr;
		ShowGuideMessage(TEXT("TAG! You got her! Nobody escapes a knight!"));
		CurrentPhase = ETutorialPhase::Complete;
		OnLevelComplete.Broadcast();
		return;
	}

	MoveLuna();

	// Out of hops?
	if (MoveCount >= BossMoveLimit)
	{
		ShowGuideMessage(TEXT("Luna got away! Try again — cut off her escape routes!"));
		bPhaseTransitionPending = true;
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ATutorialManager::ResetPhase, 2.0f, false);
		return;
	}

	// Coaching hints, most urgent first
	const int32 HopsLeft = BossMoveLimit - MoveCount;
	if (Luna && IsMoveAllowed(PlayerFile, PlayerRank, LunaFile, LunaRank))
	{
		ShowGuideMessage(FString::Printf(TEXT("She's ONE hop away — TAG HER NOW! (%d hops left)"), HopsLeft));
	}
	else if (bLunaResting)
	{
		ShowGuideMessage(FString::Printf(TEXT("Luna's catching her breath — close in! (%d hops left)"), HopsLeft));
	}
	else
	{
		ShowGuideMessage(FString::Printf(TEXT("Herd her into a corner! %d hops left!"), HopsLeft));
	}
	UpdateHighlights();
}

void AKnightTutorial::MoveLuna()
{
	if (!Luna || !Board) return;

	// Every few turns Luna rests — a predictable window for the player to close in
	++LunaTurns;
	bLunaResting = (LunaTurns % RestEvery == 0);
	if (bLunaResting) return;

	// Luna picks the knight-hop that puts her furthest from the player
	const int32 DF[8] = { 1, 1, -1, -1, 2, 2, -2, -2 };
	const int32 DR[8] = { 2, -2, 2, -2, 1, -1, 1, -1 };

	int32 BestFile = LunaFile, BestRank = LunaRank, BestDist = -1;
	for (int32 i = 0; i < 8; ++i)
	{
		const int32 NF = LunaFile + DF[i];
		const int32 NR = LunaRank + DR[i];
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

	LunaFile = BestFile;
	LunaRank = BestRank;
	Board->GlideActorToSquare(Luna, LunaFile, LunaRank, 5.f);
	Luna->BoardFile = LunaFile;
	Luna->BoardRank = LunaRank;
}

void AKnightTutorial::OnTargetCollected(int32 File, int32 Rank)
{
	if (TargetPositions.Num() > 0)
		ShowGuideMessage(FString::Printf(TEXT("Star collected! %d to go!"), TargetPositions.Num()));
}

void AKnightTutorial::OnPhaseComplete()
{
	if (CurrentPhase == ETutorialPhase::Boss)
	{
		// Boss end is handled in AfterPlayerMove — don't auto-advance
		return;
	}

	if (CurrentPhase == ETutorialPhase::Phase2)
	{
		ShowGuideMessage(TEXT("See? Walls mean NOTHING to a knight!"));
	}
	else if (CurrentPhase == ETutorialPhase::Phase3)
	{
		ShowGuideMessage(TEXT("Trail complete! You've mastered the L!"));
	}

	Super::OnPhaseComplete();
}
