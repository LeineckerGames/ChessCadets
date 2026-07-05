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

	// AI difficulty chosen by the player (1=Easy, 2=Medium, 3=Hard). Lives here so the
	// choice survives level travel — AChessManager::BeginPlay reads it on every arena load.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Game", meta = (ClampMin = "1", ClampMax = "3"))
	int32 Difficulty = 1;

	// Local two-player (hot-seat) mode: both colors are human, no AI. Read by
	// AChessManager::BeginPlay on arena load; set from menus / the pause toggle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Game")
	bool bTwoPlayerMode = false;

	// Board color theme chosen via the settings slider. Applied by
	// AChessBoard::BeginPlay so the choice follows the player between arenas.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Game", meta = (ClampMin = "0"))
	int32 BoardThemeIndex = 0;

	// Free board recoloring via the settings hue sliders (overrides themes when set).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Game")
	bool bCustomBoardColors = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Game", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightSquareHue = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Game", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DarkSquareHue = 0.62f;

	// Shared hue->color mapping so the board and the settings UI always agree:
	// light squares stay pastel-bright, dark squares stay deep, whatever the hue.
	static FLinearColor LightColorFromHue(float Hue01)
	{
		return FLinearColor::MakeFromHSV8((uint8)(FMath::Clamp(Hue01, 0.f, 1.f) * 255.f), 90, 242);
	}
	static FLinearColor DarkColorFromHue(float Hue01)
	{
		return FLinearColor::MakeFromHSV8((uint8)(FMath::Clamp(Hue01, 0.f, 1.f) * 255.f), 178, 64);
	}

private:
	UChessMapRegistry* LoadRegistry() const;

	// Maps unlocked this session. Seeded from entries flagged bUnlockedByDefault.
	UPROPERTY()
	TSet<FName> UnlockedMaps;
};
