#include "TutorialPlayerController.h"
#include "TutorialBoard.h"
#include "TutorialManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ATutorialPlayerController::ATutorialPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ATutorialPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));

	for (TActorIterator<ATutorialBoard> It(GetWorld()); It; ++It)
	{
		Board = *It;
		break;
	}
	for (TActorIterator<ATutorialManager> It(GetWorld()); It; ++It)
	{
		Manager = *It;
		break;
	}

	UE_LOG(LogTemp, Warning, TEXT("TutorialPC: Board=%s Manager=%s"),
		Board ? TEXT("FOUND") : TEXT("NULL"),
		Manager ? TEXT("FOUND") : TEXT("NULL"));
}

void ATutorialPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ATutorialPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		OnClick();
	}
}

void ATutorialPlayerController::OnClick()
{
	if (!Board || !Manager) return;

	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialPC: No hit under cursor"));
		return;
	}

	int32 File, Rank;
	if (!Board->WorldToGrid(Hit.ImpactPoint, File, Rank))
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialPC: Hit at %s but not on grid"), *Hit.ImpactPoint.ToString());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("TutorialPC: Clicked grid [%d, %d]"), File, Rank);
	Manager->OnSquareClicked(File, Rank);
}
