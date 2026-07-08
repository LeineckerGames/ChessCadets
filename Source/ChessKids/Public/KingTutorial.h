#pragma once

#include "CoreMinimal.h"
#include "TutorialManager.h"
#include "KingTutorial.generated.h"

// Level 6: King "The Core"
// Phase 1: King Maze (danger squares). Phase 2: recap of all pieces.
// Boss: hands off to the real chess match vs AXIOM.
UCLASS()
class CHESSKIDS_API AKingTutorial : public ATutorialManager
{
	GENERATED_BODY()

public:
	AKingTutorial();

	// Color of danger squares in the maze
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Colors")
	FLinearColor DangerColor = FLinearColor(0.6f, 0.02f, 0.02f);

	// Map to open for the AXIOM final battle. Leave None to stay in the level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Boss")
	FName AxiomMapName = NAME_None;

	// Seconds each friend gets during the recap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Recap")
	float RecapSecondsPerPiece = 5.f;

	// Pause after the recap intro line before the first friend appears
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Recap")
	float RecapIntroSeconds = 4.f;

protected:
	virtual void SetupPhase(ETutorialPhase Phase) override;
	virtual bool IsMoveAllowed(int32 FromFile, int32 FromRank, int32 ToFile, int32 ToRank) const override;
	virtual void OnTargetCollected(int32 File, int32 Rank) override;
	virtual void OnPhaseComplete() override;

private:
	// Maze danger squares (packed positions)
	TSet<int32> DangerSquares;

	// Recap state
	int32 RecapIndex = 0;
	UPROPERTY() AChessPiece* RecapPiece = nullptr;
	FTimerHandle RecapTimer;

	void SetupMaze();
	void SetupRecap();
	void SetupBoss();

	void AddDangerAt(int32 File, int32 Rank);
	void ShowNextRecapPiece();
	void StartAxiomBattle();
};
