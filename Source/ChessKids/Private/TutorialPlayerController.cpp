#include "TutorialPlayerController.h"
#include "TutorialBoard.h"
#include "TutorialManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "GameFramework/Pawn.h"
#include "TutorialHudWidget.h"
#include "Blueprint/UserWidget.h"

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

	// The game mode's default pawn renders as a floating sphere and its collision
	// blocks cursor traces near the board. Keep it possessed (destroying it makes
	// the view fall back to nowhere) but hide it and turn off its collision.
	if (APawn* P = GetPawn())
	{
		P->SetActorHiddenInGame(true);
		P->SetActorEnableCollision(false);
	}

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

	// Frame the lesson board automatically: no camera actor exists in these maps,
	// so without this the view is a spectator floating at PlayerStart.
	if (Board)
	{
		const float Extent = FMath::Max(Board->NumFiles, Board->NumRanks) * Board->SquareSize;
		const FVector Center = Board->GetActorLocation();
		const FVector CamLoc = Center + FVector(0.f, -Extent * 0.85f, Extent * 1.25f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (ACameraActor* Cam = GetWorld()->SpawnActor<ACameraActor>(
			CamLoc, (Center - CamLoc).Rotation(), Params))
		{
			SetViewTargetWithBlend(Cam, 0.f);
		}
	}

	// Tutorial overlay: guide speech panel + MENU button back to the main menu.
	if (UUserWidget* Hud = CreateWidget<UUserWidget>(this, UTutorialHudWidget::StaticClass()))
		Hud->AddToViewport(10);
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
