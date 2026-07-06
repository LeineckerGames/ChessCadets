#include "PauseMenuWidget.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "ChessPlayerController.h"
#include "ChessManager.h"
#include "ChessKidsGameInstance.h"
#include "SettingsMenuWidget.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UPauseMenuWidget::RebuildWidget()
{
	// Build a default UI in C++ when the Widget Blueprint has no tree of its own,
	// so the pause menu works without hand-authoring a WBP.
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Root;

		// Dark panel behind the menu content only — readable text, scene visible around it.
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
		Panel->SetBrushColor(FLinearColor(0.f, 0.f, 0.04f, 0.85f));
		Panel->SetPadding(FMargin(36.f, 24.f));
		if (UCanvasPanelSlot* PS = Root->AddChildToCanvas(Panel))
		{
			PS->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			PS->SetAlignment(FVector2D(0.5f, 0.5f));
			PS->SetAutoSize(true);
		}

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>();
		Panel->AddChild(Box);

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
		Title->SetText(FText::FromString(TEXT("PAUSED")));
		Title->SetJustification(ETextJustify::Center);
		Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 34));
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
			T->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
			T->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.08f)));
			OutBtn->AddChild(T);
			if (UVerticalBoxSlot* BS = Box->AddChildToVerticalBox(OutBtn))
			{
				BS->SetPadding(FMargin(24.f, 8.f));
				BS->SetHorizontalAlignment(HAlign_Fill);
			}
		};

		MakeButton(ResumeButton, TEXT("Resume"));
		MakeButton(UndoButton, TEXT("Undo Move"));
		MakeButton(TwoPlayerButton, TEXT("Two Players: Off"));
		MakeButton(RestartButton, TEXT("Restart"));
		MakeButton(SettingsButton, TEXT("Settings"));
		MakeButton(QuitButton, TEXT("Quit to Menu"));

		// Difficulty picker: label + Easy / Medium / Hard side by side.
		DifficultyLabel = WidgetTree->ConstructWidget<UTextBlock>();
		DifficultyLabel->SetText(FText::FromString(TEXT("Robot Difficulty")));
		DifficultyLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 16));
		DifficultyLabel->SetJustification(ETextJustify::Center);
		if (UVerticalBoxSlot* DS = Box->AddChildToVerticalBox(DifficultyLabel))
		{
			DS->SetPadding(FMargin(16.f, 14.f, 16.f, 4.f));
			DS->SetHorizontalAlignment(HAlign_Center);
		}

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		if (UVerticalBoxSlot* RS = Box->AddChildToVerticalBox(Row))
		{
			RS->SetPadding(FMargin(24.f, 4.f, 24.f, 8.f));
			RS->SetHorizontalAlignment(HAlign_Fill);
		}

		auto MakeDifficultyButton = [&](UButton*& OutBtn, const TCHAR* Label)
		{
			OutBtn = WidgetTree->ConstructWidget<UButton>();
			UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>();
			T->SetText(FText::FromString(Label));
			T->SetJustification(ETextJustify::Center);
			T->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 16));
			T->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.08f)));
			OutBtn->AddChild(T);
			if (UHorizontalBoxSlot* HS = Row->AddChildToHorizontalBox(OutBtn))
			{
				HS->SetPadding(FMargin(4.f, 0.f));
				HS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			}
		};
		MakeDifficultyButton(EasyButton, TEXT("Easy"));
		MakeDifficultyButton(MediumButton, TEXT("Medium"));
		MakeDifficultyButton(HardButton, TEXT("Hard"));
	}

	return Super::RebuildWidget();
}

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// NativeConstruct re-fires on every AddToViewport (the widget instance is cached and
	// re-added each pause) — AddUniqueDynamic keeps the handlers from stacking up.
	if (ResumeButton)   ResumeButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	if (UndoButton)     UndoButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnUndoClicked);
	if (RestartButton)  RestartButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnRestartClicked);
	if (SettingsButton) SettingsButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
	if (QuitButton)     QuitButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnQuitClicked);
	if (EasyButton)     EasyButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnEasyClicked);
	if (MediumButton)   MediumButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnMediumClicked);
	if (HardButton)     HardButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnHardClicked);
	if (TwoPlayerButton) TwoPlayerButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnTwoPlayerClicked);

	RefreshDifficultyLabel();
	RefreshTwoPlayerButton();
	ResetConfirms();   // reopening the menu resets any pending "Really ...?" question
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
	// Close the settings panel with the pause menu — leaving it up was how menus
	// got "stuck" on screen after resuming.
	if (SettingsInstance && SettingsInstance->IsInViewport())
		SettingsInstance->RemoveFromParent();
	if (AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer()))
		PC->ResumeGame();
}

void UPauseMenuWidget::OnRestartClicked()
{
	// Two-click confirm: first press asks, second press acts. Clicking anything
	// else (or reopening the menu) resets the question.
	if (!bConfirmRestart)
	{
		ResetConfirms();
		bConfirmRestart = true;
		SetButtonLabel(RestartButton, TEXT("Really restart?"));
		return;
	}
	if (SettingsInstance && SettingsInstance->IsInViewport())
		SettingsInstance->RemoveFromParent();
	if (AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer()))
		PC->RestartArena();
}

