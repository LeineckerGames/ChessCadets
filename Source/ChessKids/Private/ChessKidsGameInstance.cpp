#include "ChessKidsGameInstance.h"
#include "ChessMapRegistry.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/PackageName.h"

UChessKidsGameInstance::UChessKidsGameInstance()
{
	// Default the registry to the conventional asset path so OpenMap works without
	// a Blueprint subclass. Resolves silently once DA_MapRegistry is created.
	static ConstructorHelpers::FObjectFinder<UChessMapRegistry> RegistryFinder(
		TEXT("/Game/Data/DA_MapRegistry.DA_MapRegistry"));
	if (RegistryFinder.Succeeded())
	{
		MapRegistry = RegistryFinder.Object;
	}
}

void UChessKidsGameInstance::Init()
{
	Super::Init();

	if (UChessMapRegistry* Registry = LoadRegistry())
	{
		for (const FChessMapEntry& Entry : Registry->Maps)
		{
			if (Entry.bUnlockedByDefault && !Entry.MapId.IsNone())
			{
				UnlockedMaps.Add(Entry.MapId);
			}
		}
	}
}

UChessMapRegistry* UChessKidsGameInstance::LoadRegistry() const
{
	if (MapRegistry.IsNull())
	{
		return nullptr;
	}
	return MapRegistry.LoadSynchronous();
}

bool UChessKidsGameInstance::OpenMap(FName MapId)
{
	UChessMapRegistry* Registry = LoadRegistry();
	if (!Registry)
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenMap: no MapRegistry assigned on GameInstance."));
		return false;
	}

	FChessMapEntry Entry;
	if (!Registry->FindMap(MapId, Entry))
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenMap: unknown map id '%s'."), *MapId.ToString());
		return false;
	}

	if (!IsMapUnlocked(MapId))
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenMap: map '%s' is locked."), *MapId.ToString());
		return false;
	}

	OpenLevelAsset(Entry.Level);
	return true;
}

void UChessKidsGameInstance::OpenLevelAsset(const TSoftObjectPtr<UWorld>& Level)
{
	if (Level.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenLevelAsset: level reference is empty."));
		return;
	}

	const FString PackageName = FPackageName::ObjectPathToPackageName(Level.ToString());
	if (PackageName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenLevelAsset: could not resolve package name from '%s'."), *Level.ToString());
		return;
	}

	UGameplayStatics::OpenLevel(this, FName(*PackageName));
}

bool UChessKidsGameInstance::IsMapUnlocked(FName MapId) const
{
	return UnlockedMaps.Contains(MapId);
}

void UChessKidsGameInstance::UnlockMap(FName MapId)
{
	if (!MapId.IsNone())
	{
		UnlockedMaps.Add(MapId);
	}
}
