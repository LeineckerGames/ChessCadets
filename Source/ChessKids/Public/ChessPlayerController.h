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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void CompletePromotion(const FString& PieceLetter); 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	AChessBoard* Board = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	AChessManager* Manager = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|UI")
	TSubclassOf<UUserWidget> PromotionPickerClass;

	UPROPERTY()
	UUserWidget* PromotionPickerWidget = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void ResetTurnState();

private:
	//Actions
	void OnSelect();
	void OnDeselect();
	void HandleHover();
	void SelectPieceOnSquare(const FString& Square);

	//state
	FString SelectedSquare;
	FString HoveredSquare;
	bool bPieceSelected = false;
	bool bIsAIThinking = false;
	FString PendingMoveStr;         
	bool bAwaitingPromotion = false;  
	FTimerHandle AITimerHandle;   


};


