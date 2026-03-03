// ChessBoard.cpp

#include "ChessBoard.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AChessBoard::AChessBoard()
{
	PrimaryActorTick.bCanEverTick = true; // needed for neon pulse

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AChessBoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildBoard();
	BuildCyberpunkFrame();
	RebuildNeonDMIs();
	SnapModelToBoard();
}

void AChessBoard::BeginPlay()
{
	Super::BeginPlay();
	SnapModelToBoard(); 
}

void AChessBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (NeonPulseSpeed <= 0.f || NeonDynMaterials.IsEmpty()) return;

	NeonPulseTime += DeltaTime * NeonPulseSpeed;

	const float T   = FMath::Sin(NeonPulseTime * PI * 2.f) * 0.5f + 0.5f;
	const float Val = FMath::Lerp(NeonPulseMin, NeonPulseMax, T);

	for (UMaterialInstanceDynamic* DMI : NeonDynMaterials)
		if (IsValid(DMI))
			DMI->SetScalarParameterValue(NeonPulseParamName, Val);
}


UStaticMeshComponent* AChessBoard::MakeMeshComp(
	AActor* Owner, const FName& Name, UStaticMesh* Mesh,
	USceneComponent* Parent, const FVector& LocalLoc, const FVector& LocalScale,
	UMaterialInterface* Mat)
{
	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Owner, Name);
	Comp->SetupAttachment(Parent);
	Comp->RegisterComponent();
	Comp->SetStaticMesh(Mesh);
	Comp->SetRelativeLocation(LocalLoc);
	Comp->SetRelativeScale3D(LocalScale);
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Mat) Comp->SetMaterial(0, Mat);
	return Comp;
}

//Board squares

void AChessBoard::BuildBoard()
{
	for (UStaticMeshComponent* M : SquareMeshes)   if (IsValid(M)) M->DestroyComponent();
	for (UStaticMeshComponent* M : HighlightMeshes) if (IsValid(M)) M->DestroyComponent();
	SquareMeshes.Empty();
	HighlightMeshes.Empty();

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!PlaneMesh) return;

	const float Scale     = SquareSize / 100.f;
	const float HalfBoard = 3.5f * SquareSize; 

	for (int32 Rank = 0; Rank < 8; ++Rank)
	{
		for (int32 File = 0; File < 8; ++File)
		{
			const FVector LocalPos(File * SquareSize - HalfBoard,
			                       Rank * SquareSize - HalfBoard, 0.f);
			const bool bLight = ((File + Rank) % 2 != 0);

			UStaticMeshComponent* Sq = MakeMeshComp(
				this, *FString::Printf(TEXT("Sq_%d_%d"), File, Rank),
				PlaneMesh, GetRootComponent(), LocalPos,
				FVector(Scale, Scale, 1.f),
				bLight ? LightSquareMaterial : DarkSquareMaterial);
			SquareMeshes.Add(Sq);

			UStaticMeshComponent* Hl = MakeMeshComp(
				this, *FString::Printf(TEXT("Hl_%d_%d"), File, Rank),
				PlaneMesh, GetRootComponent(), LocalPos + FVector(0.f, 0.f, 0.5f),
				FVector(Scale * 0.9f, Scale * 0.9f, 1.f), nullptr);
			Hl->SetVisibility(false);
			HighlightMeshes.Add(Hl);
		}
	}
}


