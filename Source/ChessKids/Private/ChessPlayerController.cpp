#include "ChessPlayerController.h"
#include "ChessBoard.h"
#include "ChessManager.h"
#include "ChessPiece.h"
#include "Kismet/GameplayStatics.h"

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

void AChessPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleHover();
}

void AChessPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AChessPlayerController::OnSelect);
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
		bool bSuccess = Manager->MakeMove(MoveStr);
		bPieceSelected = false;

		if (bSuccess)
		{
			bIsAIThinking = true;
			GetWorldTimerManager().SetTimer(AITimerHandle, [this]()
			{
				if (Manager) Manager->RequestAIMove();
				bIsAIThinking = false;
			}, 0.5f, false);
		}
	}
}
