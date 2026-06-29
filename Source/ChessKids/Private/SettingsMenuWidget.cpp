#include "SettingsMenuWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"

static UButton* MakeLabeledButton(UWidgetTree* Tree, const TCHAR* Label, UTextBlock*& OutText)
{
	UButton* Btn = Tree->ConstructWidget<UButton>();
	OutText = Tree->ConstructWidget<UTextBlock>();
	OutText->SetText(FText::FromString(Label));
	OutText->SetJustification(ETextJustify::Center);
	Btn->AddChild(OutText);
	return Btn;
}

TSharedRef<SWidget> USettingsMenuWidget::RebuildWidget()
{
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
		Title->SetText(FText::FromString(TEXT("Settings")));
		Title->SetJustification(ETextJustify::Center);
		if (UVerticalBoxSlot* TS = Box->AddChildToVerticalBox(Title)) { TS->SetPadding(FMargin(16.f, 12.f)); TS->SetHorizontalAlignment(HAlign_Center); }

		// Quality row
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		if (UVerticalBoxSlot* RS = Box->AddChildToVerticalBox(Row)) { RS->SetPadding(FMargin(8.f)); RS->SetHorizontalAlignment(HAlign_Center); }
		UTextBlock* Tmp = nullptr;
		LowButton = MakeLabeledButton(WidgetTree, TEXT("Low"), Tmp);    Row->AddChildToHorizontalBox(LowButton);
		MediumButton = MakeLabeledButton(WidgetTree, TEXT("Medium"), Tmp); Row->AddChildToHorizontalBox(MediumButton);
		HighButton = MakeLabeledButton(WidgetTree, TEXT("High"), Tmp);  Row->AddChildToHorizontalBox(HighButton);
		EpicButton = MakeLabeledButton(WidgetTree, TEXT("Epic"), Tmp);  Row->AddChildToHorizontalBox(EpicButton);

		VSyncButton = MakeLabeledButton(WidgetTree, TEXT("Toggle VSync"), Tmp);
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(VSyncButton)) { S->SetPadding(FMargin(24.f, 6.f)); S->SetHorizontalAlignment(HAlign_Fill); }

		WindowModeButton = MakeLabeledButton(WidgetTree, TEXT("Toggle Window Mode"), Tmp);
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(WindowModeButton)) { S->SetPadding(FMargin(24.f, 6.f)); S->SetHorizontalAlignment(HAlign_Fill); }

		StatusText = WidgetTree->ConstructWidget<UTextBlock>();
		StatusText->SetJustification(ETextJustify::Center);
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(StatusText)) { S->SetPadding(FMargin(8.f, 10.f)); S->SetHorizontalAlignment(HAlign_Center); }

		BackButton = MakeLabeledButton(WidgetTree, TEXT("Back"), Tmp);
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(BackButton)) { S->SetPadding(FMargin(24.f, 10.f)); S->SetHorizontalAlignment(HAlign_Fill); }
	}

	return Super::RebuildWidget();
}

void USettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (LowButton)        LowButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnLow);
	if (MediumButton)     MediumButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnMedium);
	if (HighButton)       HighButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnHigh);
	if (EpicButton)       EpicButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnEpic);
	if (VSyncButton)      VSyncButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnToggleVSync);
	if (WindowModeButton) WindowModeButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnToggleWindowMode);
	if (BackButton)       BackButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::OnBack);
	RefreshStatus();
}

static UGameUserSettings* GUS()
{
	return GEngine ? GEngine->GetGameUserSettings() : nullptr;
}

void USettingsMenuWidget::SetQuality(int32 Level)
{
	if (UGameUserSettings* S = GUS())
	{
		S->SetOverallScalabilityLevel(Level);
		ApplyAndSave();
		RefreshStatus();
	}
}

void USettingsMenuWidget::OnLow()    { SetQuality(0); }
void USettingsMenuWidget::OnMedium() { SetQuality(1); }
void USettingsMenuWidget::OnHigh()   { SetQuality(2); }
void USettingsMenuWidget::OnEpic()   { SetQuality(3); }

void USettingsMenuWidget::OnToggleVSync()
{
	if (UGameUserSettings* S = GUS())
	{
		S->SetVSyncEnabled(!S->IsVSyncEnabled());
		ApplyAndSave();
		RefreshStatus();
	}
}

void USettingsMenuWidget::OnToggleWindowMode()
{
	if (UGameUserSettings* S = GUS())
	{
		const EWindowMode::Type Cur = S->GetFullscreenMode();
		EWindowMode::Type Next = EWindowMode::WindowedFullscreen;
		if (Cur == EWindowMode::WindowedFullscreen) Next = EWindowMode::Windowed;
		else if (Cur == EWindowMode::Windowed)      Next = EWindowMode::Fullscreen;
		else                                        Next = EWindowMode::WindowedFullscreen;
		S->SetFullscreenMode(Next);
		ApplyAndSave();
		RefreshStatus();
	}
}

void USettingsMenuWidget::OnBack()
{
	RemoveFromParent();
}

void USettingsMenuWidget::ApplyAndSave()
{
	if (UGameUserSettings* S = GUS())
	{
		S->ApplySettings(false);
		S->SaveSettings();
	}
}

void USettingsMenuWidget::RefreshStatus()
{
	if (!StatusText) return;
	UGameUserSettings* S = GUS();
	if (!S) { StatusText->SetText(FText::FromString(TEXT("Settings unavailable"))); return; }

	const int32 Q = S->GetOverallScalabilityLevel();
	const TCHAR* QName =
		Q == 0 ? TEXT("Low") : Q == 1 ? TEXT("Medium") : Q == 2 ? TEXT("High") :
		Q == 3 ? TEXT("Epic") : Q == 4 ? TEXT("Cinematic") : TEXT("Custom");

	const EWindowMode::Type WM = S->GetFullscreenMode();
	const TCHAR* WName =
		WM == EWindowMode::Fullscreen ? TEXT("Fullscreen") :
		WM == EWindowMode::WindowedFullscreen ? TEXT("Borderless") : TEXT("Windowed");

	StatusText->SetText(FText::FromString(FString::Printf(
		TEXT("Quality: %s   |   VSync: %s   |   %s"),
		QName, S->IsVSyncEnabled() ? TEXT("On") : TEXT("Off"), WName)));
}
