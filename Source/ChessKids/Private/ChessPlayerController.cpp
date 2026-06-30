#include "ChessPlayerController.h"
#include "ChessBoard.h"
#include "ChessManager.h"
#include "ChessPiece.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.h"

AChessPlayerController::AChessPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AChessPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);

	if (!Board)
		Board = Cast<AChessBoard>(UGameplayStatics::GetActorOfClass(this, AChessBoard::StaticClass()));
	if (!Manager)
		Manager = Cast<AChessManager>(UGameplayStatics::GetActorOfClass(this, AChessManager::StaticClass()));
}

void AChessPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear the AI delay timer — without this it can fire on a freed controller
	// during PIE-stop or level transitions.
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(AITimerHandle);

	Super::EndPlay(EndPlayReason);
}

void AChessPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleHover();
}

void AChessPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AChessPlayerController::OnSelect);

	// Escape toggles pause. In PIE the editor eats Escape to stop play, so P is a
	// second pause key that works in PIE and as a player alternative.
	FInputKeyBinding& EscBinding = InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AChessPlayerController::TogglePause);
	EscBinding.bExecuteWhenPaused = true;
	FInputKeyBinding& PBinding = InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AChessPlayerController::TogglePause);
	PBinding.bExecuteWhenPaused = true;
}

void AChessPlayerController::HandleHover()
{
	if (!Board || !Manager) return;

	FHitResult Hit;
	bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (!bHit)
	{
		if (!HoveredSquare.IsEmpty())
		{
			Board->ClearHover();
			HoveredSquare.Empty();
		}
		return;
	}

	FString Square;
	bool bParsed = Board->WorldLocationToSquare(Hit.Location, Square);

	if (!bParsed) return;
	if (Square == HoveredSquare) return;

	Board->ClearHover();
	HoveredSquare = Square;
	Board->HoverSquare(Square);
}

void AChessPlayerController::SelectPieceOnSquare(const FString& Square)
{
	TArray<FString> LegalMoves = Manager->GetLegalMovesFromSquare(Square);
	if (LegalMoves.Num() == 0) return;

	SelectedSquare = Square;
	bPieceSelected = true;
	Board->SelectSquare(Square);
	Board->ShowLegalMoveTargets(LegalMoves);
}

void AChessPlayerController::OnDeselect()
{
	if (!bPieceSelected) return;
	bPieceSelected = false;
	HoveredSquare.Empty();
	Board->ClearHighlights();
}

void AChessPlayerController::OnSelect()
{
	if (bPaused) return;   // ignore board clicks while the pause menu is up
	if (!Board || !Manager || bIsAIThinking) return;
	if (Manager->GetActiveColor() != TEXT("white")) return;

	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
		return;

	FString ClickedSquare;
	if (!Board->WorldLocationToSquare(Hit.Location, ClickedSquare))
		return;

	if (!bPieceSelected)
	{
		FString PieceStr = Manager->GetPieceOnSquare(ClickedSquare);
		if (PieceStr.IsEmpty() || !FChar::IsUpper(PieceStr[0])) return;
		SelectPieceOnSquare(ClickedSquare);
	}
	else
	{
		Board->ClearHighlights();

		if (ClickedSquare == SelectedSquare)
		{
			bPieceSelected = false;
			return;
		}

		FString PieceStr = Manager->GetPieceOnSquare(ClickedSquare);
		if (!PieceStr.IsEmpty() && FChar::IsUpper(PieceStr[0]))
		{
			SelectPieceOnSquare(ClickedSquare);
			return;
		}

		FString MoveStr = SelectedSquare + ClickedSquare;
		bPieceSelected = false;

		// Check promotion for both white and black
		FString PromoPieceStr = Manager->GetPieceOnSquare(SelectedSquare);
		bool bIsWhitePromotion = (PromoPieceStr == TEXT("P") && ClickedSquare[1] == '8');
		bool bIsBlackPromotion = (PromoPieceStr == TEXT("p") && ClickedSquare[1] == '1');

		if (bIsWhitePromotion || bIsBlackPromotion)
		{
			PendingMoveStr = MoveStr;
			bAwaitingPromotion = true;

			// Don't call MakeMove yet — wait for user choice
			if (PromotionPickerClass)
			{
				PromotionPickerWidget = CreateWidget<UUserWidget>(this, PromotionPickerClass);
				if (PromotionPickerWidget)
				{
					PromotionPickerWidget->AddToViewport();
					FInputModeUIOnly UIMode;
					SetInputMode(UIMode);
				}
			}
			return;
		}
		bool bSuccess = Manager->MakeMove(MoveStr);
		//bPieceSelected = false;

		if (bSuccess)
		{
			bIsAIThinking = true;
			TWeakObjectPtr<AChessPlayerController> WeakThis(this);
			GetWorldTimerManager().SetTimer(AITimerHandle, [WeakThis]()
			{
				AChessPlayerController* Self = WeakThis.Get();
				if (!Self) return;
				if (IsValid(Self->Manager)) Self->Manager->RequestAIMove();
				Self->bIsAIThinking = false;
			}, 0.5f, false);
		}
	}
}

