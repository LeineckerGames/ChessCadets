#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TutorialPlayerController.generated.h"

class ATutorialBoard;
class ATutorialManager;

UCLASS()
class CHESSKIDS_API ATutorialPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATutorialPlayerController();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	ATutorialBoard* Board = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	ATutorialManager* Manager = nullptr;

private:
	void OnClick();
	void HandleHover();

	FString HoveredSquare;
};
