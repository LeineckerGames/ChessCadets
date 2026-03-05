#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChessPlayerController.generated.h"

class AChessBoard;

UCLASS()
class CHESSKIDS_API AChessPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AChessPlayerController();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	AChessBoard* Board = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Chess")
	AActor* SelectedActor = nullptr; // last actor clicked

private:
	void OnSelect();
};
