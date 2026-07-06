#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiece.h"
#include "ChessManager.generated.h"

class AChessBoard;
class USoundBase;
class UAudioComponent;

namespace pulse
{
	class Search;
	class Position;
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIMoveReady,   FString, FromSquare, FString, ToSquare);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnGameOver,      FString, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoveMade,      FString, FromSquare, FString, ToSquare);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHintReady,     FString, FromSquare, FString, ToSquare);

UCLASS()
class CHESSKIDS_API AChessManager : public AActor
{
	GENERATED_BODY()

public:
	AChessManager();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

	UPROPERTY(BlueprintAssignable, Category = "Chess")
	FOnAIMoveReady OnAIMoveReady;

	UPROPERTY(BlueprintAssignable, Category = "Chess")
	FOnGameOver OnGameOver;

	UPROPERTY(BlueprintAssignable, Category = "Chess")
	FOnMoveMade OnMoveMade;

	UPROPERTY(BlueprintAssignable, Category = "Chess")
	FOnHintReady OnHintReady;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess", meta = (ClampMin = "1", ClampMax = "20"))
	int32 AISearchDepth = 5;

	// Optional lesson position for this arena (FEN). Empty = standard chess.
	// Story maps use it to teach one piece at a time (pawn battles in L_Pawn, etc).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	FString StartingFEN;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess")
	AChessBoard* Board = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Pieces")
	TMap<EChessPieceType, FPieceMeshConfig> WhitePieceMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Pieces")
	TMap<EChessPieceType, FPieceMeshConfig> BlackPieceMeshes;

	// --- Audio (defaults resolved from the sound packs at BeginPlay when unset) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio") USoundBase* MoveSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio") USoundBase* CaptureSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio") USoundBase* CheckSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio") USoundBase* WinSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio") USoundBase* LoseSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio") USoundBase* DrawSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio") USoundBase* BackgroundMusic = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SfxVolume = 0.8f;

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void NewGame();

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void SetPositionFromFEN(const FString& FEN);

	UFUNCTION(BlueprintCallable, Category = "Chess")
	bool MakeMove(const FString& MoveStr);

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void RequestAIMove();

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void StopSearch();

	// Sets AI strength and persists the choice on the GameInstance so it
	// survives level travel (1=Easy, 2=Medium, 3=Hard).
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void SetDifficulty(int32 Level);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	int32 GetDifficulty() const { return CurrentDifficulty; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	bool IsGameOver() const { return bGameOver; }

	// Local two-player (hot-seat) game — both colors human, AI disabled.
	// Mirrors UChessKidsGameInstance::bTwoPlayerMode at arena load.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	bool IsTwoPlayerMode() const { return bTwoPlayerMode; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	FString GetFEN() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	FString GetActiveColor() const;

	UFUNCTION(BlueprintCallable, Category = "Chess")
	TArray<FString> GetLegalMovesFromSquare(const FString& SquareStr) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	bool IsInCheck() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	FString GetPieceOnSquare(const FString& SquareStr) const;

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void UndoLastMove();

	// Ask the engine for a good move for the side to move; the answer arrives via
	// OnHintReady and is highlighted on the board. Uses the same async search as
	// the AI (depth-limited so kids get an answer fast).
	UFUNCTION(BlueprintCallable, Category = "Chess")
	void RequestHint();

	// Letters of the pieces a side has LOST (e.g. "Q R P P" for white's losses),
	// derived from the live position so undo/promotion stay consistent.
	// Deliberately NOT BlueprintPure: it scans the board — cache, don't bind.
	UFUNCTION(BlueprintCallable, Category = "Chess")
	FString GetCapturedPieces(bool bWhitePieces) const;

	// Increments on every move and undo — cheap change-detection key for UI
	// that derives from the position (captured trays etc.).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	int32 GetPositionVersion() const { return PositionVersion; }

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void RefreshBoard() { SpawnAllPieces(); }

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void SwapPromotedPiece(const FString& Square, const FString& PieceLetter);

	// The 3D actor currently standing on a square (nullptr if empty).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chess")
	AChessPiece* GetPieceActor(const FString& Square) const { return FindPieceOnSquare(Square); }

private:
	struct FEngineImpl;
	FEngineImpl* Engine = nullptr;
	bool bGameOver = false;
	bool bExpectingAIMove = false;   // armed by RequestAIMove; cleared by Undo/NewGame to drop stale AI replies
	bool bHintRequest = false;       // the pending search is a hint — show it, don't play it
	bool bTwoPlayerMode = false;
	int32 CurrentDifficulty = 1;
	int32 PositionVersion = 0;
	TArray<FString>FENHistory;

	UPROPERTY() UAudioComponent* MusicComponent = nullptr;

	void ResolveDefaultSounds();
	void PlaySfx(USoundBase* Sound) const;
	UFUNCTION() void RestartMusic();

	void OnBestMoveFound(int BestMove);

	static int SquareFromString(const FString& Str);

	static FString SquareToString(int Square);

	void CheckGameOver();

	UPROPERTY() TMap<FString, AChessPiece*> PieceActors;

	// Captured pieces mid-fling: no longer in PieceActors but still alive for the
	// animation — swept by SpawnAllPieces so board rebuilds can't leave ghosts.
	UPROPERTY() TArray<AChessPiece*> FlingingPieces;

	void SpawnAllPieces();

	UFUNCTION()
	void HandleMoveMade(FString From, FString To);

	AChessPiece* FindPieceOnSquare(const FString& Square) const;

	static EChessPieceType CharToPieceType(TCHAR C);
};
