#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialHudWidget.generated.h"

class UButton;
class UTextBlock;
class UBorder;
class ATutorialManager;

// Tutorial overlay: the guide's speech panel (bound to OnGuideMessage — replaces
// the debug-text fallback) and a MENU button back to the main menu. C++ fallback
// UI; a styled WBP subclass can rename-match the widgets to restyle.
UCLASS()
class CHESSKIDS_API UTutorialHudWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(meta = (BindWidgetOptional)) UBorder* GuidePanel;
	UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* GuideText;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* MenuButton;

	UFUNCTION() void HandleGuideMessage(const FString& Message);
	UFUNCTION() void OnMenuClicked();

private:
	UPROPERTY() ATutorialManager* Manager = nullptr;
	FTimerHandle HideTimer;
};
