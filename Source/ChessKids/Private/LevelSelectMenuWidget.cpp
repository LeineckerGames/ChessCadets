#include "LevelSelectMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void ULevelSelectMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Level2) Btn_Level2->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel2);
	if (Btn_Level3) Btn_Level3->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel3);
	if (Btn_Level4) Btn_Level4->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel4);
	if (Btn_Level5) Btn_Level5->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel5);
	if (Btn_Level6) Btn_Level6->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel6);
}

void ULevelSelectMenuWidget::OpenArena(FName MapName)
{
	UGameplayStatics::OpenLevel(this, MapName);
}

// Progression order: pawns first, royalty last.
void ULevelSelectMenuWidget::OnLevel2() { OpenArena(TEXT("L_Pawn")); }
void ULevelSelectMenuWidget::OnLevel3() { OpenArena(TEXT("L_Knight")); }
void ULevelSelectMenuWidget::OnLevel4() { OpenArena(TEXT("L_Bishop")); }
void ULevelSelectMenuWidget::OnLevel5() { OpenArena(TEXT("L_Rook")); }
void ULevelSelectMenuWidget::OnLevel6() { OpenArena(TEXT("L_Royalty")); }
