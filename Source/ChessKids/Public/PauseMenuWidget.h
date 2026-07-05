#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UTextBlock;

// Base class for WBP_PauseMenu. All logic lives here; the Widget Blueprint only
// needs buttons named ResumeButton / RestartButton / SettingsButton / QuitButton
// (optionally UndoButton / EasyButton / MediumButton / HardButton / DifficultyLabel).
UCLASS()
class CHESSKIDS_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ResumeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* UndoButton;

	// Toggles hot-seat two-player mode (restarts the arena in the new mode).
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* TwoPlayerButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* SettingsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* QuitButton;

	// Difficulty picker
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DifficultyLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* EasyButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* MediumButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* HardButton;

	// Optional settings panel opened by the Settings button (WBP_Settings).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pause")
	TSubclassOf<UUserWidget> SettingsWidgetClass;

	UFUNCTION() void OnResumeClicked();
	UFUNCTION() void OnUndoClicked();
	UFUNCTION() void OnRestartClicked();
	UFUNCTION() void OnSettingsClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnEasyClicked();
	UFUNCTION() void OnMediumClicked();
	UFUNCTION() void OnHardClicked();
	UFUNCTION() void OnTwoPlayerClicked();

private:
	UPROPERTY()
	UUserWidget* SettingsInstance = nullptr;

	void SetDifficulty(int32 Level);
	void RefreshDifficultyLabel();
	void RefreshTwoPlayerButton();
};