void AChessBoard::BuildCyberpunkFrame()
{
	// wipe anything left from a previous OnConstruction before rebuilding
	for (UStaticMeshComponent* M : BorderWalls)   if (IsValid(M)) M->DestroyComponent();
	for (UStaticMeshComponent* M : CornerAccents) if (IsValid(M)) M->DestroyComponent();
	if (IsValid(GridOverlayMesh)) { GridOverlayMesh->DestroyComponent(); GridOverlayMesh = nullptr; }
	BorderWalls.Empty();
	CornerAccents.Empty();

	UStaticMesh* CubeMesh  = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!CubeMesh || !PlaneMesh) return;

	const float HalfBoard = 3.5f * SquareSize;      
	const float FullBoard = 8.f  * SquareSize;       
	const float BT        = BorderThickness;
	const float BH        = BorderHeight;

	// engine cubes are 100x100x100 by default so divide to get actual
	const float WallCenterZ = BH * 0.5f;             
	const float WallYPos    = HalfBoard + BT * 0.5f; // distance from board origin to wall centre

	const float LongScale = (FullBoard + 2.f * BT) / 100.f;
	const float ThickScale = BT / 100.f;
	const float HeightScale = BH / 100.f;

	BorderWalls.Add(MakeMeshComp(this, TEXT("Wall_PY"), CubeMesh, GetRootComponent(),
		FVector(0.f,  WallYPos, WallCenterZ),
		FVector(LongScale, ThickScale, HeightScale), BorderMaterial));

	BorderWalls.Add(MakeMeshComp(this, TEXT("Wall_NY"), CubeMesh, GetRootComponent(),
		FVector(0.f, -WallYPos, WallCenterZ),
		FVector(LongScale, ThickScale, HeightScale), BorderMaterial));

	const float InnerScale = FullBoard / 100.f;
	const float WallXPos   = HalfBoard + BT * 0.5f;

	BorderWalls.Add(MakeMeshComp(this, TEXT("Wall_PX"), CubeMesh, GetRootComponent(),
		FVector( WallXPos, 0.f, WallCenterZ),
		FVector(ThickScale, InnerScale, HeightScale), BorderMaterial));

	BorderWalls.Add(MakeMeshComp(this, TEXT("Wall_NX"), CubeMesh, GetRootComponent(),
		FVector(-WallXPos, 0.f, WallCenterZ),
		FVector(ThickScale, InnerScale, HeightScale), BorderMaterial));

	const float CA       = CornerAccentSize;
	const float CAScale  = CA / 100.f;
	const float CornerXY = HalfBoard + BT * 0.5f;    
	const float CornerZ  = BH + CA * 0.5f;           

	CornerAccents.Add(MakeMeshComp(this, TEXT("Corner_PP"), CubeMesh, GetRootComponent(),
		FVector( CornerXY,  CornerXY, CornerZ), FVector(CAScale, CAScale, CAScale), CornerAccentMaterial));
	CornerAccents.Add(MakeMeshComp(this, TEXT("Corner_PN"), CubeMesh, GetRootComponent(),
		FVector( CornerXY, -CornerXY, CornerZ), FVector(CAScale, CAScale, CAScale), CornerAccentMaterial));
	CornerAccents.Add(MakeMeshComp(this, TEXT("Corner_NP"), CubeMesh, GetRootComponent(),
		FVector(-CornerXY,  CornerXY, CornerZ), FVector(CAScale, CAScale, CAScale), CornerAccentMaterial));
	CornerAccents.Add(MakeMeshComp(this, TEXT("Corner_NN"), CubeMesh, GetRootComponent(),
		FVector(-CornerXY, -CornerXY, CornerZ), FVector(CAScale, CAScale, CAScale), CornerAccentMaterial));

	const float GridScale = FullBoard / 100.f;
	GridOverlayMesh = MakeMeshComp(this, TEXT("GridOverlay"), PlaneMesh, GetRootComponent(),
		FVector(0.f, 0.f, 1.f),
		FVector(GridScale, GridScale, 1.f), GridOverlayMaterial);
}

//Neon pulse dynamic material instances

void AChessBoard::RebuildNeonDMIs()
{
	NeonDynMaterials.Empty();
	if (NeonPulseSpeed <= 0.f) return;

	auto MakeDMI = [&](UStaticMeshComponent* Comp, UMaterialInterface* BaseMat)
	{
		if (!IsValid(Comp) || !IsValid(BaseMat)) return;
		UMaterialInstanceDynamic* DMI = UMaterialInstanceDynamic::Create(BaseMat, this);
		Comp->SetMaterial(0, DMI);
		NeonDynMaterials.Add(DMI);
	};

	for (UStaticMeshComponent* W : BorderWalls)   MakeDMI(W, BorderMaterial);
	for (UStaticMeshComponent* C : CornerAccents) MakeDMI(C, CornerAccentMaterial);
	if (IsValid(GridOverlayMesh))                  MakeDMI(GridOverlayMesh, GridOverlayMaterial);
}

