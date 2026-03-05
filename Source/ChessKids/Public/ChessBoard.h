#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessBoard.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPointLightComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	UMaterialInterface* LightSquareMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	UMaterialInterface* DarkSquareMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	UMaterialInterface* LegalMoveMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Board")
	UMaterialInterface* SelectionMaterial = nullptr;

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

	// Edge lights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Lights")
	FLinearColor EdgeLightColor = FLinearColor(0.f, 0.7f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Lights")
	float EdgeLightIntensity = 800.f;

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
	FVector FileRankToWorldLocation(int32 File, int32 Rank) const;

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

	UFUNCTION(BlueprintCallable, Category = "Chess|Board")
	void SelectSquareAndShowMoves(const FString& SquareStr, const TArray<FString>& LegalMoves);

	UFUNCTION(BlueprintCallable, Category = "Chess|Board")
	void ClearHighlights();

private:
	UPROPERTY() TArray<UStaticMeshComponent*> SquareMeshes;
	UPROPERTY() TArray<UStaticMeshComponent*> HighlightMeshes;
	UPROPERTY() UStaticMeshComponent* GridOverlayMesh = nullptr;
	UPROPERTY() UStaticMeshComponent* ScanPlaneMesh = nullptr;
	UPROPERTY() TArray<UPointLightComponent*> EdgeLights;
	UPROPERTY() TArray<UMaterialInstanceDynamic*> NeonDynMaterials;

	float NeonPulseTime = 0.f;
	float ScanOffset = 0.f;

	void BuildBoard();
	void BuildHolographicFrame();
	void RebuildNeonDMIs();
	void SnapModelToBoard();

	UStaticMeshComponent* GetHighlightMesh(int32 File, int32 Rank) const;
	static bool ParseSquare(const FString& Str, int32& OutFile, int32& OutRank);
	static UStaticMeshComponent* MakeMeshComp(
		AActor* Owner, const FName& Name, UStaticMesh* Mesh,
		USceneComponent* Parent, const FVector& LocalLoc, const FVector& LocalScale,
		UMaterialInterface* Mat);
};
