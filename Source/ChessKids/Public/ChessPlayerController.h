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

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void CompletePromotion(const FString& PieceLetter); 

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

	// In-game HUD (turn / check / game-over). Soft ref like the pause menu;
	// falls back to the C++ UChessHudWidget when unset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|UI")
	TSoftClassPtr<UUserWidget> HudClass;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	bool IsAIThinking() const { return bIsAIThinking; }

	// Takes back the last move safely: cancels the queued AI reply and clears
	// selection state before delegating to the manager. Bound to Z; also used
	// by the pause menu's Undo button.
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void UndoMove();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Pause")
	FName MainMenuMapName = TEXT("MainMenu");

	// --- Promotion picker (from main) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|UI")
	TSubclassOf<UUserWidget> PromotionPickerClass;

	UPROPERTY()
	UUserWidget* PromotionPickerWidget = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void ResetTurnState();

private:
	void ShowPauseMenu();
	void HidePauseMenu();

	UPROPERTY()
	UUserWidget* PauseMenuInstance = nullptr;

	UPROPERTY()
	UUserWidget* HudInstance = nullptr;

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
	FString PendingMoveStr;         
	bool bAwaitingPromotion = false;  
	FTimerHandle AITimerHandle;   


};


