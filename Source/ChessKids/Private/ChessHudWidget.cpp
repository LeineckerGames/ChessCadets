#include "ChessHudWidget.h"
#include "ChessManager.h"
#include "ChessPlayerController.h"
#include "ChessBoard.h"
#include "ChessKidsGameInstance.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UChessHudWidget::RebuildWidget()
{
	// C++ fallback UI — used when no Widget Blueprint provides a tree.
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Root;

		// Turn indicator — top center.
		TurnText = WidgetTree->ConstructWidget<UTextBlock>();
		TurnText->SetText(FText::FromString(TEXT("Your turn!")));
		TurnText->SetJustification(ETextJustify::Center);
		TurnText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 24));
		TurnText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.95f, 1.f)));
		if (UCanvasPanelSlot* TS = Root->AddChildToCanvas(TurnText))
		{
			TS->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
			TS->SetAlignment(FVector2D(0.5f, 0.f));
			TS->SetPosition(FVector2D(0.f, 24.f));
			TS->SetAutoSize(true);
		}

		// Check warning — under the turn indicator, red, hidden by default.
		CheckText = WidgetTree->ConstructWidget<UTextBlock>();
		CheckText->SetText(FText::FromString(TEXT("Check!")));
		CheckText->SetJustification(ETextJustify::Center);
		CheckText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 20));
		CheckText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.25f, 0.2f)));
		CheckText->SetVisibility(ESlateVisibility::Hidden);
		if (UCanvasPanelSlot* CS = Root->AddChildToCanvas(CheckText))
		{
			CS->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
			CS->SetAlignment(FVector2D(0.5f, 0.f));
			CS->SetPosition(FVector2D(0.f, 62.f));
			CS->SetAutoSize(true);
		}

		// Key hints — bottom left.
		HintText = WidgetTree->ConstructWidget<UTextBlock>();
		HintText->SetText(FText::FromString(TEXT("P — Pause    Z — Undo move")));
		HintText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 12));
		HintText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.55f)));
		if (UCanvasPanelSlot* HS = Root->AddChildToCanvas(HintText))
		{
			HS->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
			HS->SetAlignment(FVector2D(0.f, 1.f));
			HS->SetPosition(FVector2D(24.f, -18.f));
			HS->SetAutoSize(true);
		}

		// Board color sliders — bottom left, stacked above the key hints.
		UVerticalBox* ColorBox = WidgetTree->ConstructWidget<UVerticalBox>();
		if (UCanvasPanelSlot* CBS = Root->AddChildToCanvas(ColorBox))
		{
			CBS->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
			CBS->SetAlignment(FVector2D(0.f, 1.f));
			CBS->SetPosition(FVector2D(24.f, -44.f));
			CBS->SetAutoSize(false);
			CBS->SetSize(FVector2D(190.f, 116.f));
		}

		auto MakeHueSlider = [&](UTextBlock*& OutLabel, USlider*& OutSlider, const TCHAR* Label)
		{
			OutLabel = WidgetTree->ConstructWidget<UTextBlock>();
			OutLabel->SetText(FText::FromString(Label));
			OutLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 11));
			OutLabel->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.6f)));
			if (UVerticalBoxSlot* S = ColorBox->AddChildToVerticalBox(OutLabel))
				S->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));

			OutSlider = WidgetTree->ConstructWidget<USlider>();
			OutSlider->SetMinValue(0.f);
			OutSlider->SetMaxValue(1.f);
			if (UVerticalBoxSlot* S = ColorBox->AddChildToVerticalBox(OutSlider))
			{
				S->SetPadding(FMargin(0.f, 2.f, 0.f, 4.f));
				S->SetHorizontalAlignment(HAlign_Fill);
			}
		};
		MakeHueSlider(WhiteColorLabel, WhiteColorSlider, TEXT("White squares"));
		MakeHueSlider(BlackColorLabel, BlackColorSlider, TEXT("Black squares"));

		// Game-over overlay — centered, hidden until OnGameOver.
		GameOverPanel = WidgetTree->ConstructWidget<UVerticalBox>();
		GameOverPanel->SetVisibility(ESlateVisibility::Hidden);
		if (UCanvasPanelSlot* GS = Root->AddChildToCanvas(GameOverPanel))
		{
			GS->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			GS->SetAlignment(FVector2D(0.5f, 0.5f));
			GS->SetAutoSize(true);
		}

		GameOverText = WidgetTree->ConstructWidget<UTextBlock>();
		GameOverText->SetJustification(ETextJustify::Center);
		GameOverText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 32));
		GameOverText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.9f, 0.3f)));
		if (UVerticalBoxSlot* GTS = GameOverPanel->AddChildToVerticalBox(GameOverText))
		{
			GTS->SetPadding(FMargin(24.f, 16.f));
			GTS->SetHorizontalAlignment(HAlign_Center);
		}

		auto MakeButton = [&](UButton*& OutBtn, const TCHAR* Label)
		{
			OutBtn = WidgetTree->ConstructWidget<UButton>();
			UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>();
			T->SetText(FText::FromString(Label));
			T->SetJustification(ETextJustify::Center);
			OutBtn->AddChild(T);
			if (UVerticalBoxSlot* BS = GameOverPanel->AddChildToVerticalBox(OutBtn))
			{
				BS->SetPadding(FMargin(32.f, 8.f));
				BS->SetHorizontalAlignment(HAlign_Fill);
			}
		};
		MakeButton(PlayAgainButton, TEXT("Play Again"));
		MakeButton(MainMenuButton, TEXT("Main Menu"));
	}

	return Super::RebuildWidget();
}

void UChessHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// AddUniqueDynamic: NativeConstruct can re-fire if the HUD is ever re-added.
	if (PlayAgainButton) PlayAgainButton->OnClicked.AddUniqueDynamic(this, &UChessHudWidget::OnPlayAgainClicked);
	if (MainMenuButton)  MainMenuButton->OnClicked.AddUniqueDynamic(this, &UChessHudWidget::OnMainMenuClicked);

	if (AChessPlayerController* PC = GetChessPC())
		Manager = PC->Manager;
	if (!Manager)
		Manager = Cast<AChessManager>(UGameplayStatics::GetActorOfClass(this, AChessManager::StaticClass()));

	if (Manager)
		Manager->OnGameOver.AddUniqueDynamic(this, &UChessHudWidget::HandleGameOver);

	if (WhiteColorSlider) WhiteColorSlider->OnValueChanged.AddUniqueDynamic(this, &UChessHudWidget::OnWhiteColorChanged);
	if (BlackColorSlider) BlackColorSlider->OnValueChanged.AddUniqueDynamic(this, &UChessHudWidget::OnBlackColorChanged);

	// Sliders start where the player left them (SetValue doesn't fire OnValueChanged).
	if (const UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
	{
		if (WhiteColorSlider) WhiteColorSlider->SetValue(GI->LightSquareHue);
		if (BlackColorSlider) BlackColorSlider->SetValue(GI->DarkSquareHue);
	}
}

void UChessHudWidget::ApplyBoardColors()
{
	UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance());
	if (!GI) return;

	GI->bCustomBoardColors = true;
	if (WhiteColorSlider) GI->LightSquareHue = WhiteColorSlider->GetValue();
	if (BlackColorSlider) GI->DarkSquareHue  = BlackColorSlider->GetValue();

	AChessPlayerController* PC = GetChessPC();
	AChessBoard* Board = PC ? PC->Board : nullptr;
	if (Board)
		Board->SetSquareColors(
			UChessKidsGameInstance::LightColorFromHue(GI->LightSquareHue),
			UChessKidsGameInstance::DarkColorFromHue(GI->DarkSquareHue));
}

void UChessHudWidget::OnWhiteColorChanged(float Value) { ApplyBoardColors(); }
void UChessHudWidget::OnBlackColorChanged(float Value) { ApplyBoardColors(); }

void UChessHudWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Manager) return;
	if (Manager->IsGameOver()) return;   // frozen once the overlay is up

	// Undo after checkmate resumes the game (bGameOver resets) — take the overlay
	// back down and bring the turn indicator back.
	if (GameOverPanel && GameOverPanel->GetVisibility() == ESlateVisibility::Visible)
	{
		GameOverPanel->SetVisibility(ESlateVisibility::Hidden);
		if (TurnText) TurnText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	AChessPlayerController* PC = GetChessPC();
	const bool bTwoPlayer = Manager->IsTwoPlayerMode();
	const bool bWhiteToMove = Manager->GetActiveColor() == TEXT("white");
	const bool bThinking = !bTwoPlayer && ((PC && PC->IsAIThinking()) || !bWhiteToMove);

	if (TurnText)
	{
		const int32 TurnState = bTwoPlayer ? (bWhiteToMove ? 0 : 1) : (bThinking ? 3 : 2);
		if (TurnState != LastTurnState)
		{
			LastTurnState = TurnState;
			if (bTwoPlayer)
			{
				// P1 = White team, P2 = Black team: text in the team's color,
				// outlined with the ENEMY team's color so it pops either way.
				const FLinearColor WhiteTeam(0.95f, 0.95f, 0.90f);
				const FLinearColor BlackTeam(0.06f, 0.06f, 0.10f);
				const bool bP1 = bWhiteToMove;

				TurnText->SetText(FText::FromString(bP1 ? TEXT("P1's Turn") : TEXT("P2's Turn")));
				FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Bold", 28);
				Font.OutlineSettings = FFontOutlineSettings(3, bP1 ? BlackTeam : WhiteTeam);
				TurnText->SetFont(Font);
				TurnText->SetColorAndOpacity(FSlateColor(bP1 ? WhiteTeam : BlackTeam));
			}
			else
			{
				TurnText->SetText(FText::FromString(bThinking ? TEXT("Robot is thinking…") : TEXT("Your turn!")));
				TurnText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 24));
				TurnText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.95f, 1.f)));
			}
		}
	}

	if (CheckText)
	{
		const bool bInCheck = Manager->IsInCheck();
		CheckText->SetVisibility(bInCheck ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		if (bInCheck)
		{
			FString Warn;
			if (bTwoPlayer)
				Warn = bWhiteToMove ? TEXT("P1 — your King is in check!") : TEXT("P2 — your King is in check!");
			else
				Warn = bWhiteToMove ? TEXT("Careful — your King is in check!") : TEXT("Nice one — enemy King is in check!");
			CheckText->SetText(FText::FromString(Warn));
		}
	}
}

void UChessHudWidget::HandleGameOver(FString Result)
{
	if (GameOverText)
	{
		const bool bTwoPlayer = Manager && Manager->IsTwoPlayerMode();
		FString Msg;
		if (Result == TEXT("white"))      Msg = bTwoPlayer ? TEXT("P1 wins! Great game!") : TEXT("You win! Amazing game!");
		else if (Result == TEXT("black")) Msg = bTwoPlayer ? TEXT("P2 wins! Great game!") : TEXT("Good try! You'll get the robot next time!");
		else                              Msg = TEXT("It's a draw — what a battle!");
		GameOverText->SetText(FText::FromString(Msg));
	}
	if (TurnText)  TurnText->SetVisibility(ESlateVisibility::Hidden);
	if (CheckText) CheckText->SetVisibility(ESlateVisibility::Hidden);
	if (GameOverPanel) GameOverPanel->SetVisibility(ESlateVisibility::Visible);
}

void UChessHudWidget::OnPlayAgainClicked()
{
	if (AChessPlayerController* PC = GetChessPC())
		PC->RestartArena();
}

void UChessHudWidget::OnMainMenuClicked()
{
	if (AChessPlayerController* PC = GetChessPC())
		PC->QuitToMainMenu();
}

AChessPlayerController* UChessHudWidget::GetChessPC() const
{
	return Cast<AChessPlayerController>(GetOwningPlayer());
}
