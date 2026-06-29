#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

class UButton;
class UTextBlock;

// Graphics/quality settings driven by UGameUserSettings. Pure C++ UI (RebuildWidget)
// so it works without a hand-authored WBP; opened from the pause menu's Settings button.
UCLASS()
class CHESSKIDS_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(meta = (BindWidgetOptional)) UButton* LowButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* MediumButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* HighButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* EpicButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* VSyncButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* WindowModeButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* BackButton;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* StatusText;

	UFUNCTION() void OnLow();
	UFUNCTION() void OnMedium();
	UFUNCTION() void OnHigh();
	UFUNCTION() void OnEpic();
	UFUNCTION() void OnToggleVSync();
	UFUNCTION() void OnToggleWindowMode();
	UFUNCTION() void OnBack();

private:
	void SetQuality(int32 Level);
	void ApplyAndSave();
	void RefreshStatus();
};
