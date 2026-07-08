#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

// C++ base for WBP_MainMenu. The Widget Blueprint keeps its styling and its own
// navigation graph; this base layers game-mode semantics onto the existing buttons:
//   STORY MODE -> single player vs the AI (bTwoPlayerMode = false)
//   CHESS MODE -> local two-player hot-seat  (bTwoPlayerMode = true)
// and fixes the unbound Quit button + injects a Settings button.
UCLASS()
class CHESSKIDS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// Names match the widgets already inside WBP_MainMenu.
	UPROPERTY(meta = (BindWidgetOptional)) UButton* StoryModeButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* chessModeButton;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* quitButton;

	// Optional styled settings panel; falls back to the C++ USettingsMenuWidget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> SettingsWidgetClass;

	// Optional styled mode picker; falls back to the C++ UModeSelectWidget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> ModeSelectWidgetClass;

	UFUNCTION() void OnStoryMode();
	UFUNCTION() void OnChessMode();
	UFUNCTION() void OnQuit();
	UFUNCTION() void OnSettings();
	UFUNCTION() void OnLearn();

	// First tutorial level (isabella's Street Dash); the chain continues from there.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FName FirstTutorialLevel = TEXT("L_Tutorial_Pawn");

private:
	UPROPERTY() UUserWidget* SettingsInstance = nullptr;
	UPROPERTY() UUserWidget* ModeSelectInstance = nullptr;
	UPROPERTY() UButton* InjectedSettingsButton = nullptr;
	UPROPERTY() UButton* InjectedLearnButton = nullptr;

	void SetTwoPlayerMode(bool bTwoPlayer);
	void InjectSettingsButton();
	void InjectLearnButton();
};
