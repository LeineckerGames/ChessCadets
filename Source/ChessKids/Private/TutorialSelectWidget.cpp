#include "TutorialSelectWidget.h"
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
	// Lesson index -> display name (isabella's minigame titles) — star when done.
	const TCHAR* LessonNames[6] = {
		TEXT("PAWN — Street Dash"),
		TEXT("ROOK — Laser Grid"),
		TEXT("BISHOP — Color Lock"),
		TEXT("KNIGHT — Rooftop Hop"),
		TEXT("QUEEN — Power Grid"),
		TEXT("KING — The Core"),
	};
}

TSharedRef<SWidget> UTutorialSelectWidget::RebuildWidget()
{
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

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>();
		Scrim->AddChild(Box);

		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
		Title->SetText(FText::FromString(TEXT("LEARN CHESS")));
		Title->SetJustification(ETextJustify::Center);
		Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 42));
		if (UVerticalBoxSlot* TS = Box->AddChildToVerticalBox(Title))
		{
			TS->SetPadding(FMargin(24.f, 8.f, 24.f, 16.f));
			TS->SetHorizontalAlignment(HAlign_Center);
		}

		const UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance());
		UButton** Buttons[6] = { &Lesson0Button, &Lesson1Button, &Lesson2Button,
		                         &Lesson3Button, &Lesson4Button, &Lesson5Button };
		for (int32 i = 0; i < 6; ++i)
		{
			UButton*& Btn = *Buttons[i];
			Btn = WidgetTree->ConstructWidget<UButton>();
			UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>();
			const bool bDone = GI && GI->IsTutorialComplete(i);
			T->SetText(FText::FromString(FString::Printf(TEXT("%s%s"),
				LessonNames[i], bDone ? TEXT("   *") : TEXT(""))));
			T->SetJustification(ETextJustify::Center);
			T->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 20));
			Btn->AddChild(T);
			if (UVerticalBoxSlot* BS = Box->AddChildToVerticalBox(Btn))
			{
				BS->SetPadding(FMargin(48.f, 6.f));
				BS->SetHorizontalAlignment(HAlign_Fill);
			}
		}

		BackButton = WidgetTree->ConstructWidget<UButton>();
		UTextBlock* BT = WidgetTree->ConstructWidget<UTextBlock>();
		BT->SetText(FText::FromString(TEXT("BACK")));
		BT->SetJustification(ETextJustify::Center);
		BT->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 16));
		BackButton->AddChild(BT);
		if (UVerticalBoxSlot* BS = Box->AddChildToVerticalBox(BackButton))
		{
			BS->SetPadding(FMargin(48.f, 14.f, 48.f, 6.f));
			BS->SetHorizontalAlignment(HAlign_Fill);
		}
	}

	return Super::RebuildWidget();
}

void UTutorialSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Lesson0Button) Lesson0Button->OnClicked.AddUniqueDynamic(this, &UTutorialSelectWidget::OnLesson0);
	if (Lesson1Button) Lesson1Button->OnClicked.AddUniqueDynamic(this, &UTutorialSelectWidget::OnLesson1);
	if (Lesson2Button) Lesson2Button->OnClicked.AddUniqueDynamic(this, &UTutorialSelectWidget::OnLesson2);
	if (Lesson3Button) Lesson3Button->OnClicked.AddUniqueDynamic(this, &UTutorialSelectWidget::OnLesson3);
	if (Lesson4Button) Lesson4Button->OnClicked.AddUniqueDynamic(this, &UTutorialSelectWidget::OnLesson4);
	if (Lesson5Button) Lesson5Button->OnClicked.AddUniqueDynamic(this, &UTutorialSelectWidget::OnLesson5);
	if (BackButton)    BackButton->OnClicked.AddUniqueDynamic(this, &UTutorialSelectWidget::OnBack);

	SetIsFocusable(true);
	SetKeyboardFocus();
}

FReply UTutorialSelectWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnBack();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTutorialSelectWidget::OpenLesson(FName MapName)
{
	UGameplayStatics::OpenLevel(this, MapName);
}

void UTutorialSelectWidget::OnLesson0() { OpenLesson(TEXT("L_Tutorial_Pawn")); }
void UTutorialSelectWidget::OnLesson1() { OpenLesson(TEXT("L_Tutorial_Rook")); }
void UTutorialSelectWidget::OnLesson2() { OpenLesson(TEXT("L_Tutorial_Bishop")); }
void UTutorialSelectWidget::OnLesson3() { OpenLesson(TEXT("L_Tutorial_Knight")); }
void UTutorialSelectWidget::OnLesson4() { OpenLesson(TEXT("L_Tutorial_Queen")); }
void UTutorialSelectWidget::OnLesson5() { OpenLesson(TEXT("L_Tutorial_King")); }

void UTutorialSelectWidget::OnBack()
{
	RemoveFromParent();
}
