// ChessCamera.cpp

#include "ChessCamera.h"
#include "ChessBoard.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AChessCamera::AChessCamera()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->bDoCollisionTest        = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch           = false;
	SpringArm->bInheritYaw             = false;
	SpringArm->bInheritRoll            = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void AChessCamera::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyView();
}

void AChessCamera::BeginPlay()
{
	Super::BeginPlay();
	ApplyView();

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC) PC->SetViewTargetWithBlend(this, 0.f);
}

void AChessCamera::ApplyView()
{
	if (IsValid(TargetBoard))
		SetActorLocation(TargetBoard->GetBoardCenter());

	SpringArm->TargetArmLength = ArmLength;
	SpringArm->SetRelativeRotation(FRotator(-PitchAngle, Yaw, 0.f));
}
