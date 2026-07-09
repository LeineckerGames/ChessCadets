// ChessBoard.cpp

#include "ChessBoard.h"
#include "ChessPiece.h"
#include "ChessKidsGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

// Contrast guarantee: whatever color reaches the board (sliders, details panel,
// scripts), light squares are forced light and dark squares forced dark — the
// value bands never overlap, so the checkerboard always reads.
static FLinearColor ClampBoardColor(const FLinearColor& In, bool bLight)
{
	FLinearColor HSV = In.LinearRGBToHSV();   // R = hue, G = sat, B = value
	if (bLight)
	{
		HSV.B = FMath::Max(HSV.B, 0.85f);
		HSV.G = FMath::Min(HSV.G, 0.50f);
	}
	else
	{
		HSV.B = FMath::Min(HSV.B, 0.28f);
		HSV.G = FMath::Max(HSV.G, 0.40f);
	}
	return HSV.HSVToLinearRGB();
}

AChessBoard::AChessBoard()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AChessBoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildBoard();
	BuildHolographicFrame();
	RebuildNeonDMIs();
	//SnapModelToBoard();

	// Apply currently selected theme (no-op when BoardThemes is empty)
	if (BoardThemes.IsValidIndex(CurrentThemeIndex))
		SetBoardTheme(CurrentThemeIndex);
}

void AChessBoard::BeginPlay()
{
	Super::BeginPlay();
	//SnapModelToBoard(); // re-snap at runtime in case the actor was moved after OnConstruction

	// Re-apply the player's saved board colors (settings sliders) — custom hues
	// win over themes when set.
	if (const UChessKidsGameInstance* GI = Cast<UChessKidsGameInstance>(GetGameInstance()))
	{
		if (GI->bCustomBoardColors)
			SetSquareColors(UChessKidsGameInstance::LightColorFromHue(GI->LightSquareHue),
			                UChessKidsGameInstance::DarkColorFromHue(GI->DarkSquareHue));
		else if (BoardThemes.IsValidIndex(GI->BoardThemeIndex) && GI->BoardThemeIndex != CurrentThemeIndex)
			SetBoardTheme(GI->BoardThemeIndex);
	}
}

void AChessBoard::SetSquareColors(FLinearColor LightColor, FLinearColor DarkColor)
{
	// Contrast guarantee applies to every caller.
	LightColor = ClampBoardColor(LightColor, true);
	DarkColor  = ClampBoardColor(DarkColor, false);

	LightSquareColor = LightColor;
	DarkSquareColor  = DarkColor;

	if (LightTintDMI && DarkTintDMI)
	{
		// Fast path: squares already re-skinned to the tint DMIs — just retint.
		LightTintDMI->SetVectorParameterValue(TEXT("Color"), LightColor);
		DarkTintDMI->SetVectorParameterValue(TEXT("Color"), DarkColor);
		return;
	}

	// First recolor this session: create the DMIs and take over the squares.
	EnsureTintDMIs();
	if (!LightTintDMI || !DarkTintDMI) return;

	for (int32 Idx = 0; Idx < SquareMeshes.Num(); ++Idx)
	{
		UStaticMeshComponent* Sq = SquareMeshes[Idx];
		if (!IsValid(Sq)) continue;
		const int32 File = Idx % 8;
		const int32 Rank = Idx / 8;
		const bool bLight = ((File + Rank) % 2 != 0);
		Sq->SetMaterial(0, bLight ? LightTintDMI : DarkTintDMI);
	}
}

void AChessBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// neon pulse
	if (NeonPulseSpeed > 0.f && !NeonDynMaterials.IsEmpty())
	{
		NeonPulseTime += DeltaTime * NeonPulseSpeed;
		const float T   = FMath::Sin(NeonPulseTime * PI * 2.f) * 0.5f + 0.5f;
		const float Val = FMath::Lerp(NeonPulseMin, NeonPulseMax, T);
		for (UMaterialInstanceDynamic* DMI : NeonDynMaterials)
			if (IsValid(DMI))
				DMI->SetScalarParameterValue(NeonPulseParamName, Val);
	}

	// holographic scan plane ping-pongs: sweeps up, then back down — no snap-reset
	if (IsValid(ScanPlaneMesh) && ScanSpeed > 0.f)
	{
		ScanOffset = FMath::Fmod(ScanOffset + DeltaTime * ScanSpeed, ScanHeight * 2.f);
		const float PingPong = (ScanOffset <= ScanHeight)
			? ScanOffset                          // rising leg
			: (ScanHeight * 2.f - ScanOffset);    // falling leg
		ScanPlaneMesh->SetRelativeLocation(FVector(0.f, 0.f, GridOverlayZOffset + PingPong));
	}
}

