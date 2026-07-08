#pragma once

#include "CoreMinimal.h"
#include "TutorialManager.h"
#include "PawnTutorial.generated.h"

// Level 1: Pawn "Street Dash"
// 4-column grid, teaches forward movement, first-move sprint, diagonal capture
UCLASS()
class CHESSKIDS_API APawnTutorial : public ATutorialManager
{
	GENERATED_BODY()

public:
	APawnTutorial();

	// Boss phase: AI pawn's position
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial|Boss")
	int32 AIPawnFile = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial|Boss")
	int32 AIPawnRank = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial|Boss")
	int32 BossMoveLimit = 20;

protected:
	virtual void SetupPhase(ETutorialPhase Phase) override;
	virtual void AfterPlayerMove(int32 File, int32 Rank) override;
	virtual void OnTargetCollected(int32 File, int32 Rank) override;
	virtual void OnPhaseComplete() override;
	virtual bool IsMoveAllowed(int32 FromFile, int32 FromRank, int32 ToFile, int32 ToRank) const override;

private:
	UPROPERTY() AChessPiece* AIPawn = nullptr;

	void SetupPhase1();
	void SetupPhase2();
	void SetupPhase3();
	void SetupBoss();

	void MoveAIPawn();
	bool IsSquareOccupied(int32 File, int32 Rank) const;
};