void UPauseMenuWidget::OnSettingsClicked()
{
	ResetConfirms();
	UClass* Cls = SettingsWidgetClass ? SettingsWidgetClass.Get() : USettingsMenuWidget::StaticClass();
	if (!Cls) return;
	if (!SettingsInstance)
		SettingsInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), Cls);
	if (SettingsInstance && !SettingsInstance->IsInViewport())
		SettingsInstance->AddToViewport(100);
}

void UPauseMenuWidget::OnQuitClicked()
{
	if (!bConfirmQuit)
	{
		ResetConfirms();
		bConfirmQuit = true;
		SetButtonLabel(QuitButton, TEXT("Really quit?"));
		return;
	}
	if (SettingsInstance && SettingsInstance->IsInViewport())
		SettingsInstance->RemoveFromParent();
	if (AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer()))
		PC->QuitToMainMenu();
}

void UPauseMenuWidget::ResetConfirms()
{
	if (bConfirmRestart) SetButtonLabel(RestartButton, TEXT("Restart"));
	if (bConfirmQuit)    SetButtonLabel(QuitButton, TEXT("Quit to Menu"));
	bConfirmRestart = false;
	bConfirmQuit = false;
}

void UPauseMenuWidget::SetButtonLabel(UButton* Button, const FString& Label)
{
	if (!Button) return;
	// Descend through wrapper panels (Overlay/Border/SizeBox in a styled WBP)
	// until the label TextBlock is found — GetChildAt(0) alone breaks on them.
	UWidget* W = Button->GetChildAt(0);
	while (W)
	{
		if (UTextBlock* T = Cast<UTextBlock>(W))
		{
			T->SetText(FText::FromString(Label));
			return;
		}
		UPanelWidget* Panel = Cast<UPanelWidget>(W);
		W = (Panel && Panel->GetChildrenCount() > 0) ? Panel->GetChildAt(0) : nullptr;
	}
}

void UPauseMenuWidget::OnUndoClicked()
{
	ResetConfirms();
	AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer());
	if (!PC || !PC->Manager) return;
	// Resume first (UndoMove no-ops while paused), then take back via the controller
	// path so the queued AI-reply timer and selection state are cleared too.
	PC->ResumeGame();
	PC->UndoMove();
}

void UPauseMenuWidget::OnEasyClicked()   { SetDifficulty(1); }
void UPauseMenuWidget::OnMediumClicked() { SetDifficulty(2); }
void UPauseMenuWidget::OnHardClicked()   { SetDifficulty(3); }

void UPauseMenuWidget::SetDifficulty(int32 Level)
{
	ResetConfirms();
	AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer());
	if (PC && PC->Manager)
		PC->Manager->SetDifficulty(Level);   // also persists to the GameInstance
	RefreshDifficultyLabel();
}

void UPauseMenuWidget::RefreshDifficultyLabel()
{
	if (!DifficultyLabel) return;

	int32 Level = 1;
	bool bTwoPlayer = false;
	if (AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer()))
		if (PC->Manager)
		{
			Level = PC->Manager->GetDifficulty();
			bTwoPlayer = PC->Manager->IsTwoPlayerMode();
		}

	// No robot in hot-seat mode — hide the difficulty controls.
	const ESlateVisibility Vis = bTwoPlayer ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	DifficultyLabel->SetVisibility(Vis);
	if (EasyButton)   EasyButton->SetVisibility(Vis);
	if (MediumButton) MediumButton->SetVisibility(Vis);
	if (HardButton)   HardButton->SetVisibility(Vis);

	const TCHAR* Name = (Level == 3) ? TEXT("Hard") : (Level == 2) ? TEXT("Medium") : TEXT("Easy");
	DifficultyLabel->SetText(FText::FromString(FString::Printf(TEXT("Robot Difficulty: %s"), Name)));
}

void UPauseMenuWidget::OnTwoPlayerClicked()
{
	ResetConfirms();
	AChessPlayerController* PC = Cast<AChessPlayerController>(GetOwningPlayer());
	if (!PC) return;

	UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance());
	if (!GI) return;

	// Flip the persisted mode and restart the arena so the game starts cleanly in it
	// (AChessManager::BeginPlay picks the flag up on load).
	GI->bTwoPlayerMode = !GI->bTwoPlayerMode;
	PC->RestartArena();
}

void UPauseMenuWidget::RefreshTwoPlayerButton()
{
	if (!TwoPlayerButton) return;

	bool bTwoPlayer = false;
	if (const UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
		bTwoPlayer = GI->bTwoPlayerMode;

	if (UTextBlock* Label = Cast<UTextBlock>(TwoPlayerButton->GetChildAt(0)))
		Label->SetText(FText::FromString(bTwoPlayer ? TEXT("Two Players: On") : TEXT("Two Players: Off")));
}