// one helper so we don't copy-paste attach+register+mesh+mat for every piece

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

void AChessBoard::EnsureTintDMIs()
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/Materials/BoardThemes/M_SquareTint.M_SquareTint"));
	if (!Base)
	{
		LightTintDMI = nullptr;
		DarkTintDMI = nullptr;
		return;
	}
	LightTintDMI = UMaterialInstanceDynamic::Create(Base, this);
	DarkTintDMI  = UMaterialInstanceDynamic::Create(Base, this);
	LightTintDMI->SetVectorParameterValue(TEXT("Color"), ClampBoardColor(LightSquareColor, true));
	DarkTintDMI->SetVectorParameterValue(TEXT("Color"), ClampBoardColor(DarkSquareColor, false));
}

void AChessBoard::BuildBoard()
{
	for (UStaticMeshComponent* M : SquareMeshes)    if (IsValid(M)) M->DestroyComponent();
	for (UStaticMeshComponent* M : HighlightMeshes) if (IsValid(M)) M->DestroyComponent();
	SquareMeshes.Empty();
	HighlightMeshes.Empty();

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!PlaneMesh) return;

	// Colors ARE the board's look: squares are tinted instances of M_SquareTint,
	// driven by LightSquareColor/DarkSquareColor (details panel or in-game sliders).
	// The material slots are only a fallback when the tint material is missing.
	EnsureTintDMIs();
	UMaterialInterface* LightMat = LightTintDMI ? (UMaterialInterface*)LightTintDMI : LightSquareMaterial;
	UMaterialInterface* DarkMat  = DarkTintDMI  ? (UMaterialInterface*)DarkTintDMI  : DarkSquareMaterial;

	const float Scale     = SquareSize / 100.f;
	const float HalfBoard = 3.5f * SquareSize;

	for (int32 Rank = 0; Rank < 8; ++Rank)
	{
		for (int32 File = 0; File < 8; ++File)
		{
			const FVector LocalPos(File * SquareSize - HalfBoard,
			                       HalfBoard - Rank * SquareSize, 0.f);
			const bool bLight = ((File + Rank) % 2 != 0);

			UStaticMeshComponent* Sq = MakeMeshComp(
				this, *FString::Printf(TEXT("Sq_%d_%d"), File, Rank),
				PlaneMesh, GetRootComponent(), LocalPos,
				FVector(Scale, Scale, 1.f),
				bLight ? LightMat : DarkMat);
			SquareMeshes.Add(Sq); // register square so SetBoardTheme() can re-skin it in place
			Sq->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // ← only square meshes get collision

			UStaticMeshComponent* Hl = MakeMeshComp(
				this, *FString::Printf(TEXT("Hl_%d_%d"), File, Rank),
				PlaneMesh, GetRootComponent(), LocalPos + FVector(0.f, 0.f, HighlightZOffset),
				FVector(Scale * HighlightScaleFactor, Scale * HighlightScaleFactor, 5.f), nullptr);
			Hl->SetVisibility(false);
			HighlightMeshes.Add(Hl);
		}
	}
}

//Holographic frame

