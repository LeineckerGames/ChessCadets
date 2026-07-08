#pragma once

#include "CoreMinimal.h"
#include "TutorialManager.h"
#include "RookTutorial.generated.h"

// Level 2: Rook "Laser Grid"
// 6x6 grid, teaches straight-line sliding, unlimited range, blocked by obstacles
UCLASS()
class CHESSKIDS_API ARookTutorial : public ATutorialManager
{
	GENERATED_BODY()

public:
	ARookTutorial();

	// Boss phase: the fleeing runner's position
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial|Boss")
	int32 RunnerFile = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial|Boss")
	int32 RunnerRank = 0;

	// How many moves the player has to catch the runner
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Boss")
	int32 BossMoveLimit = 12;

protected:
	virtual void SetupPhase(ETutorialPhase Phase) override;
	virtual void AfterPlayerMove(int32 File, int32 Rank) override;
	virtual void OnTargetCollected(int32 File, int32 Rank) override;
	virtual void OnPhaseComplete() override;

private:
	UPROPERTY() AChessPiece* Runner = nullptr;

	void SetupPhase1();
	void SetupPhase2();
	void SetupPhase3();
	void SetupBoss();

	void MoveRunner();
};
