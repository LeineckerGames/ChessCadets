// ChessManager.h
// Wraps the embedded Pulse chess engine and exposes a clean UE5 interface.
// Place one AChessManager actor in the level to manage a chess game.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessManager.generated.h"

// Forward declare engine types to avoid pulling STL into UE5 headers
namespace pulse
{
	class Search;
	class Position;
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIMoveReady,   FString, FromSquare, FString, ToSquare);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnGameOver,      FString, Result);   // "white", "black", "draw"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoveMade,      FString, FromSquare, FString, ToSquare);

UCLASS()
class CHESSKIDS_API AChessManager : public AActor
{
	GENERATED_BODY()

public:
	AChessManager();
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	// ---------------------------------------------------------------
	// Events — bind these in Blueprint to react to engine output
	// ---------------------------------------------------------------

	// Fires when the AI has chosen its move
	UPROPERTY(BlueprintAssignable, Category = "Chess")
	FOnAIMoveReady OnAIMoveReady;

	// Fires when the game ends
	UPROPERTY(BlueprintAssignable, Category = "Chess")
	FOnGameOver OnGameOver;

	// Fires whenever any move is committed to the position
	UPROPERTY(BlueprintAssignable, Category = "Chess")
	FOnMoveMade OnMoveMade;

	// ---------------------------------------------------------------
	// Settings
	// ---------------------------------------------------------------

	// Search depth for AI (higher = stronger but slower; 4-6 is comfortable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess", meta = (ClampMin = "1", ClampMax = "20"))
	int32 AISearchDepth = 5;

	// ---------------------------------------------------------------
	// Game control
	// ---------------------------------------------------------------

	// Reset to the standard starting position
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void NewGame();

	// Load a position from a FEN string
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void SetPositionFromFEN(const FString& FEN);

	// Human makes a move (e.g. "e2e4", "e7e8q" for promotion)
	// Returns false if the move is illegal
	UFUNCTION(BlueprintCallable, Category = "Chess")
	bool MakeMove(const FString& MoveStr);

	// Ask the AI to find and play the best move at AISearchDepth
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void RequestAIMove();

	// Stop any ongoing search
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void StopSearch();

	// ---------------------------------------------------------------
	// Queries
	// ---------------------------------------------------------------

	// Returns the current position as a FEN string
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	FString GetFEN() const;

	// Returns whose turn it is: "white" or "black"
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	FString GetActiveColor() const;

	// Returns all legal moves from a square (e.g. "e2") as an array of target squares
	UFUNCTION(BlueprintCallable, Category = "Chess")
	TArray<FString> GetLegalMovesFromSquare(const FString& SquareStr) const;

	// Returns true if the active player is in check
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	bool IsInCheck() const;

	// Returns the piece on a square, e.g. "P" (white pawn), "k" (black king), "" (empty)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	FString GetPieceOnSquare(const FString& SquareStr) const;

private:
	// PImpl — engine objects heap-allocated; full types only in ChessManager.cpp
	struct FEngineImpl;
	FEngineImpl* Engine = nullptr;

	// Called back from Search on a background thread — marshalled to game thread
	void OnBestMoveFound(int BestMove);

	// Parse "e2" -> engine square index; returns -1 on failure
	static int SquareFromString(const FString& Str);

	// Engine square index -> "e2"
	static FString SquareToString(int Square);

	// Check for game-over conditions and broadcast OnGameOver if needed
	void CheckGameOver();
};