void AChessBoard::BuildHolographicFrame()
{
	// wipe anything from a previous OnConstruction
	for (UPointLightComponent* L : EdgeLights) if (IsValid(L)) L->DestroyComponent();
	EdgeLights.Empty();
	if (IsValid(GridOverlayMesh)) { GridOverlayMesh->DestroyComponent(); GridOverlayMesh = nullptr; }
	if (IsValid(ScanPlaneMesh))   { ScanPlaneMesh->DestroyComponent();   ScanPlaneMesh   = nullptr; }
	ScanOffset = 0.f;

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!PlaneMesh) return;

	const float FullBoard = 8.f * SquareSize;
	const float GridScale = FullBoard / 100.f;
	const float BoardEdge = 4.f * SquareSize; // outer edge of the board

	// neon grid overlay over the whole 8x8 surface — hidden when no material is
	// assigned (a bare plane would render opaque default-gray and cover the squares)
	GridOverlayMesh = MakeMeshComp(this, TEXT("GridOverlay"), PlaneMesh, GetRootComponent(),
		FVector(0.f, 0.f, GridOverlayZOffset), FVector(GridScale, GridScale, 1.f), GridOverlayMaterial);
	GridOverlayMesh->SetVisibility(GridOverlayMaterial != nullptr);

	ScanPlaneMesh = MakeMeshComp(this, TEXT("ScanPlane"), PlaneMesh, GetRootComponent(),
		FVector(0.f, 0.f, GridOverlayZOffset), FVector(GridScale, GridScale, ScanPlaneZScale), HolographicScanMaterial);
	ScanPlaneMesh->SetVisibility(HolographicScanMaterial != nullptr);

	// one point light per edge, sitting just above the board surface
	auto AddLight = [&](FName Name, FVector Pos)
	{
		UPointLightComponent* L = NewObject<UPointLightComponent>(this, Name);
		L->SetupAttachment(GetRootComponent());
		L->RegisterComponent();
		L->SetRelativeLocation(Pos);
		L->SetLightColor(EdgeLightColor);
		L->Intensity = EdgeLightIntensity;
		L->AttenuationRadius = FullBoard * EdgeLightAttenuationScale;
		L->bUseInverseSquaredFalloff = bEdgeLightInverseSquaredFalloff;
		EdgeLights.Add(L);
	};

	AddLight(TEXT("Light_PY"), FVector(0.f,        BoardEdge, EdgeLightHeight));
	AddLight(TEXT("Light_NY"), FVector(0.f,       -BoardEdge, EdgeLightHeight));
	AddLight(TEXT("Light_PX"), FVector( BoardEdge, 0.f,       EdgeLightHeight));
	AddLight(TEXT("Light_NX"), FVector(-BoardEdge, 0.f,       EdgeLightHeight));
}

//Neon DMIs

void AChessBoard::RebuildNeonDMIs()
{
	NeonDynMaterials.Empty();
	if (NeonPulseSpeed <= 0.f) return;

	// local helper — create DMI, swap it onto the comp, add to tracking list
	auto MakeDMI = [&](UStaticMeshComponent* Comp, UMaterialInterface* BaseMat)
	{
		if (!IsValid(Comp) || !IsValid(BaseMat)) return;
		UMaterialInstanceDynamic* DMI = UMaterialInstanceDynamic::Create(BaseMat, this);
		Comp->SetMaterial(0, DMI);
		NeonDynMaterials.Add(DMI);
	};

	if (IsValid(GridOverlayMesh)) MakeDMI(GridOverlayMesh, GridOverlayMaterial);
	if (IsValid(ScanPlaneMesh))   MakeDMI(ScanPlaneMesh,   HolographicScanMaterial);
}

/*void AChessBoard::SnapModelToBoard()
{
	if (!IsValid(SnappedModel)) return;

	// SnapOffset is in board-local space so a positive Z still means "above the board"
	// regardless of how the board itself is rotated in the world.
	const FVector WorldPos = GetActorTransform().TransformPosition(SnapOffset);
	SnappedModel->SetActorLocation(WorldPos);

	if (bSnapRotation)
		SnappedModel->SetActorRotation(GetActorRotation() + SnapRotation);
}*/

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

FVector AChessBoard::FileRankToWorldLocation(int32 File, int32 Rank, float ZOffset) const
{
	if (File < 0 || File > 7 || Rank < 0 || Rank > 7)
		return FVector::ZeroVector;

	const float HalfBoard = 3.5f * SquareSize;

	const FVector Local(
		File * SquareSize - HalfBoard,
		HalfBoard - Rank * SquareSize,
		ZOffset
	);

	return GetActorTransform().TransformPosition(Local);
}

FVector AChessBoard::SquareToWorldLocation(const FString& SquareStr) const
{
	int32 File, Rank;
	if (!ParseSquare(SquareStr, File, Rank)) return GetActorLocation();
	return FileRankToWorldLocation(File, Rank);
}

