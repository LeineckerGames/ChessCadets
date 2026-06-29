#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChessPlayerController.generated.h"

class AChessBoard;
class AChessManager;
class AChessPiece;
class UUserWidget;


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	AChessBoard* Board = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	AChessManager* Manager = nullptr;

	// --- Pause menu ---
	UFUNCTION(BlueprintCallable, Category = "Chess|Pause") void TogglePause();
	UFUNCTION(BlueprintCallable, Category = "Chess|Pause") void ResumeGame();
	UFUNCTION(BlueprintCallable, Category = "Chess|Pause") void RestartArena();
	UFUNCTION(BlueprintCallable, Category = "Chess|Pause") void QuitToMainMenu();

	// WBP_PauseMenu (soft ref: no hard dependency before the asset exists).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Pause")
	TSoftClassPtr<UUserWidget> PauseMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Pause")
	FName MainMenuMapName = TEXT("MainMenu");

private:
	void ShowPauseMenu();
	void HidePauseMenu();

	UPROPERTY()
	UUserWidget* PauseMenuInstance = nullptr;

	bool bPaused = false;

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
	FTimerHandle AITimerHandle;   


};


