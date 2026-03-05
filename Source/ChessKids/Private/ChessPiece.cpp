#include "ChessPiece.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AChessPiece::AChessPiece()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PieceMesh"));
	MeshComp->SetupAttachment(Root);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AChessPiece::Init(EChessPieceType Type, EChessColor Color, int32 File, int32 Rank)
{
	PieceType = Type;
	PieceColor = Color;
	BoardFile = File;
	BoardRank = Rank;
	SetupMeshAndMaterial();
}

void AChessPiece::SetupMeshAndMaterial()
{
	const TCHAR* MeshPath = nullptr;
	FVector Scale = FVector(0.4f, 0.4f, 0.4f);

	switch (PieceType)
	{
	case EChessPieceType::Pawn:
		MeshPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
		Scale = FVector(0.25f, 0.25f, 0.25f);
		break;
	case EChessPieceType::Rook:
		MeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
		Scale = FVector(0.3f, 0.3f, 0.4f);
		break;
	case EChessPieceType::Knight:
		MeshPath = TEXT("/Engine/BasicShapes/Cone.Cone");
		Scale = FVector(0.3f, 0.3f, 0.45f);
		break;
	case EChessPieceType::Bishop:
		MeshPath = TEXT("/Engine/BasicShapes/Cone.Cone");
		Scale = FVector(0.25f, 0.25f, 0.5f);
		break;
	case EChessPieceType::Queen:
		MeshPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
		Scale = FVector(0.3f, 0.3f, 0.55f);
		break;
	case EChessPieceType::King:
		MeshPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
		Scale = FVector(0.35f, 0.35f, 0.6f);
		break;
	}

	if (MeshPath)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
		if (Mesh)
			MeshComp->SetStaticMesh(Mesh);
	}

	MeshComp->SetRelativeScale3D(Scale);
	MeshComp->SetRelativeLocation(FVector(0.f, 0.f, Scale.Z * 50.f));

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