void AChessBoard::SnapModelToBoard()
{
	if (!IsValid(SnappedModel)) return;

	const FVector WorldPos = GetActorTransform().TransformPosition(SnapOffset);
	SnappedModel->SetActorLocation(WorldPos);

	if (bSnapRotation)
		SnappedModel->SetActorRotation(GetActorRotation() + SnapRotation);
}

//Coordinate helpers

bool AChessBoard::ParseSquare(const FString& Str, int32& OutFile, int32& OutRank)
{
	if (Str.Len() < 2) return false;
	const TCHAR FileChar = FChar::ToLower(Str[0]);
	const TCHAR RankChar = Str[1];
	if (FileChar < TEXT('a') || FileChar > TEXT('h')) return false;
	if (RankChar  < TEXT('1') || RankChar  > TEXT('8')) return false;
	OutFile = FileChar - TEXT('a');
	OutRank = RankChar  - TEXT('1');
	return true;
}

FVector AChessBoard::FileRankToWorldLocation(int32 File, int32 Rank) const
{
	const float HalfBoard = 3.5f * SquareSize;
	const FVector Local(File * SquareSize - HalfBoard,
	                    Rank * SquareSize - HalfBoard, 0.f);
	return GetActorTransform().TransformPosition(Local);
}

FVector AChessBoard::SquareToWorldLocation(const FString& SquareStr) const
{
	int32 File, Rank;
	if (!ParseSquare(SquareStr, File, Rank)) return GetActorLocation();
	return FileRankToWorldLocation(File, Rank);
}

bool AChessBoard::SquareToFileRank(const FString& SquareStr, int32& OutFile, int32& OutRank) const
{
	return ParseSquare(SquareStr, OutFile, OutRank);
}

FString AChessBoard::FileRankToSquareName(int32 File, int32 Rank) const
{
	if (File < 0 || File > 7 || Rank < 0 || Rank > 7) return TEXT("??");
	return FString::Printf(TEXT("%c%c"), TEXT('a') + File, TEXT('1') + Rank);
}

FVector AChessBoard::GetBoardCenter() const
{
	return GetActorLocation();
}

//highlighting

UStaticMeshComponent* AChessBoard::GetHighlightMesh(int32 File, int32 Rank) const
{
	if (File < 0 || File > 7 || Rank < 0 || Rank > 7) return nullptr;
	const int32 Idx = File + Rank * 8;
	return HighlightMeshes.IsValidIndex(Idx) ? HighlightMeshes[Idx] : nullptr;
}

void AChessBoard::ClearHighlights()
{
	for (UStaticMeshComponent* Hl : HighlightMeshes)
		if (IsValid(Hl)) Hl->SetVisibility(false);
}

void AChessBoard::SelectSquare(const FString& SquareStr)
{
	ClearHighlights();
	int32 File, Rank;
	if (!ParseSquare(SquareStr, File, Rank)) return;

	UStaticMeshComponent* Hl = GetHighlightMesh(File, Rank);
	if (!Hl) return;
	if (SelectionMaterial) Hl->SetMaterial(0, SelectionMaterial);
	Hl->SetVisibility(true);
}

void AChessBoard::ShowLegalMoveTargets(const TArray<FString>& Squares)
{
	for (const FString& Sq : Squares)
	{
		int32 File, Rank;
		if (!ParseSquare(Sq, File, Rank)) continue;

		UStaticMeshComponent* Hl = GetHighlightMesh(File, Rank);
		if (!Hl) continue;
		if (LegalMoveMaterial) Hl->SetMaterial(0, LegalMoveMaterial);
		Hl->SetVisibility(true);
	}
}

void AChessBoard::SelectSquareAndShowMoves(const FString& SquareStr, const TArray<FString>& LegalMoves)
{
	SelectSquare(SquareStr);
	ShowLegalMoveTargets(LegalMoves);
}
