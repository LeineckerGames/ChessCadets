#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChessPlayerController.generated.h"

class AChessBoard;
class AChessManager;
class AChessPiece;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	AChessManager* Manager = nullptr;

private:
	void OnSelect();

	FString SelectedSquare;
	bool bPieceSelected = false;
};
