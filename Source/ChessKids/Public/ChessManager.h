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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess", meta = (ClampMin = "1", ClampMax = "20"))
	int32 AISearchDepth = 5;

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

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void RefreshBoard() { SpawnAllPieces(); }

	UFUNCTION(BlueprintCallable, Category = "Chess")
	void SwapPromotedPiece(const FString& Square, const FString& PieceLetter);  

private:
	struct FEngineImpl;
	FEngineImpl* Engine = nullptr;
	bool bGameOver = false;
	bool bExpectingAIMove = false;   // armed by RequestAIMove; cleared by Undo/NewGame to drop stale AI replies
	bool bTwoPlayerMode = false;
	int32 CurrentDifficulty = 1;
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
	void SpawnAllPieces();

	UFUNCTION()
	void HandleMoveMade(FString From, FString To);

	AChessPiece* FindPieceOnSquare(const FString& Square) const;

	static EChessPieceType CharToPieceType(TCHAR C);
};
