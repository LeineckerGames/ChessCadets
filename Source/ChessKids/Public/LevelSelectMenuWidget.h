#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelSelectMenuWidget.generated.h"

class UButton;

// C++ base for WBP_LevelSelect. The BP already wires Btn_Level1 (-> L_Field) and
// Btn_Back; this base wires the five dead buttons to the piece arenas.
UCLASS()
class CHESSKIDS_API ULevelSelectMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// Names match the widgets already inside WBP_LevelSelect.
	// Btn_Level1 / Btn_Back intentionally NOT bound here — the BP handles them.
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Btn_Level2;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Btn_Level3;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Btn_Level4;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Btn_Level5;
	UPROPERTY(meta = (BindWidgetOptional)) UButton* Btn_Level6;

	UFUNCTION() void OnLevel2();
	UFUNCTION() void OnLevel3();
	UFUNCTION() void OnLevel4();
	UFUNCTION() void OnLevel5();
	UFUNCTION() void OnLevel6();

private:
	void OpenArena(FName MapName);
};
