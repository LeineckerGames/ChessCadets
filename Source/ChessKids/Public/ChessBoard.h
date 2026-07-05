#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessBoard.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPointLightComponent;

USTRUCT(BlueprintType)
struct FBoardTheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Theme")
	FName DisplayName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Theme")
	UMaterialInterface* LightSquareMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Theme")
	UMaterialInterface* DarkSquareMaterial = nullptr;
};

UCLASS()
class CHESSKIDS_API AChessBoard : public AActor
{
	GENERATED_BODY()

public:
	AChessBoard();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Board
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board", meta = (ClampMin = "10"))
	float SquareSize = 100.f;

	// THE way to color the board: squares render as tinted instances of
	// M_SquareTint driven by these two colors. Edit them here or drag the
	// in-game HUD sliders — same path. The material slots below are only a
	// fallback for when the tint material is missing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	FLinearColor LightSquareColor = FLinearColor(0.93f, 0.89f, 0.79f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	FLinearColor DarkSquareColor = FLinearColor(0.09f, 0.10f, 0.16f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board", AdvancedDisplay)
	UMaterialInterface* LightSquareMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board", AdvancedDisplay)
	UMaterialInterface* DarkSquareMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	UMaterialInterface* LegalMoveMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	UMaterialInterface* SelectionMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	UMaterialInterface* HoverMaterial = nullptr;

	// Board color themes — drives the in-game slider.
	// Populate this on the AChessBoard placed in the level. Index 0 = default theme.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Themes")
	TArray<FBoardTheme> BoardThemes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Themes", meta = (ClampMin = "0"))
	int32 CurrentThemeIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	float HighlightZOffset = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board",
	          meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float HighlightScaleFactor = 0.9f;

	// Holographic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic")
	UMaterialInterface* GridOverlayMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic")
	UMaterialInterface* HolographicScanMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic",
	          meta = (ClampMin = "0", ClampMax = "10"))
	float NeonPulseSpeed = 1.f; // 0 = off

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic")
	FName NeonPulseParamName = TEXT("EmissiveBoost"); // must match your material

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic")
	float NeonPulseMin = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic")
	float NeonPulseMax = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic")
	float ScanSpeed = 40.f; // cm/s, 0 = off

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic")
	float ScanHeight = 80.f; // how high above the board the scan travels before looping

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic")
	float GridOverlayZOffset = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Holographic",
	          meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float ScanPlaneZScale = 0.05f;

	// Edge lights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Lights")
	FLinearColor EdgeLightColor = FLinearColor(0.f, 0.7f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Lights")
	float EdgeLightIntensity = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Lights",
	          meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float EdgeLightAttenuationScale = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Lights")
	bool bEdgeLightInverseSquaredFalloff = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Lights")
	float EdgeLightHeight = 5.f;

	// Model snap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Snap")
	AActor* SnappedModel = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Snap")
	FVector SnapOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Snap")
	bool bSnapRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Snap",
	          meta = (EditCondition = "bSnapRotation"))
	FRotator SnapRotation = FRotator::ZeroRotator;

	// Coordinate helpers
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	FVector SquareToWorldLocation(const FString& SquareStr) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	bool WorldLocationToSquare(FVector WorldLoc, FString& OutSquare) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	FVector FileRankToWorldLocation(int32 File, int32 Rank, float Zoffset = 0.f) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	bool SquareToFileRank(const FString& SquareStr, int32& OutFile, int32& OutRank) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	FString FileRankToSquareName(int32 File, int32 Rank) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	FVector GetBoardCenter() const;

	// Highlighting
	UFUNCTION(BlueprintCallable, Category = "Chess|Board")
	void SelectSquare(const FString& SquareStr);

	UFUNCTION(BlueprintCallable, Category = "Chess|Board")
	void ShowLegalMoveTargets(const TArray<FString>& Squares);

	/*UFUNCTION(BlueprintCallable, Category = "Chess|Board")
	void SelectSquareAndShowMoves(const FString& SquareStr, const TArray<FString>& LegalMoves);*/

	UFUNCTION(BlueprintCallable, Category = "Chess|Board")
	void ClearHighlights();

	UFUNCTION(BlueprintCallable, Category = "Chess|Board")  
	void HoverSquare(const FString& SquareStr);             

	UFUNCTION(BlueprintCallable, Category = "Chess|Board")  
	void ClearHover();      
	
	UFUNCTION(BlueprintCallable, Category="Chess|Snap")
	void SnapActorToSquare(AActor* ActorToSnap, int32 File, int32 Rank, float ZOffset = 0.f,bool bSnapRot = false, FRotator Rot = FRotator::ZeroRotator);

	// Like SnapActorToSquare but chess pieces glide smoothly instead of teleporting.
	// Non-piece actors (and spawns) still snap instantly.
	UFUNCTION(BlueprintCallable, Category="Chess|Snap")
	void GlideActorToSquare(AActor* ActorToMove, int32 File, int32 Rank, float ZOffset = 0.f);

	// Free recoloring — used by the settings hue sliders. Tints the squares via
	// dynamic instances of M_SquareTint; any color combination works, no themes needed.
	UFUNCTION(BlueprintCallable, Category = "Chess|Themes")
	void SetSquareColors(FLinearColor LightColor, FLinearColor DarkColor);

	// Theme API — used by the in-game color slider.
	// Swaps square materials in place; does not rebuild board geometry.
	UFUNCTION(BlueprintCallable, Category = "Chess|Themes")
	void SetBoardTheme(int32 ThemeIndex);

	// Convenience for UMG sliders: maps a 0..1 value to a theme index.
	UFUNCTION(BlueprintCallable, Category = "Chess|Themes")
	void SetBoardThemeBySliderValue(float Value);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Themes")
	int32 GetNumBoardThemes() const { return BoardThemes.Num(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Themes")
	int32 GetCurrentThemeIndex() const { return CurrentThemeIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Themes")
	FName GetCurrentThemeName() const;

private:
	UPROPERTY() TArray<UStaticMeshComponent*> SquareMeshes;
	UPROPERTY() TArray<UStaticMeshComponent*> HighlightMeshes;
	UPROPERTY() UStaticMeshComponent* GridOverlayMesh = nullptr;
	UPROPERTY() UStaticMeshComponent* ScanPlaneMesh = nullptr;
	UPROPERTY() TArray<UPointLightComponent*> EdgeLights;
	UPROPERTY() TArray<UMaterialInstanceDynamic*> NeonDynMaterials;
	UPROPERTY() UMaterialInstanceDynamic* LightTintDMI = nullptr;
	UPROPERTY() UMaterialInstanceDynamic* DarkTintDMI = nullptr;

	FString HoveredSquare;  

	float NeonPulseTime = 0.f;
	float ScanOffset = 0.f;

	void BuildBoard();
	void BuildHolographicFrame();
	void RebuildNeonDMIs();
	void EnsureTintDMIs();
	//void SnapModelToBoard();

	UStaticMeshComponent* GetHighlightMesh(int32 File, int32 Rank) const;
	static bool ParseSquare(const FString& Str, int32& OutFile, int32& OutRank);
	static UStaticMeshComponent* MakeMeshComp(
		AActor* Owner, const FName& Name, UStaticMesh* Mesh,
		USceneComponent* Parent, const FVector& LocalLoc, const FVector& LocalScale,
		UMaterialInterface* Mat);
};
