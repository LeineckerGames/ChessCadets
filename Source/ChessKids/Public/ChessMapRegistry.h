#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChessMapRegistry.generated.h"

class UWorld;
class UTexture2D;

// One selectable arena. Authored in DA_MapRegistry, consumed by the map picker UI
// and UChessKidsGameInstance::OpenMap.
USTRUCT(BlueprintType)
struct FChessMapEntry
{
	GENERATED_BODY()

	// Stable identifier used by OpenMap(FName). Must be unique within the registry.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Map")
	FName MapId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Map")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Map", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Map")
	TObjectPtr<UTexture2D> Thumbnail = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Map")
	bool bUnlockedByDefault = true;
};

// Data-driven list of playable arenas. Create one instance (DA_MapRegistry) under
// Content/Data and point UChessKidsGameInstance at it.
UCLASS(BlueprintType)
class CHESSKIDS_API UChessMapRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Maps")
	TArray<FChessMapEntry> Maps;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Maps")
	bool FindMap(FName MapId, FChessMapEntry& OutEntry) const;
};
