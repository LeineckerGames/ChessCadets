#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialSelectWidget.generated.h"

class UButton;

// Lesson picker shown by the main menu's LEARN CHESS button: jump into any of
// the six lessons (completed ones get a star). Overlay on the menu backdrop.
UCLASS()
class CHESSKIDS_API UTutorialSelectWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidgetOptional)) UButton* Lesson0Button;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Lesson1Button;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Lesson2Button;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Lesson3Button;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Lesson4Button;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Lesson5Button;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* BackButton;

	UFUNCTION() void OnLesson0();
	UFUNCTION() void OnLesson1();
	UFUNCTION() void OnLesson2();
	UFUNCTION() void OnLesson3();
	UFUNCTION() void OnLesson4();
	UFUNCTION() void OnLesson5();
	UFUNCTION() void OnBack();

private:
	void OpenLesson(FName MapName);
};
