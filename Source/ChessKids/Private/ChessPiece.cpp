#include "ChessPiece.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"


AChessPiece::AChessPiece()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PieceMesh"));
	MeshComp->SetupAttachment(Root);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
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

	if (MeshPath)
	{
		USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, MeshPath);
		if (Mesh)
			MeshComp->SetSkeletalMesh(Mesh);
	}

	MeshComp->SetRelativeScale3D(Scale);
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, Scale.Z * 50.f));

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
