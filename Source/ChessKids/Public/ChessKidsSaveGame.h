#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ChessKidsSaveGame.generated.h"

// Persistent player progress (slot "ChessKidsProgress"). Loaded by the
// GameInstance on Init, saved whenever something unlocks.
UCLASS()
class CHESSKIDS_API UChessKidsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Highest story chapter available (1 = L_Pawn ... 5 = L_Royalty).
	UPROPERTY()
	int32 UnlockedStoryChapters = 1;

	// Lesson indices (0 = Pawn ... 5 = King) the player has finished.
	UPROPERTY()
	TArray<int32> CompletedTutorials;
};
