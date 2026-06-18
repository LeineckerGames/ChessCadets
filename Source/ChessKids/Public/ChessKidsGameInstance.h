#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ChessKidsGameInstance.generated.h"

class UChessMapRegistry;

// Game instance that owns the arena list and performs level travel for the
// map picker. Set as GameInstanceClass in DefaultEngine.ini.
UCLASS()
class CHESSKIDS_API UChessKidsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UChessKidsGameInstance();

	virtual void Init() override;

	// Registry of selectable arenas. Assign DA_MapRegistry in the class defaults
	// of the BP_ChessKidsGameInstance asset (or set on the C++ default).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chess|Maps")
	TSoftObjectPtr<UChessMapRegistry> MapRegistry;

	// Travels to the arena registered under MapId. Returns false if the id is unknown
	// or the map is locked.
	UFUNCTION(BlueprintCallable, Category = "Chess|Maps")
	bool OpenMap(FName MapId);

	// Travels directly to a level asset, bypassing the registry.
	UFUNCTION(BlueprintCallable, Category = "Chess|Maps")
	void OpenLevelAsset(const TSoftObjectPtr<UWorld>& Level);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess|Maps")
	bool IsMapUnlocked(FName MapId) const;

	UFUNCTION(BlueprintCallable, Category = "Chess|Maps")
	void UnlockMap(FName MapId);

private:
	UChessMapRegistry* LoadRegistry() const;

	// Maps unlocked this session. Seeded from entries flagged bUnlockedByDefault.
	UPROPERTY()
	TSet<FName> UnlockedMaps;
};
