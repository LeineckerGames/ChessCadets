// ChessPlayerController.cpp

#include "ChessPlayerController.h"
#include "ChessBoard.h"

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
}

void AChessPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AChessPlayerController::OnSelect);
}

                         
void AChessPlayerController::OnSelect()
{
	FHitResult Hit;
	if(!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
		return;

	SelectedActor = Hit.GetActor();

	// if we clicked directly on the board, highlight whichever square was hit
	if (AChessBoard* ChessBoard = Cast<AChessBoard>(Hit.GetActor()))
	{
		FString Square;
		if (ChessBoard->WorldLocationToSquare(Hit.Location, Square))
			ChessBoard->SelectSquare(Square);
	}
}