bool AChessBoard::WorldLocationToSquare(FVector WorldLoc, FString& OutSquare) const
{
	// transform into board-local space first so rotation doesn't break the math
	const FVector Local = GetActorTransform().InverseTransformPosition(WorldLoc);
	const int32 File = FMath::FloorToInt((Local.X + 4.f * SquareSize) / SquareSize);
	const int32 Rank = FMath::FloorToInt((4.f * SquareSize - Local.Y) / SquareSize);
	if (File < 0 || File > 7 || Rank < 0 || Rank > 7) return false;
	OutSquare = FileRankToSquareName(File, Rank);
	return true;
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

//Highlighting

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

void AChessBoard::SnapActorToSquare(AActor* ActorToSnap, int32 File, int32 Rank, float ZOffset,
                                   bool bSnapRot, FRotator Rot)
{
    if (!ActorToSnap) return;

    const FVector Target = FileRankToWorldLocation(File, Rank, ZOffset);
    const FVector Before = ActorToSnap->GetActorLocation();

    ActorToSnap->SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);

    const FVector After = ActorToSnap->GetActorLocation();

    /*UE_LOG(LogTemp, Warning, TEXT("Snap %s: (%d,%d,%.1f) Before=%s After=%s Target=%s"),
        *GetNameSafe(ActorToSnap), File, Rank, ZOffset,
        *Before.ToString(), *After.ToString(), *Target.ToString());*/
}

void AChessBoard::GlideActorToSquare(AActor* ActorToMove, int32 File, int32 Rank, float ZOffset)
{
	if (!ActorToMove) return;

	const FVector Target = FileRankToWorldLocation(File, Rank, ZOffset);
	if (AChessPiece* Piece = Cast<AChessPiece>(ActorToMove))
		Piece->StartGlide(Target);
	else
		ActorToMove->SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);
}

// Themes

void AChessBoard::SetBoardTheme(int32 ThemeIndex)
{
	if (!BoardThemes.IsValidIndex(ThemeIndex)) return;

	const FBoardTheme& Theme = BoardThemes[ThemeIndex];
	CurrentThemeIndex = ThemeIndex;

	// Mirror onto the canonical properties so existing logic (selection clear, etc.) stays consistent.
	if (Theme.LightSquareMaterial) LightSquareMaterial = Theme.LightSquareMaterial;
	if (Theme.DarkSquareMaterial)  DarkSquareMaterial  = Theme.DarkSquareMaterial;

	// Re-skin existing square meshes in place — avoids rebuilding the board (which would
	// drop highlights, holographic frame DMIs, and any snapped pieces).
	for (int32 Idx = 0; Idx < SquareMeshes.Num(); ++Idx)
	{
		UStaticMeshComponent* Sq = SquareMeshes[Idx];
		if (!IsValid(Sq)) continue;

		const int32 File = Idx % 8;
		const int32 Rank = Idx / 8;
		const bool bLight = ((File + Rank) % 2 != 0);
		UMaterialInterface* Mat = bLight ? Theme.LightSquareMaterial : Theme.DarkSquareMaterial;
		if (Mat) Sq->SetMaterial(0, Mat);
	}
}

void AChessBoard::SetBoardThemeBySliderValue(float Value)
{
	const int32 N = BoardThemes.Num();
	if (N <= 0) return;

	const float Clamped = FMath::Clamp(Value, 0.f, 1.f);
	const int32 Idx = FMath::Clamp(FMath::RoundToInt(Clamped * (N - 1)), 0, N - 1);
	SetBoardTheme(Idx);
}

FName AChessBoard::GetCurrentThemeName() const
{
	return BoardThemes.IsValidIndex(CurrentThemeIndex)
		? BoardThemes[CurrentThemeIndex].DisplayName
		: NAME_None;
}

void AChessBoard::HoverSquare(const FString& SquareStr)
{
    int32 File, Rank;
    if (!ParseSquare(SquareStr, File, Rank)) return;

    UStaticMeshComponent* Hl = GetHighlightMesh(File, Rank);
    if (!Hl) return;

    if (HoverMaterial) Hl->SetMaterial(0, HoverMaterial);
    Hl->SetVisibility(true);

    HoveredSquare = SquareStr;
}

void AChessBoard::ClearHover()
{
    if (HoveredSquare.IsEmpty()) return;

    int32 File, Rank;
    if (!ParseSquare(HoveredSquare, File, Rank))
    {
        HoveredSquare.Empty();
        return;
    }

    UStaticMeshComponent* Hl = GetHighlightMesh(File, Rank);

    // Only hide it if it's purely a hover highlight — not a selection or legal move
    if (IsValid(Hl) && Hl->GetMaterial(0) == HoverMaterial)
        Hl->SetVisibility(false);

    HoveredSquare.Empty();
}