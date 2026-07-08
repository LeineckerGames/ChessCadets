#include "QueenTutorial.h"
#include "TutorialBoard.h"
#include "ChessPiece.h"

AQueenTutorial::AQueenTutorial()
{
	TaughtPiece = EChessPieceType::Queen;
}

void AQueenTutorial::SetupPhase(ETutorialPhase Phase)
{
	Super::SetupPhase(Phase);

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

// Phase 1: Targets in a row — slide straight like a rook
void AQueenTutorial::SetupPhase1()
{
	SpawnPlayerPiece(0, 2);

	SpawnTargetAt(1, 2);
	SpawnTargetAt(3, 2);
	SpawnTargetAt(5, 2);

	ShowGuideMessage(TEXT("Move like Tara's rook! Slide straight and grab them all!"));
}

// Phase 2: Targets on diagonals — slide like a bishop
void AQueenTutorial::SetupPhase2()
{
	SpawnPlayerPiece(0, 0);

	SpawnTargetAt(1, 1);
	SpawnTargetAt(3, 3);
	SpawnTargetAt(5, 1);

	ShowGuideMessage(TEXT("Now move like Zig's bishop! Slide diagonally!"));
}

// Phase 3: Targets everywhere — rows, columns, and diagonals mixed
void AQueenTutorial::SetupPhase3()
{
	SpawnPlayerPiece(0, 0);

	SpawnTargetAt(5, 0); // straight across
	SpawnTargetAt(3, 3); // diagonal
	SpawnTargetAt(3, 5); // straight up
	SpawnTargetAt(0, 2); // diagonal then straight — many routes
	SpawnTargetAt(4, 1);

	ShowGuideMessage(TEXT("Now use BOTH! Straight AND diagonal — you're unstoppable!"));
}

// Boss: Clear the Board — 10 orbs, with a par score to beat
void AQueenTutorial::SetupBoss()
{
	SpawnPlayerPiece(0, 0);

	SpawnTargetAt(3, 0);
	SpawnTargetAt(5, 0);
	SpawnTargetAt(1, 2);
	SpawnTargetAt(4, 2);
	SpawnTargetAt(2, 3);
	SpawnTargetAt(5, 3);
	SpawnTargetAt(0, 4);
	SpawnTargetAt(2, 4);
	SpawnTargetAt(1, 5);
	SpawnTargetAt(5, 5);

	PrevFile = 0;
	PrevRank = 0;

	ShowGuideMessage(FString::Printf(
		TEXT("BOSS: Clear the board! You sweep up every orb you slide across — can you do it in %d moves?"), BossPar));
}

void AQueenTutorial::AfterPlayerMove(int32 File, int32 Rank)
{
	if (CurrentPhase != ETutorialPhase::Boss) { PrevFile = File; PrevRank = Rank; return; }

	// Sweep: collect every orb along the slide path (landing square is handled by the base class)
	const int32 FileStep = (File > PrevFile) ? 1 : (File < PrevFile) ? -1 : 0;
	const int32 RankStep = (Rank > PrevRank) ? 1 : (Rank < PrevRank) ? -1 : 0;
	int32 CF = PrevFile + FileStep, CR = PrevRank + RankStep;
	while (CF != File || CR != Rank)
	{
		if (TargetPositions.Contains(FIntPoint(CF, CR)))
		{
			RemoveTargetAt(CF, CR);
			TargetsCollected++;
			OnTargetCollected(CF, CR);
		}
		CF += FileStep;
		CR += RankStep;
	}

	PrevFile = File;
	PrevRank = Rank;
}

void AQueenTutorial::OnTargetCollected(int32 File, int32 Rank)
{
	if (TargetPositions.Num() == 0) return; // final message comes from OnPhaseComplete

	if (CurrentPhase == ETutorialPhase::Boss)
	{
		ShowGuideMessage(FString::Printf(TEXT("%d orbs left — %d moves used!"),
			TargetPositions.Num(), MoveCount));
	}
	else
	{
		ShowGuideMessage(FString::Printf(TEXT("Got it! %d to go!"), TargetPositions.Num()));
	}
}

void AQueenTutorial::OnPhaseComplete()
{
	if (CurrentPhase == ETutorialPhase::Boss)
	{
		// Always a win — the par is a challenge, not a fail state
		if (MoveCount <= BossPar)
		{
			ShowGuideMessage(FString::Printf(
				TEXT("UNDER PAR! All 10 orbs in %d moves! The queen bows to no one!"), MoveCount));
		}
		else
		{
			ShowGuideMessage(FString::Printf(
				TEXT("Board cleared in %d moves! (Par is %d — think you can beat it next time?)"), MoveCount, BossPar));
		}
		CurrentPhase = ETutorialPhase::Complete;
		OnLevelComplete.Broadcast();
		return;
	}

	if (CurrentPhase == ETutorialPhase::Phase3)
	{
		ShowGuideMessage(TEXT("Rows, columns, diagonals — the whole board is yours!"));
	}

	Super::OnPhaseComplete();
}
