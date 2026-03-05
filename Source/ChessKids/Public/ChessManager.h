#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiece.h"
#include "ChessManager.generated.h"

class AChessBoard;

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

private:
	struct FEngineImpl;
	FEngineImpl* Engine = nullptr;

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
