#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiece.h"
#include "TutorialManager.generated.h"

class ATutorialBoard;
class AChessPiece;

UENUM(BlueprintType)
enum class ETutorialPhase : uint8
{
	Phase1,
	Phase2,
	Phase3,
	Boss,
	Complete
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, ETutorialPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuideMessage, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTryAgain);

UCLASS()
class CHESSKIDS_API ATutorialManager : public AActor
{
	GENERATED_BODY()

public:
	ATutorialManager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Events for UI to bind to
	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnGuideMessage OnGuideMessage;

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnLevelComplete OnLevelComplete;

	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnTryAgain OnTryAgain;

	// Set in editor — which board this manager controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	ATutorialBoard* Board = nullptr;

	// The piece type this level teaches
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	EChessPieceType TaughtPiece = EChessPieceType::Pawn;

	// Level to travel to a few seconds after this lesson completes
	// (chains the tutorials: Pawn -> Rook -> ... -> King -> Axiom). None = stay.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName NextLevelName = NAME_None;

	// Current state
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	ETutorialPhase CurrentPhase = ETutorialPhase::Phase1;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	int32 TargetsCollected = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	int32 TargetsTotal = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	int32 MoveCount = 0;

	// Player piece position on the grid
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	int32 PlayerFile = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	int32 PlayerRank = 0;

	// Called when the player clicks a square
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void OnSquareClicked(int32 File, int32 Rank);

	// Advance to next phase
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void AdvancePhase();

	// Reset current phase
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ResetPhase();

	// Show guide dialogue
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ShowGuideMessage(const FString& Message);

	// Check if a move is legal for the taught piece
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial")
	TArray<FIntPoint> GetLegalMoves() const;

protected:
	// Override in subclasses for level-specific logic
	virtual void SetupPhase(ETutorialPhase Phase);
	virtual void OnTargetCollected(int32 File, int32 Rank);
	virtual void OnPhaseComplete();
	virtual void AfterPlayerMove(int32 File, int32 Rank);
	virtual bool IsMoveAllowed(int32 FromFile, int32 FromRank, int32 ToFile, int32 ToRank) const;

	// Piece mesh config for spawning player piece
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial|Pieces")
	TMap<EChessPieceType, FPieceMeshConfig> PieceMeshes;

	// Player piece actor
	UPROPERTY() AChessPiece* PlayerPiece = nullptr;

	// Target positions for current phase
	TArray<FIntPoint> TargetPositions;

	// Enemy positions for current phase
	TArray<FIntPoint> EnemyPositions;

	// Tracks whether the player's pawn has moved (for two-square first move)
	bool bPlayerHasMoved = false;

	// Blocks input during the delay between phase completion and the next phase's setup
	bool bPhaseTransitionPending = false;

	void SpawnPlayerPiece(int32 File, int32 Rank);
	void MovePlayerTo(int32 File, int32 Rank);
	void SpawnTargetAt(int32 File, int32 Rank);
	void RemoveTargetAt(int32 File, int32 Rank);
	void SpawnEnemyAt(int32 File, int32 Rank);
	void RemoveEnemyAt(int32 File, int32 Rank);
	void SpawnWallAt(int32 File, int32 Rank);
	bool IsSquareBlocked(int32 File, int32 Rank) const;
	void UpdateHighlights();

protected:
	UPROPERTY() TMap<int32, AChessPiece*> TargetActors;
	UPROPERTY() TMap<int32, AChessPiece*> EnemyActors;

	// Walls: block movement, can't be captured, don't count toward phase completion
	UPROPERTY() TMap<int32, AActor*> WallActors;

	int32 PackPos(int32 File, int32 Rank) const;
};
