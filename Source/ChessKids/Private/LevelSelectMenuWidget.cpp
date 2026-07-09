#include "LevelSelectMenuWidget.h"
#include "ChessKidsGameInstance.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void ULevelSelectMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Take the level buttons over completely: clear any Blueprint routing (the BP
	// used to send Level1 to L_Field) and bind the story ladder 1..5.
	auto Take = [](UButton* Btn) { if (Btn) Btn->OnClicked.Clear(); };
	Take(Btn_Level1); Take(Btn_Level2); Take(Btn_Level3);
	Take(Btn_Level4); Take(Btn_Level5); Take(Btn_Level6);

	if (Btn_Level1) Btn_Level1->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel1);
	if (Btn_Level2) Btn_Level2->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel2);
	if (Btn_Level3) Btn_Level3->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel3);
	if (Btn_Level4) Btn_Level4->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel4);
	if (Btn_Level5) Btn_Level5->OnClicked.AddUniqueDynamic(this, &ULevelSelectMenuWidget::OnLevel5);

	// Progression gate: chapter buttons enable as the player wins (or the dev
	// unlock-all toggle). SetIsEnabled(true) also overrides any disabled state
	// baked into the Widget Blueprint.
	const UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance());
	UButton* Chapters[5] = { Btn_Level1, Btn_Level2, Btn_Level3, Btn_Level4, Btn_Level5 };
	for (int32 i = 0; i < 5; ++i)
		if (Chapters[i])
			Chapters[i]->SetIsEnabled(!GI || GI->IsStoryChapterUnlocked(i + 1));

	// Story is five chapters; free play lives in CHESS MODE's venue picker.
	if (Btn_Level6) Btn_Level6->SetVisibility(ESlateVisibility::Collapsed);
}

void ULevelSelectMenuWidget::OpenArena(FName MapName)
{
	// Story mode is always you vs the robot.
	if (UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
		GI->bTwoPlayerMode = false;
	UGameplayStatics::OpenLevel(this, MapName);
}

// The class ladder: worker to royalty.
void ULevelSelectMenuWidget::OnLevel1() { OpenArena(TEXT("L_Pawn")); }
void ULevelSelectMenuWidget::OnLevel2() { OpenArena(TEXT("L_Knight")); }
void ULevelSelectMenuWidget::OnLevel3() { OpenArena(TEXT("L_Bishop")); }
void ULevelSelectMenuWidget::OnLevel4() { OpenArena(TEXT("L_Rook")); }
void ULevelSelectMenuWidget::OnLevel5() { OpenArena(TEXT("L_Royalty")); }
