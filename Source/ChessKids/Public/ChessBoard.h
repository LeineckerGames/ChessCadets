#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessBoard.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class CHESSKIDS_API AChessBoard : public AActor
{
	GENERATED_BODY()

public:
	AChessBoard();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk")
	UMaterialInterface* BorderMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk")
	UMaterialInterface* GridOverlayMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk")
	UMaterialInterface* CornerAccentMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk", meta = (ClampMin = "0"))
	float BorderHeight = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk", meta = (ClampMin = "1"))
	float BorderThickness = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk", meta = (ClampMin = "1"))
	float CornerAccentSize = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk",
	          meta = (ClampMin = "0", ClampMax = "10"))
	float NeonPulseSpeed = 1.f; // 0 = off

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk")
	FName NeonPulseParamName = TEXT("EmissiveBoost"); // must match your material

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk")
	float NeonPulseMin = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Cyberpunk")
	float NeonPulseMax = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Snap")
	AActor* SnappedModel = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Snap")
	FVector SnapOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Snap")
	bool bSnapRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Snap",
	          meta = (EditCondition = "bSnapRotation"))
	FRotator SnapRotation = FRotator::ZeroRotator;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	FVector SquareToWorldLocation(const FString& SquareStr) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	FVector FileRankToWorldLocation(int32 File, int32 Rank) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	bool SquareToFileRank(const FString& SquareStr, int32& OutFile, int32& OutRank) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	FString FileRankToSquareName(int32 File, int32 Rank) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Board")
	FVector GetBoardCenter() const;

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
	UPROPERTY() TArray<UStaticMeshComponent*> BorderWalls;
	UPROPERTY() TArray<UStaticMeshComponent*> CornerAccents;
	UPROPERTY() UStaticMeshComponent* GridOverlayMesh = nullptr;
	UPROPERTY() TArray<UMaterialInstanceDynamic*> NeonDynMaterials;

	float NeonPulseTime = 0.f;

	void BuildBoard();
	void BuildCyberpunkFrame();
	void RebuildNeonDMIs();
	void SnapModelToBoard();

	UStaticMeshComponent* GetHighlightMesh(int32 File, int32 Rank) const;
	static bool ParseSquare(const FString& Str, int32& OutFile, int32& OutRank);
	static UStaticMeshComponent* MakeMeshComp(
		AActor* Owner, const FName& Name, UStaticMesh* Mesh,
		USceneComponent* Parent, const FVector& LocalLoc, const FVector& LocalScale,
		UMaterialInterface* Mat);
};
