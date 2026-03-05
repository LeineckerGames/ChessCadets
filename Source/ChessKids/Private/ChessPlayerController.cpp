#include "ChessPlayerController.h"
#include "ChessBoard.h"
#include "ChessManager.h"
#include "ChessPiece.h"
#include "Kismet/GameplayStatics.h"

AChessPlayerController::AChessPlayerController()
{
	bShowMouseCursor = true;
}

void AChessPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);

	if (!Board)
		Board = Cast<AChessBoard>(UGameplayStatics::GetActorOfClass(this, AChessBoard::StaticClass()));
	if (!Manager)
		Manager = Cast<AChessManager>(UGameplayStatics::GetActorOfClass(this, AChessManager::StaticClass()));
}

void AChessPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AChessPlayerController::OnSelect);
}

void AChessPlayerController::OnSelect()
{
	if (!Board || !Manager) return;

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
		if (PieceStr.IsEmpty()) return;

		if (!FChar::IsUpper(PieceStr[0])) return;

		TArray<FString> LegalMoves = Manager->GetLegalMovesFromSquare(ClickedSquare);
		if (LegalMoves.Num() == 0) return;

		SelectedSquare = ClickedSquare;
		bPieceSelected = true;

		Board->SelectSquare(ClickedSquare);
		Board->ShowLegalMoveTargets(LegalMoves);
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
			TArray<FString> LegalMoves = Manager->GetLegalMovesFromSquare(ClickedSquare);
			if (LegalMoves.Num() > 0)
			{
				SelectedSquare = ClickedSquare;
				Board->SelectSquare(ClickedSquare);
				Board->ShowLegalMoveTargets(LegalMoves);
				return;
			}
		}

		FString MoveStr = SelectedSquare + ClickedSquare;
		bool bSuccess = Manager->MakeMove(MoveStr);
		bPieceSelected = false;

		if (bSuccess)
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, [this]()
			{
				if (Manager) Manager->RequestAIMove();
			}, 0.5f, false);
		}
	}
}
