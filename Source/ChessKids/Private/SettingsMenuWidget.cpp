#include "SettingsMenuWidget.h"
#include "ChessBoard.h"
#include "ChessKidsGameInstance.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "InputCoreTypes.h"

static UButton* MakeLabeledButton(UWidgetTree* Tree, const TCHAR* Label, UTextBlock*& OutText, int32 FontSize = 18)
{
	UButton* Btn = Tree->ConstructWidget<UButton>();
	OutText = Tree->ConstructWidget<UTextBlock>();
	OutText->SetText(FText::FromString(Label));
	OutText->SetJustification(ETextJustify::Center);
	OutText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", FontSize));
	OutText->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.05f, 0.08f)));
	Btn->AddChild(OutText);
	return Btn;
}

TSharedRef<SWidget> USettingsMenuWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Root;

		// Dark panel behind the menu CONTENT only (not full-screen): text stays
		// readable while the board remains visible around it — essential for the
		// color sliders, whose feedback is the board itself recoloring live.
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
		Title->SetText(FText::FromString(TEXT("SETTINGS")));
		Title->SetJustification(ETextJustify::Center);
		Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 34));
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

		// Board colors — two hue sliders for free recoloring (hidden when no board).
		BoardColorLabel = WidgetTree->ConstructWidget<UTextBlock>();
		BoardColorLabel->SetText(FText::FromString(TEXT("Board Colors")));
		BoardColorLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
		BoardColorLabel->SetJustification(ETextJustify::Center);
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(BoardColorLabel)) { S->SetPadding(FMargin(16.f, 10.f, 16.f, 2.f)); S->SetHorizontalAlignment(HAlign_Center); }

		auto MakeHueSlider = [&](UTextBlock*& OutLabel, USlider*& OutSlider, const TCHAR* Label)
		{
			OutLabel = WidgetTree->ConstructWidget<UTextBlock>();
			OutLabel->SetText(FText::FromString(Label));
			OutLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
			OutLabel->SetJustification(ETextJustify::Center);
			if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(OutLabel)) { S->SetPadding(FMargin(16.f, 4.f, 16.f, 0.f)); S->SetHorizontalAlignment(HAlign_Center); }

			OutSlider = WidgetTree->ConstructWidget<USlider>();
			OutSlider->SetMinValue(0.f);
			OutSlider->SetMaxValue(1.f);
			if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(OutSlider)) { S->SetPadding(FMargin(24.f, 2.f, 24.f, 4.f)); S->SetHorizontalAlignment(HAlign_Fill); }
		};
		MakeHueSlider(WhiteColorLabel, WhiteColorSlider, TEXT("White squares"));
		MakeHueSlider(BlackColorLabel, BlackColorSlider, TEXT("Black squares"));

		StatusText = WidgetTree->ConstructWidget<UTextBlock>();
		StatusText->SetJustification(ETextJustify::Center);
		StatusText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 15));
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(StatusText)) { S->SetPadding(FMargin(8.f, 10.f)); S->SetHorizontalAlignment(HAlign_Center); }

		DevUnlockButton = MakeLabeledButton(WidgetTree, TEXT("DEV: Unlock All - Off"), Tmp, 13);
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(DevUnlockButton)) { S->SetPadding(FMargin(24.f, 4.f)); S->SetHorizontalAlignment(HAlign_Fill); }

		BackButton = MakeLabeledButton(WidgetTree, TEXT("Back"), Tmp);
		if (UVerticalBoxSlot* S = Box->AddChildToVerticalBox(BackButton)) { S->SetPadding(FMargin(24.f, 10.f)); S->SetHorizontalAlignment(HAlign_Fill); }
	}

	return Super::RebuildWidget();
}

void USettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// AddUniqueDynamic: this widget instance is cached by the pause menu and re-added
	// each time settings opens, so NativeConstruct re-fires — plain AddDynamic would
	// stack handlers (a double-bound VSync toggle nets to a no-op).
	if (LowButton)        LowButton->OnClicked.AddUniqueDynamic(this, &USettingsMenuWidget::OnLow);
	if (MediumButton)     MediumButton->OnClicked.AddUniqueDynamic(this, &USettingsMenuWidget::OnMedium);
	if (HighButton)       HighButton->OnClicked.AddUniqueDynamic(this, &USettingsMenuWidget::OnHigh);
	if (EpicButton)       EpicButton->OnClicked.AddUniqueDynamic(this, &USettingsMenuWidget::OnEpic);
	if (VSyncButton)      VSyncButton->OnClicked.AddUniqueDynamic(this, &USettingsMenuWidget::OnToggleVSync);
	if (WindowModeButton) WindowModeButton->OnClicked.AddUniqueDynamic(this, &USettingsMenuWidget::OnToggleWindowMode);
	if (BackButton)       BackButton->OnClicked.AddUniqueDynamic(this, &USettingsMenuWidget::OnBack);
	if (WhiteColorSlider) WhiteColorSlider->OnValueChanged.AddUniqueDynamic(this, &USettingsMenuWidget::OnWhiteColorChanged);
	if (BlackColorSlider) BlackColorSlider->OnValueChanged.AddUniqueDynamic(this, &USettingsMenuWidget::OnBlackColorChanged);
	if (DevUnlockButton)  DevUnlockButton->OnClicked.AddUniqueDynamic(this, &USettingsMenuWidget::OnToggleDevUnlock);
	SetupBoardColorSliders();
	RefreshStatus();

	SetIsFocusable(true);
	SetKeyboardFocus();   // so Escape closes the panel
}

FReply USettingsMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::P)
	{
		OnBack();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
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

void USettingsMenuWidget::OnToggleDevUnlock()
{
	UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance());
	if (!GI) return;
	GI->bDevUnlockAll = !GI->bDevUnlockAll;
	if (DevUnlockButton)
		if (UTextBlock* T = Cast<UTextBlock>(DevUnlockButton->GetChildAt(0)))
			T->SetText(FText::FromString(GI->bDevUnlockAll
				? TEXT("DEV: Unlock All - ON") : TEXT("DEV: Unlock All - Off")));
}

void USettingsMenuWidget::OnBack()
{
	RemoveFromParent();
}

void USettingsMenuWidget::SetupBoardColorSliders()
{
	if (!CachedBoard)
		CachedBoard = Cast<AChessBoard>(UGameplayStatics::GetActorOfClass(this, AChessBoard::StaticClass()));

	// No board here (menu maps) — hide the whole section.
	const ESlateVisibility Vis = CachedBoard ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (BoardColorLabel)  BoardColorLabel->SetVisibility(Vis);
	if (WhiteColorLabel)  WhiteColorLabel->SetVisibility(Vis);
	if (WhiteColorSlider) WhiteColorSlider->SetVisibility(Vis);
	if (BlackColorLabel)  BlackColorLabel->SetVisibility(Vis);
	if (BlackColorSlider) BlackColorSlider->SetVisibility(Vis);
	if (!CachedBoard) return;

	// Sliders mirror the saved hues (SetValue doesn't fire OnValueChanged).
	if (const UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
	{
		if (WhiteColorSlider) WhiteColorSlider->SetValue(GI->LightSquareHue);
		if (BlackColorSlider) BlackColorSlider->SetValue(GI->DarkSquareHue);
	}
}

void USettingsMenuWidget::ApplyBoardColors()
{
	UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance());
	if (!GI) return;

	GI->bCustomBoardColors = true;   // custom hues now win over themes, everywhere
	if (WhiteColorSlider) GI->LightSquareHue = WhiteColorSlider->GetValue();
	if (BlackColorSlider) GI->DarkSquareHue  = BlackColorSlider->GetValue();

	// The widget instance outlives level travel — re-find the board if stale.
	if (!IsValid(CachedBoard))
	{
		CachedBoard = nullptr;
		CachedBoard = Cast<AChessBoard>(UGameplayStatics::GetActorOfClass(this, AChessBoard::StaticClass()));
	}

	UE_LOG(LogTemp, Log, TEXT("ApplyBoardColors: board=%s hueW=%.2f hueB=%.2f"),
		*GetNameSafe(CachedBoard), GI->LightSquareHue, GI->DarkSquareHue);

	if (CachedBoard)
		CachedBoard->SetSquareColors(
			UChessKidsGameInstance::LightColorFromHue(GI->LightSquareHue),
			UChessKidsGameInstance::DarkColorFromHue(GI->DarkSquareHue));
}

void USettingsMenuWidget::OnWhiteColorChanged(float Value) { ApplyBoardColors(); }
void USettingsMenuWidget::OnBlackColorChanged(float Value) { ApplyBoardColors(); }

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
