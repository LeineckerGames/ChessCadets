#include "ModeSelectWidget.h"
#include "ChessKidsGameInstance.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"

namespace
{
	UTextBlock* MakeText(UWidgetTree* Tree, UVerticalBox* Box, const TCHAR* Text,
	                     int32 Size, const TCHAR* Style, const FMargin& Pad)
	{
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>();
		T->SetText(FText::FromString(Text));
		T->SetJustification(ETextJustify::Center);
		T->SetFont(FCoreStyle::GetDefaultFontStyle(Style, Size));
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(T))
		{
			S->SetPadding(Pad);
			S->SetHorizontalAlignment(HAlign_Center);
		}
		return T;
	}

	void MakeButton(UWidgetTree* Tree, UVerticalBox* Box, UButton*& OutBtn,
	                const TCHAR* Label, int32 FontSize)
	{
		OutBtn = Tree->ConstructWidget<UButton>();
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>();
		T->SetText(FText::FromString(Label));
		T->SetJustification(ETextJustify::Center);
		T->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", FontSize));
		OutBtn->AddChild(T);
		if (UVerticalBoxSlot* BS = Box->AddChildToVerticalBox(OutBtn))
		{
			BS->SetPadding(FMargin(48.f, 8.f));
			BS->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}

TSharedRef<SWidget> UModeSelectWidget::RebuildWidget()
{
	// C++ fallback UI — a WBP subclass with the named widgets can restyle this freely.
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Root;

		// Translucent scrim so the menu background stays visible behind the question.
		UBorder* Scrim = WidgetTree->ConstructWidget<UBorder>();
		Scrim->SetBrushColor(FLinearColor(0.f, 0.f, 0.05f, 0.55f));
		Scrim->SetHorizontalAlignment(HAlign_Center);
		Scrim->SetVerticalAlignment(VAlign_Center);
		if (UCanvasPanelSlot* SS = Root->AddChildToCanvas(Scrim))
		{
			SS->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			SS->SetOffsets(FMargin(0.f));
		}

		// The scrim holds a wrapper canvas with both pages; one is visible at a time.
		UCanvasPanel* Pages = WidgetTree->ConstructWidget<UCanvasPanel>();
		Scrim->AddChild(Pages);

		// --- Page 1: mode ---
		ModeBox = WidgetTree->ConstructWidget<UVerticalBox>();
		if (UCanvasPanelSlot* PS = Pages->AddChildToCanvas(ModeBox)) { PS->SetAutoSize(true); }
		MakeText(WidgetTree, ModeBox, TEXT("CHESS MODE"), 42, TEXT("Bold"), FMargin(24.f, 8.f));
		MakeText(WidgetTree, ModeBox, TEXT("How do you want to play?"), 20, TEXT("Regular"), FMargin(24.f, 0.f, 24.f, 18.f));
		MakeButton(WidgetTree, ModeBox, OnePlayerButton, TEXT("1 PLAYER  VS  CPU"), 26);
		MakeButton(WidgetTree, ModeBox, TwoPlayerButton, TEXT("2 PLAYERS"), 26);
		MakeButton(WidgetTree, ModeBox, BackButton, TEXT("BACK"), 18);

		// --- Page 2: robot difficulty (hidden until 1P is chosen) ---
		DifficultyBox = WidgetTree->ConstructWidget<UVerticalBox>();
		DifficultyBox->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* PS = Pages->AddChildToCanvas(DifficultyBox)) { PS->SetAutoSize(true); }
		MakeText(WidgetTree, DifficultyBox, TEXT("1 PLAYER VS CPU"), 42, TEXT("Bold"), FMargin(24.f, 8.f));
		MakeText(WidgetTree, DifficultyBox, TEXT("How strong should the robot be?"), 20, TEXT("Regular"), FMargin(24.f, 0.f, 24.f, 18.f));
		MakeButton(WidgetTree, DifficultyBox, EasyButton, TEXT("EASY"), 26);
		MakeButton(WidgetTree, DifficultyBox, MediumButton, TEXT("MEDIUM"), 26);
		MakeButton(WidgetTree, DifficultyBox, HardButton, TEXT("HARD"), 26);
		MakeButton(WidgetTree, DifficultyBox, DifficultyBackButton, TEXT("BACK"), 18);
	}

	return Super::RebuildWidget();
}

void UModeSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (OnePlayerButton)      OnePlayerButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnOnePlayer);
	if (TwoPlayerButton)      TwoPlayerButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnTwoPlayer);
	if (BackButton)           BackButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnBack);
	if (EasyButton)           EasyButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnEasy);
	if (MediumButton)         MediumButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnMedium);
	if (HardButton)           HardButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnHard);
	if (DifficultyBackButton) DifficultyBackButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnDifficultyBack);

	ShowDifficultyPage(false);   // always land on the mode page
	SetIsFocusable(true);
	SetKeyboardFocus();          // so Escape works
}

FReply UModeSelectWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		// Difficulty page backs up one step; mode page closes the overlay.
		if (DifficultyBox && DifficultyBox->GetVisibility() == ESlateVisibility::Visible)
			ShowDifficultyPage(false);
		else
			OnBack();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UModeSelectWidget::ShowDifficultyPage(bool bShow)
{
	if (ModeBox)       ModeBox->SetVisibility(bShow ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (DifficultyBox) DifficultyBox->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UModeSelectWidget::StartGame(bool bTwoPlayer)
{
	if (UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
		GI->bTwoPlayerMode = bTwoPlayer;
	UGameplayStatics::OpenLevel(this, TargetLevel);
}

void UModeSelectWidget::StartOnePlayer(int32 Difficulty)
{
	if (UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
		GI->Difficulty = FMath::Clamp(Difficulty, 1, 3);
	StartGame(false);
}

void UModeSelectWidget::OnOnePlayer()      { ShowDifficultyPage(true); }
void UModeSelectWidget::OnTwoPlayer()      { StartGame(true); }
void UModeSelectWidget::OnEasy()           { StartOnePlayer(1); }
void UModeSelectWidget::OnMedium()         { StartOnePlayer(2); }
void UModeSelectWidget::OnHard()           { StartOnePlayer(3); }
void UModeSelectWidget::OnDifficultyBack() { ShowDifficultyPage(false); }

void UModeSelectWidget::OnBack()
{
	RemoveFromParent();
}
