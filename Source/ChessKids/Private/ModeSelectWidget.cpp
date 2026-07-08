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

		UBorder* Scrim = WidgetTree->ConstructWidget<UBorder>();
		Scrim->SetBrushColor(FLinearColor(0.f, 0.f, 0.05f, 0.55f));
		Scrim->SetHorizontalAlignment(HAlign_Center);
		Scrim->SetVerticalAlignment(VAlign_Center);
		if (UCanvasPanelSlot* SS = Root->AddChildToCanvas(Scrim))
		{
			SS->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			SS->SetOffsets(FMargin(0.f));
		}

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

		// --- Page 2: robot difficulty ---
		DifficultyBox = WidgetTree->ConstructWidget<UVerticalBox>();
		DifficultyBox->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* PS = Pages->AddChildToCanvas(DifficultyBox)) { PS->SetAutoSize(true); }
		MakeText(WidgetTree, DifficultyBox, TEXT("1 PLAYER VS CPU"), 42, TEXT("Bold"), FMargin(24.f, 8.f));
		MakeText(WidgetTree, DifficultyBox, TEXT("How strong should the robot be?"), 20, TEXT("Regular"), FMargin(24.f, 0.f, 24.f, 18.f));
		MakeButton(WidgetTree, DifficultyBox, EasyButton, TEXT("EASY"), 26);
		MakeButton(WidgetTree, DifficultyBox, MediumButton, TEXT("MEDIUM"), 26);
		MakeButton(WidgetTree, DifficultyBox, HardButton, TEXT("HARD"), 26);
		MakeButton(WidgetTree, DifficultyBox, DifficultyBackButton, TEXT("BACK"), 18);

		// --- Page 3: venue (both free play) ---
		VenueBox = WidgetTree->ConstructWidget<UVerticalBox>();
		VenueBox->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* PS = Pages->AddChildToCanvas(VenueBox)) { PS->SetAutoSize(true); }
		MakeText(WidgetTree, VenueBox, TEXT("PICK YOUR ARENA"), 42, TEXT("Bold"), FMargin(24.f, 8.f));
		MakeText(WidgetTree, VenueBox, TEXT("Where do you want to play?"), 20, TEXT("Regular"), FMargin(24.f, 0.f, 24.f, 18.f));
		MakeButton(WidgetTree, VenueBox, GridButton, TEXT("THE GRID"), 26);
		MakeButton(WidgetTree, VenueBox, FieldButton, TEXT("THE FIELD"), 26);
		MakeButton(WidgetTree, VenueBox, VenueBackButton, TEXT("BACK"), 18);
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
	if (GridButton)           GridButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnGrid);
	if (FieldButton)          FieldButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnField);
	if (VenueBackButton)      VenueBackButton->OnClicked.AddUniqueDynamic(this, &UModeSelectWidget::OnVenueBack);

	ShowPage(EPage::Mode);
	SetIsFocusable(true);
	SetKeyboardFocus();
}

FReply UModeSelectWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		// Escape steps back one page; on the first page it closes the overlay.
		if (CurrentPage == EPage::Venue)
			ShowPage(bPendingTwoPlayer ? EPage::Mode : EPage::Difficulty);
		else if (CurrentPage == EPage::Difficulty)
			ShowPage(EPage::Mode);
		else
			OnBack();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UModeSelectWidget::ShowPage(EPage Page)
{
	CurrentPage = Page;
	if (ModeBox)       ModeBox->SetVisibility(Page == EPage::Mode ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (DifficultyBox) DifficultyBox->SetVisibility(Page == EPage::Difficulty ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (VenueBox)      VenueBox->SetVisibility(Page == EPage::Venue ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UModeSelectWidget::PickDifficulty(int32 Difficulty)
{
	PendingDifficulty = FMath::Clamp(Difficulty, 1, 3);
	ShowPage(EPage::Venue);
}

void UModeSelectWidget::StartGame(FName Venue)
{
	if (UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
	{
		GI->bTwoPlayerMode = bPendingTwoPlayer;
		if (!bPendingTwoPlayer)
			GI->Difficulty = PendingDifficulty;
	}
	UGameplayStatics::OpenLevel(this, Venue);
}

void UModeSelectWidget::OnOnePlayer()      { bPendingTwoPlayer = false; ShowPage(EPage::Difficulty); }
void UModeSelectWidget::OnTwoPlayer()      { bPendingTwoPlayer = true; ShowPage(EPage::Venue); }
void UModeSelectWidget::OnEasy()           { PickDifficulty(1); }
void UModeSelectWidget::OnMedium()         { PickDifficulty(2); }
void UModeSelectWidget::OnHard()           { PickDifficulty(3); }
void UModeSelectWidget::OnDifficultyBack() { ShowPage(EPage::Mode); }
void UModeSelectWidget::OnGrid()           { StartGame(TEXT("L_Grid")); }
void UModeSelectWidget::OnField()          { StartGame(TEXT("L_Field")); }
void UModeSelectWidget::OnVenueBack()      { ShowPage(bPendingTwoPlayer ? EPage::Mode : EPage::Difficulty); }

void UModeSelectWidget::OnBack()
{
	RemoveFromParent();
}
