#include "TutorialHudWidget.h"
#include "TutorialManager.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "TimerManager.h"

TSharedRef<SWidget> UTutorialHudWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Root;

		// Guide speech panel — bottom center, where kids' eyes rest.
		GuidePanel = WidgetTree->ConstructWidget<UBorder>();
		GuidePanel->SetBrushColor(FLinearColor(0.f, 0.f, 0.05f, 0.8f));
		GuidePanel->SetPadding(FMargin(28.f, 14.f));
		GuidePanel->SetVisibility(ESlateVisibility::Hidden);
		if (UCanvasPanelSlot* GS = Root->AddChildToCanvas(GuidePanel))
		{
			GS->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
			GS->SetAlignment(FVector2D(0.5f, 1.f));
			GS->SetPosition(FVector2D(0.f, -48.f));
			GS->SetAutoSize(true);
		}
		GuideText = WidgetTree->ConstructWidget<UTextBlock>();
		GuideText->SetJustification(ETextJustify::Center);
		GuideText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 20));
		GuideText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.98f, 1.f)));
		GuidePanel->AddChild(GuideText);

		// MENU button — top right, always available.
		MenuButton = WidgetTree->ConstructWidget<UButton>();
		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
		Label->SetText(FText::FromString(TEXT("MENU")));
		Label->SetJustification(ETextJustify::Center);
		Label->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 16));
		Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.08f)));
		MenuButton->AddChild(Label);
		if (UCanvasPanelSlot* MS = Root->AddChildToCanvas(MenuButton))
		{
			MS->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
			MS->SetAlignment(FVector2D(1.f, 0.f));
			MS->SetPosition(FVector2D(-24.f, 24.f));
			MS->SetAutoSize(true);
		}
	}

	return Super::RebuildWidget();
}

void UTutorialHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MenuButton)
		MenuButton->OnClicked.AddUniqueDynamic(this, &UTutorialHudWidget::OnMenuClicked);

	Manager = Cast<ATutorialManager>(
		UGameplayStatics::GetActorOfClass(this, ATutorialManager::StaticClass()));
	if (Manager)
		Manager->OnGuideMessage.AddUniqueDynamic(this, &UTutorialHudWidget::HandleGuideMessage);
}

void UTutorialHudWidget::HandleGuideMessage(const FString& Message)
{
	if (!GuideText || !GuidePanel) return;
	GuideText->SetText(FText::FromString(Message));
	GuidePanel->SetVisibility(ESlateVisibility::HitTestInvisible);

	// Linger long enough to read aloud, then tuck away.
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UTutorialHudWidget> WeakThis(this);
		World->GetTimerManager().SetTimer(HideTimer, [WeakThis]()
		{
			if (UTutorialHudWidget* Self = WeakThis.Get())
				if (Self->GuidePanel)
					Self->GuidePanel->SetVisibility(ESlateVisibility::Hidden);
		}, 6.f, false);
	}
}

void UTutorialHudWidget::OnMenuClicked()
{
	UGameplayStatics::OpenLevel(this, TEXT("MainMenu"));
}
