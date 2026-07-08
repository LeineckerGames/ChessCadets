#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialBoard.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class CHESSKIDS_API ATutorialBoard : public AActor
{
	GENERATED_BODY()

public:
	ATutorialBoard();
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	int32 NumFiles = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	int32 NumRanks = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board", meta = (ClampMin = "10"))
	float SquareSize = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	FLinearColor LightSquareColor = FLinearColor(0.93f, 0.89f, 0.79f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	FLinearColor DarkSquareColor = FLinearColor(0.09f, 0.10f, 0.16f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	UMaterialInterface* LegalMoveMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	UMaterialInterface* SelectionMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	UMaterialInterface* HoverMaterial = nullptr;

	// Optional: darken/disable specific squares (e.g. bishop's wrong-color squares)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	UMaterialInterface* DisabledSquareMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board")
	float HighlightZOffset = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Board",
	          meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float HighlightScaleFactor = 0.9f;

	// Coordinate helpers
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial|Board")
	FVector GridToWorldLocation(int32 File, int32 Rank, float ZOffset = 0.f) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial|Board")
	bool WorldToGrid(FVector WorldLoc, int32& OutFile, int32& OutRank) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial|Board")
	bool IsValidSquare(int32 File, int32 Rank) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial|Board")
	FVector GetBoardCenter() const;

	// Highlighting
	UFUNCTION(BlueprintCallable, Category = "Tutorial|Board")
	void HighlightSquare(int32 File, int32 Rank, UMaterialInterface* Material);

	UFUNCTION(BlueprintCallable, Category = "Tutorial|Board")
	void ShowLegalMoves(const TArray<FIntPoint>& Squares);

	UFUNCTION(BlueprintCallable, Category = "Tutorial|Board")
	void SelectSquare(int32 File, int32 Rank);

	UFUNCTION(BlueprintCallable, Category = "Tutorial|Board")
	void ClearHighlights();

	// Recolor a board square at runtime (e.g. darken unreachable squares, paint visited ones)
	UFUNCTION(BlueprintCallable, Category = "Tutorial|Board")
	void TintSquare(int32 File, int32 Rank, FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Tutorial|Board")
	void DisableSquare(int32 File, int32 Rank);

	UFUNCTION(BlueprintCallable, Category = "Tutorial|Board")
	void DisableSquares(const TArray<FIntPoint>& Squares);

	UFUNCTION(BlueprintCallable, Category = "Tutorial|Board")
	void GlideActorToSquare(AActor* ActorToMove, int32 File, int32 Rank, float ZOffset = 0.f);

private:
	UPROPERTY() TArray<UStaticMeshComponent*> SquareMeshes;
	UPROPERTY() TArray<UStaticMeshComponent*> HighlightMeshes;
	UPROPERTY() UMaterialInstanceDynamic* LightTintDMI = nullptr;
	UPROPERTY() UMaterialInstanceDynamic* DarkTintDMI = nullptr;

	void BuildBoard();
	void EnsureTintDMIs();
	void AutoLoadMaterials();
	UStaticMeshComponent* GetHighlightMesh(int32 File, int32 Rank) const;
	int32 GridIndex(int32 File, int32 Rank) const;
};
