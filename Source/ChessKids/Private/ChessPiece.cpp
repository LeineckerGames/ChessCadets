#include "ChessPiece.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"


AChessPiece::AChessPiece()
{
	// Tick exists only to drive the move glide; it stays disabled until StartGlide.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PieceMesh"));
	MeshComp->SetupAttachment(Root);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AChessPiece::StartGlide(const FVector& TargetLocation, float Duration)
{
	// Moving ends the selection hover; the glide starts from wherever the piece
	// currently floats and lands on the destination square.
	bSelectedHover = false;
	HoverAlpha = 0.f;

	GlideStart = GetActorLocation();
	GlideTarget = TargetLocation;
	GlideElapsed = 0.f;
	GlideDuration = FMath::Max(Duration, 0.01f);
	bGliding = true;
	SetActorTickEnabled(true);
}

void AChessPiece::SetSelected(bool bSelected)
{
	if (bSelectedHover == bSelected) return;
	bSelectedHover = bSelected;

	if (bSelected)
	{
		// Lift from where the piece rests (or where it's headed if mid-glide).
		HoverBase = bGliding ? GlideTarget : GetActorLocation();
		HoverTime = 0.f;
		SetActorTickEnabled(true);
	}
	// Deselect: Tick eases HoverAlpha back down and restores HoverBase exactly.
}

void AChessPiece::StartCaptureFling()
{
	bGliding = false;   // a capture overrides any move in progress
	bFlinging = true;
	FlingElapsed = 0.f;
	FlingStartScale = GetActorScale3D();
	SetActorEnableCollision(false);   // don't block cursor traces on the way out
	SetActorTickEnabled(true);
}

void AChessPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFlinging)
	{
		FlingElapsed += DeltaTime;
		const float Alpha = FMath::Clamp(FlingElapsed / 0.45f, 0.f, 1.f);
		AddActorWorldOffset(FVector(0.f, 0.f, 320.f * DeltaTime));
		SetActorScale3D(FlingStartScale * (1.f - FMath::InterpEaseIn(0.f, 1.f, Alpha, 2.f)));
		if (Alpha >= 1.f)
			Destroy();
		return;
	}

	// Selection hover: quick lift, then a gentle bob until deselected.
	// (Skipped while gliding — the glide owns the location until it lands.)
	if (!bGliding && (bSelectedHover || HoverAlpha > 0.f))
	{
		HoverTime += DeltaTime;
		const float Target = bSelectedHover ? 1.f : 0.f;
		HoverAlpha = FMath::FInterpConstantTo(HoverAlpha, Target, DeltaTime, 1.f / 0.15f);

		const float Bob = FMath::Sin(HoverTime * 2.f * PI * 1.4f) * 6.f;
		SetActorLocation(HoverBase + FVector(0.f, 0.f, HoverAlpha * (24.f + Bob)));

		if (!bSelectedHover && HoverAlpha <= 0.f)
			SetActorLocation(HoverBase);   // land exactly where we lifted from
		return;
	}

	if (!bGliding) { SetActorTickEnabled(false); return; }

	GlideElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(GlideElapsed / GlideDuration, 0.f, 1.f);
	const float Eased = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);

	FVector Pos = FMath::Lerp(GlideStart, GlideTarget, Eased);

	// A small hop makes the move readable; knights leap higher, as they should.
	const float HopHeight = (PieceType == EChessPieceType::Knight) ? 30.f : 8.f;
	Pos.Z += FMath::Sin(Alpha * PI) * HopHeight;

	SetActorLocation(Pos);

	if (Alpha >= 1.f)
	{
		SetActorLocation(GlideTarget);
		bGliding = false;
		SetActorTickEnabled(false);
	}
}

void AChessPiece::Init(EChessPieceType Type, EChessColor Color, int32 File, int32 Rank, const FPieceMeshConfig* MeshOverride)
{
	PieceType = Type;
	PieceColor = Color;
	BoardFile = File;
	BoardRank = Rank;
	SetupMeshAndMaterial(MeshOverride);
}

void AChessPiece::SetupMeshAndMaterial(const FPieceMeshConfig* MeshOverride)
{
	if (MeshOverride && MeshOverride->Mesh)
	{
		MeshComp->SetSkeletalMesh(MeshOverride->Mesh);
		MeshComp->SetRelativeScale3D(MeshOverride->Scale);
		MeshComp->SetRelativeLocation(FVector(0.f, 0.f, MeshOverride->ZOffset));

		if (MeshOverride->MaterialOverride)
		{
			MeshComp->SetMaterial(0, MeshOverride->MaterialOverride);
		}
		return;
	}

	const TCHAR* MeshPath = nullptr;
	FVector Scale = FVector(0.4f, 0.4f, 0.4f);

	switch (PieceType)
	{
	case EChessPieceType::Pawn:
		MeshPath = TEXT("/Game/Blueprints/Models/pawn__2_.pawn__2_");
		Scale = FVector(0.45f, 0.45f, 0.45f);
		break;
	case EChessPieceType::Rook:
		MeshPath = TEXT("/Game/Blueprints/Models/rook.rook");
		Scale = FVector(0.5f, 0.5f, 0.5f);
		break;
	case EChessPieceType::Knight:
		MeshPath = TEXT("/Game/Blueprints/Models/knight.knight");
		Scale = FVector(0.5f, 0.5f, 0.5f);
		break;
	case EChessPieceType::Bishop:
		MeshPath = TEXT("/Game/Blueprints/Models/bishop.bishop");
		Scale = FVector(0.5f, 0.5f, 0.5f);
		break;
	case EChessPieceType::Queen:
		MeshPath = TEXT("/Game/Blueprints/Models/queen.queen");
		Scale = FVector(0.5f, 0.5f, 0.65f);
		break;
	case EChessPieceType::King:
		MeshPath = TEXT("/Game/Blueprints/Models/king.king");
		Scale = FVector(0.5f, 0.5f, 0.70f);
		break;
	}

	USkeletalMesh* Mesh = MeshPath ? LoadObject<USkeletalMesh>(nullptr, MeshPath) : nullptr;
	if (Mesh)
		MeshComp->SetSkeletalMesh(Mesh);

	MeshComp->SetRelativeScale3D(Scale);

	// Seat the piece on the board: offset so the mesh's lowest point rests at the actor
	// origin (the square surface), regardless of where the skeletal mesh's pivot sits.
	float MeshBottomLocal = 0.f;
	if (Mesh)
	{
		const FBoxSphereBounds B = Mesh->GetBounds();
		MeshBottomLocal = B.Origin.Z - B.BoxExtent.Z;
	}
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -MeshBottomLocal * Scale.Z));

	if (PieceColor == EChessColor::White)
	{
		MeshComp->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	}

	UMaterialInstanceDynamic* DMI = UMaterialInstanceDynamic::Create(
		MeshComp->GetMaterial(0), this);
	if (DMI)
	{
		FLinearColor Color = PieceColor == EChessColor::White
			? FLinearColor(0.9f, 0.9f, 0.85f)
			: FLinearColor(0.15f, 0.15f, 0.2f);
		DMI->SetVectorParameterValue(TEXT("BaseColor"), Color);
		MeshComp->SetMaterial(0, DMI);
	}
}
