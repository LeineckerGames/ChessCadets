#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChessHudWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class USlider;
class AChessManager;
class AChessPlayerController;

// In-game HUD: whose turn it is, a check warning, key hints, and a friendly
// game-over overlay with Play Again / Main Menu. Works standalone (the C++
// fallback builds its own tree) or as the base class of a styled WBP whose
// widgets use the BindWidgetOptional names below.
UCLASS()
class CHESSKIDS_API UChessHudWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TurnText;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* CheckText;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* HintText;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* HintResultText;   // "Hint: e2 → e4"
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* WhiteCapturedText;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* BlackCapturedText;
	UPROPERTY(meta = (BindWidgetOptional)) UVerticalBox* GameOverPanel;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* GameOverText;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* PlayAgainButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* MainMenuButton;

	// Board color sliders — bottom-left, always available in-game.
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* WhiteColorLabel;
	UPROPERTY(meta = (BindWidgetOptional)) USlider* WhiteColorSlider;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* BlackColorLabel;
	UPROPERTY(meta = (BindWidgetOptional)) USlider* BlackColorSlider;

	UFUNCTION() void HandleGameOver(FString Result);
	UFUNCTION() void HandleHintReady(FString FromSquare, FString ToSquare);
	UFUNCTION() void OnPlayAgainClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnWhiteColorChanged(float Value);
	UFUNCTION() void OnBlackColorChanged(float Value);

private:
	UPROPERTY() AChessManager* Manager = nullptr;

	// Last applied turn-indicator state (avoids restyling the text every tick).
	// 0 = P1 (2P), 1 = P2 (2P), 2 = your turn (1P), 3 = robot thinking (1P), -1 = unset.
	int32 LastTurnState = -1;

	void ApplyBoardColors();
	AChessPlayerController* GetChessPC() const;

	FTimerHandle HintHideTimer;
	int32 LastPositionVersion = -1;   // gates the captured-tray board scan to actual moves
};
