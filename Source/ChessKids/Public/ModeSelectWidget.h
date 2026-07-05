#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ModeSelectWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

// Shown when CHESS MODE is clicked on the main menu: a translucent overlay
// (menu background stays visible) asking 1 Player vs CPU or 2 Players.
// Picking 1P swaps to a difficulty page (Easy/Medium/Hard) before starting;
// picking 2P starts immediately. Choices persist on the GameInstance.
UCLASS()
class CHESSKIDS_API UModeSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Level to open after choosing (what CHESS MODE opened directly before).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FName TargetLevel = TEXT("L_Field");

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// Page 1 — mode
	UPROPERTY(meta = (BindWidgetOptional)) UVerticalBox* ModeBox;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* OnePlayerButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* TwoPlayerButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* BackButton;

	// Page 2 — robot difficulty (1P only)
	UPROPERTY(meta = (BindWidgetOptional)) UVerticalBox* DifficultyBox;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* EasyButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* MediumButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* HardButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* DifficultyBackButton;

	UFUNCTION() void OnOnePlayer();
	UFUNCTION() void OnTwoPlayer();
	UFUNCTION() void OnBack();
	UFUNCTION() void OnEasy();
	UFUNCTION() void OnMedium();
	UFUNCTION() void OnHard();
	UFUNCTION() void OnDifficultyBack();

private:
	void ShowDifficultyPage(bool bShow);
	void StartOnePlayer(int32 Difficulty);
	void StartGame(bool bTwoPlayer);
};
