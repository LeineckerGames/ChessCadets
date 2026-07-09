#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

class UButton;
class UTextBlock;
class USlider;
class AChessBoard;

// Graphics/quality settings driven by UGameUserSettings. Pure C++ UI (RebuildWidget)
// so it works without a hand-authored WBP; opened from the pause menu's Settings button.
UCLASS()
class CHESSKIDS_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidgetOptional)) UButton* LowButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* MediumButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* HighButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* EpicButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* VSyncButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* WindowModeButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* BackButton;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* StatusText;

	// Board colors — free recoloring via hue sliders (shown only where a board exists).
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* BoardColorLabel;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* WhiteColorLabel;
	UPROPERTY(meta = (BindWidgetOptional)) USlider* WhiteColorSlider;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* BlackColorLabel;
	UPROPERTY(meta = (BindWidgetOptional)) USlider* BlackColorSlider;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* DevUnlockButton;

	UFUNCTION() void OnLow();
	UFUNCTION() void OnMedium();
	UFUNCTION() void OnHigh();
	UFUNCTION() void OnEpic();
	UFUNCTION() void OnToggleVSync();
	UFUNCTION() void OnToggleWindowMode();
	UFUNCTION() void OnBack();
	UFUNCTION() void OnWhiteColorChanged(float Value);
	UFUNCTION() void OnBlackColorChanged(float Value);
	UFUNCTION() void OnToggleDevUnlock();

private:
	UPROPERTY() AChessBoard* CachedBoard = nullptr;

	void SetQuality(int32 Level);
	void ApplyAndSave();
	void RefreshStatus();
	void SetupBoardColorSliders();
	void ApplyBoardColors();
};
