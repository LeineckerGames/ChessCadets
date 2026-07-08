#pragma once

#include "CoreMinimal.h"
#include "TutorialManager.h"
#include "KnightTutorial.generated.h"

// Level 4: Knight "Rooftop Hop"
// 6x6 grid, teaches the L-shaped move and jumping over obstacles
UCLASS()
class CHESSKIDS_API AKnightTutorial : public ATutorialManager
{
	GENERATED_BODY()

public:
	AKnightTutorial();

	// Boss phase: Luna's position
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial|Boss")
	int32 LunaFile = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial|Boss")
	int32 LunaRank = 0;

	// How many moves the player has to tag Luna
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Boss")
	int32 BossMoveLimit = 15;

protected:
	virtual void SetupPhase(ETutorialPhase Phase) override;
	virtual void AfterPlayerMove(int32 File, int32 Rank) override;
	virtual void OnTargetCollected(int32 File, int32 Rank) override;
	virtual void OnPhaseComplete() override;

private:
	UPROPERTY() AChessPiece* Luna = nullptr;

	// Luna rests every RestEvery-th turn, giving the player a window to close in
	int32 LunaTurns = 0;
	static constexpr int32 RestEvery = 3;
	bool bLunaResting = false;

	void SetupPhase1();
	void SetupPhase2();
	void SetupPhase3();
	void SetupBoss();

	void MoveLuna();
};
