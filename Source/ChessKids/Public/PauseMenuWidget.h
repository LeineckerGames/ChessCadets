#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;

// Base class for WBP_PauseMenu. All logic lives here; the Widget Blueprint only
// needs buttons named ResumeButton / RestartButton / SettingsButton / QuitButton.
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
	UButton* RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* SettingsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* QuitButton;

	// Optional settings panel opened by the Settings button (WBP_Settings).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pause")
	TSubclassOf<UUserWidget> SettingsWidgetClass;

	UFUNCTION() void OnResumeClicked();
	UFUNCTION() void OnRestartClicked();
	UFUNCTION() void OnSettingsClicked();
	UFUNCTION() void OnQuitClicked();

private:
	UPROPERTY()
	UUserWidget* SettingsInstance = nullptr;
};
