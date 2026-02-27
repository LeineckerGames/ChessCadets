// VegetationSpawner.h
// Drop one or more of these actors on L_Field to procedurally scatter foliage.
// All settings are editable in the Details panel — no blueprint needed.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "VegetationSpawner.generated.h"

USTRUCT(BlueprintType)
struct FVegetationEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "500"))
	int32 Count = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleMin = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleMax = 1.3f;
};

UCLASS()
class CHESSKIDS_API AVegetationSpawner : public AActor
{
	GENERATED_BODY()

public:
	AVegetationSpawner();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Spawner")
	TArray<FVegetationEntry> SpawnEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Spawner", meta = (ClampMin = "100"))
	float SpawnRadius = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Spawner", meta = (ClampMin = "0"))
	float ExclusionRadius = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Spawner")
	bool bAlignToSurface = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Spawner")
	bool bRandomYaw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Spawner", meta = (ClampMin = "1", ClampMax = "20"))
	int32 MaxPlacementAttempts = 10;

	UFUNCTION(CallInEditor, Category = "Vegetation Spawner")
	void SpawnMeshes();

	UFUNCTION(CallInEditor, Category = "Vegetation Spawner")
	void ClearSpawnedMeshes();

private:
	UPROPERTY()
	TArray<AActor*> SpawnedActors;

	bool TryGetGroundLocation(const FVector& Origin, float Radius, FVector& OutLocation, FRotator& OutRotation) const;
};
