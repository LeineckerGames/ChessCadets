#pragma once

#include "CoreMinimal.h"
#include "TutorialManager.h"
#include "QueenTutorial.generated.h"

// Level 5: Queen "Power Grid"
// 6x6 grid — rook + bishop combined. The power fantasy level.
UCLASS()
class CHESSKIDS_API AQueenTutorial : public ATutorialManager
{
	GENERATED_BODY()

public:
	AQueenTutorial();

	// Boss: par score shown to the player ("Can you do it in N moves?")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Boss")
	int32 BossPar = 8;

protected:
	virtual void SetupPhase(ETutorialPhase Phase) override;
	virtual void AfterPlayerMove(int32 File, int32 Rank) override;
	virtual void OnTargetCollected(int32 File, int32 Rank) override;
	virtual void OnPhaseComplete() override;

private:
	// Where the player was before the current move (to sweep orbs along the slide)
	int32 PrevFile = 0;
	int32 PrevRank = 0;

	void SetupPhase1();
	void SetupPhase2();
	void SetupPhase3();
	void SetupBoss();
};