// ---------------------------------------------------------------------------
// Pause menu
// ---------------------------------------------------------------------------

void AChessPlayerController::TogglePause()
{
	if (bPaused)
		ResumeGame();
	else
		ShowPauseMenu();
}

void AChessPlayerController::ShowPauseMenu()
{
	if (bPaused) return;

	if (!PauseMenuInstance)
	{
		UClass* MenuClass = PauseMenuClass.LoadSynchronous();
		if (!MenuClass) MenuClass = UPauseMenuWidget::StaticClass();   // C++ fallback UI
		PauseMenuInstance = CreateWidget<UUserWidget>(this, MenuClass);
	}
	if (!PauseMenuInstance) return;

	if (!PauseMenuInstance->IsInViewport())
		PauseMenuInstance->AddToViewport(50);

	UGameplayStatics::SetGamePaused(this, true);
	bPaused = true;
	bShowMouseCursor = true;

	// Modal: UI-only input so clicks can't fall through to the chessboard behind the
	// menu (which is what made the game react / "unpause" when clicking buttons).
	PauseMenuInstance->SetIsFocusable(true);
	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(PauseMenuInstance->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void AChessPlayerController::HidePauseMenu()
{
	if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
		PauseMenuInstance->RemoveFromParent();

	UGameplayStatics::SetGamePaused(this, false);
	bPaused = false;

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
	bShowMouseCursor = true;
}

void AChessPlayerController::ResumeGame()
{
	HidePauseMenu();
}

void AChessPlayerController::RestartArena()
{
	UGameplayStatics::SetGamePaused(this, false);
	const FString Current = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*Current));
}

void AChessPlayerController::QuitToMainMenu()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, MainMenuMapName);
}

// ---------------------------------------------------------------------------
// Promotion (from main)
// ---------------------------------------------------------------------------

void AChessPlayerController::CompletePromotion(const FString& PieceLetter)
{
	if (!bAwaitingPromotion) return;
	bAwaitingPromotion = false;

	if (PromotionPickerWidget)
	{
		PromotionPickerWidget->RemoveFromParent();
		PromotionPickerWidget = nullptr;
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
	}

	// Make the move with the correct promotion piece
	FString MoveStr = PendingMoveStr + PieceLetter; // e.g. "e7e8n"
	bool bSuccess = Manager->MakeMove(MoveStr);
	UE_LOG(LogTemp, Warning, TEXT("Promotion MakeMove result: %s, Move: %s"),
		bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"), *MoveStr);

	if (bSuccess)
	{
		Manager->RefreshBoard();
		bIsAIThinking = true;
		TWeakObjectPtr<AChessPlayerController> WeakThis(this);
		GetWorldTimerManager().SetTimer(AITimerHandle, [WeakThis]()
			{
				AChessPlayerController* Self = WeakThis.Get();
				if (!Self) return;
				if (IsValid(Self->Manager)) Self->Manager->RequestAIMove();
				Self->bIsAIThinking = false;
			}, 0.5f, false);
	}
}

void AChessPlayerController::ResetTurnState()
{
	UE_LOG(LogTemp, Warning, TEXT("ResetTurnState called!"));
	bIsAIThinking = false;
	bAwaitingPromotion = false;
	bPieceSelected = false;
	if (PromotionPickerWidget)
	{
		PromotionPickerWidget->RemoveFromParent();
		PromotionPickerWidget = nullptr;
	}
}
