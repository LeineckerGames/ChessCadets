#include "MainMenuWidget.h"
#include "ChessKidsGameInstance.h"
#include "SettingsMenuWidget.h"
#include "ModeSelectWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Story: BP keeps its own navigation; C++ only adds the mode flag.
	if (StoryModeButton) StoryModeButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnStoryMode);

	// Chess mode: take the button over completely — Clear() drops the BP's direct
	// OpenLevel binding so the 1P/2P picker can be shown first.
	if (chessModeButton)
	{
		chessModeButton->OnClicked.Clear();
		chessModeButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnChessMode);
	}

	if (quitButton)      quitButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnQuit);

	InjectSettingsButton();
	InjectLearnButton();
}

void UMainMenuWidget::SetTwoPlayerMode(bool bTwoPlayer)
{
	if (UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
		GI->bTwoPlayerMode = bTwoPlayer;
}

void UMainMenuWidget::OnStoryMode()
{
	SetTwoPlayerMode(false);   // story = you vs the robot; BP handles navigation
}

void UMainMenuWidget::OnChessMode()
{
	// Ask 1P-vs-CPU or 2-players on an overlay (menu background stays visible);
	// the picker sets the mode and opens the level.
	UClass* Cls = ModeSelectWidgetClass ? ModeSelectWidgetClass.Get() : UModeSelectWidget::StaticClass();
	if (!Cls) return;
	if (!ModeSelectInstance)
		ModeSelectInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), Cls);
	if (ModeSelectInstance && !ModeSelectInstance->IsInViewport())
		ModeSelectInstance->AddToViewport(50);
}

void UMainMenuWidget::OnQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UMainMenuWidget::OnSettings()
{
	UClass* Cls = SettingsWidgetClass ? SettingsWidgetClass.Get() : USettingsMenuWidget::StaticClass();
	if (!Cls) return;
	if (!SettingsInstance)
		SettingsInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), Cls);
	if (SettingsInstance && !SettingsInstance->IsInViewport())
		SettingsInstance->AddToViewport(100);
}

void UMainMenuWidget::OnLearn()
{
	SetTwoPlayerMode(false);
	UGameplayStatics::OpenLevel(this, FirstTutorialLevel);
}

void UMainMenuWidget::InjectLearnButton()
{
	if (InjectedLearnButton) return;

	UCanvasPanel* Canvas = WidgetTree ? Cast<UCanvasPanel>(WidgetTree->RootWidget) : nullptr;
	if (!Canvas) return;

	InjectedLearnButton = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
	Label->SetText(FText::FromString(TEXT("LEARN CHESS")));
	Label->SetJustification(ETextJustify::Center);
	Label->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 16));
	Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.08f)));
	InjectedLearnButton->AddChild(Label);
	InjectedLearnButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnLearn);

	if (UCanvasPanelSlot* CS = Canvas->AddChildToCanvas(InjectedLearnButton))
	{
		CS->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));   // bottom-left corner
		CS->SetAlignment(FVector2D(0.f, 1.f));
		CS->SetPosition(FVector2D(24.f, -24.f));
		CS->SetAutoSize(true);
	}
}

void UMainMenuWidget::InjectSettingsButton()
{
	if (InjectedSettingsButton) return;   // once per widget lifetime

	UCanvasPanel* Canvas = WidgetTree ? Cast<UCanvasPanel>(WidgetTree->RootWidget) : nullptr;
	if (!Canvas) return;

	InjectedSettingsButton = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
	Label->SetText(FText::FromString(TEXT("Settings")));
	Label->SetJustification(ETextJustify::Center);
	InjectedSettingsButton->AddChild(Label);
	InjectedSettingsButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnSettings);

	if (UCanvasPanelSlot* CS = Canvas->AddChildToCanvas(InjectedSettingsButton))
	{
		CS->SetAnchors(FAnchors(1.f, 1.f, 1.f, 1.f));   // bottom-right corner
		CS->SetAlignment(FVector2D(1.f, 1.f));
		CS->SetPosition(FVector2D(-24.f, -24.f));
		CS->SetAutoSize(true);
	}
}
