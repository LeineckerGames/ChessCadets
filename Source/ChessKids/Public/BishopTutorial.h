#pragma once

#include "CoreMinimal.h"
#include "TutorialManager.h"
#include "BishopTutorial.generated.h"

// Level 3: Bishop "Color Lock"
// 6x6 grid, half neon green / half neon purple. Bishop is locked to its own color.
UCLASS()
class CHESSKIDS_API ABishopTutorial : public ATutorialManager
{
	GENERATED_BODY()

public:
	ABishopTutorial();

	// Color used for the bishop's own (reachable) squares
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Colors")
	FLinearColor OwnColor = FLinearColor(0.01f, 0.18f, 0.05f); // deep green (reads as green under bright lights)

	// Color for the opposite (unreachable) squares — kept dark so kids see the lock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Colors")
	FLinearColor OffColor = FLinearColor(0.04f, 0.005f, 0.08f); // near-black purple

	// Color a square turns when painted during the boss
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Colors")
	FLinearColor PaintColor = FLinearColor(0.0f, 2.0f, 0.3f); // vivid green, strong enough to read over bloom

	// Boss: how many moves the player gets to paint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Boss")
	int32 BossMoveLimit = 8;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial|Boss")
	int32 PaintedCount = 0;

protected:
	virtual void SetupPhase(ETutorialPhase Phase) override;
	virtual void AfterPlayerMove(int32 File, int32 Rank) override;
	virtual void OnTargetCollected(int32 File, int32 Rank) override;
	virtual void OnPhaseComplete() override;

private:
	// Decoy targets on the wrong color — visible but unreachable, not required for completion
	UPROPERTY() TArray<AChessPiece*> DecoyActors;

	// Squares painted during the boss
	TSet<int32> PaintedSquares;

	// Where the player was before the current move (to paint the slide path)
	int32 PrevFile = 0;
	int32 PrevRank = 0;

	void SetupPhase1();
	void SetupPhase2();
	void SetupPhase3();
	void SetupBoss();

	void ApplyColorLock();
	void SpawnDecoyAt(int32 File, int32 Rank);
	void PaintSquare(int32 File, int32 Rank);
	bool IsOwnColor(int32 File, int32 Rank) const;
};
