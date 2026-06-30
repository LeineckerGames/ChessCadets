#include "PauseMenuWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "ChessPlayerController.h"
#include "SettingsMenuWidget.h"
#include "InputCoreTypes.h"

TSharedRef<SWidget> UPauseMenuWidget::RebuildWidget()
{
	// Build a default UI in C++ when the Widget Blueprint has no tree of its own,
	// so the pause menu works without hand-authoring a WBP.
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Root;

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>();
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(Box))
		{
			CS->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			CS->SetAlignment(FVector2D(0.5f, 0.5f));
			CS->SetAutoSize(true);
		}

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
		Title->SetText(FText::FromString(TEXT("Paused")));
		Title->SetJustification(ETextJustify::Center);
		if (UVerticalBoxSlot* TS = Box->AddChildToVerticalBox(Title))
		{
			TS->SetPadding(FMargin(16.f, 12.f));
			TS->SetHorizontalAlignment(HAlign_Center);
		}

		auto MakeButton = [&](UButton*& OutBtn, const TCHAR* Label)
		{
			OutBtn = WidgetTree->ConstructWidget<UButton>();
			UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>();
			T->SetText(FText::FromString(Label));
			T->SetJustification(ETextJustify::Center);
			OutBtn->AddChild(T);
			if (UVerticalBoxSlot* BS = Box->AddChildToVerticalBox(OutBtn))
			{
				BS->SetPadding(FMargin(24.f, 8.f));
				BS->SetHorizontalAlignment(HAlign_Fill);
			}
		};

		MakeButton(ResumeButton, TEXT("Resume"));
		MakeButton(RestartButton, TEXT("Restart"));
		MakeButton(SettingsButton, TEXT("Settings"));
		MakeButton(QuitButton, TEXT("Quit to Menu"));
	}

	return Super::RebuildWidget();
}

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)   ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	if (RestartButton)  RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClicked);
	if (SettingsButton) SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
	if (QuitButton)     QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
}

FReply UPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::P)
	{
		if (SettingsInstance && SettingsInstance->IsInViewport())
			SettingsInstance->RemoveFromParent();   // close settings, stay paused
		else if (AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer()))
			PC->ResumeGame();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPauseMenuWidget::OnResumeClicked()
{
	if (AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer()))
		PC->ResumeGame();
}

void UPauseMenuWidget::OnRestartClicked()
{
	if (AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer()))
		PC->RestartArena();
}

void UPauseMenuWidget::OnSettingsClicked()
{
	UClass* Cls = SettingsWidgetClass ? SettingsWidgetClass.Get() : USettingsMenuWidget::StaticClass();
	if (!Cls) return;
	if (!SettingsInstance)
		SettingsInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), Cls);
	if (SettingsInstance && !SettingsInstance->IsInViewport())
		SettingsInstance->AddToViewport(100);
}

void UPauseMenuWidget::OnQuitClicked()
{
	if (AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer()))
		PC->QuitToMainMenu();
}
