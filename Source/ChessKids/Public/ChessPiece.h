#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiece.generated.h"

UENUM(BlueprintType)
enum class EChessPieceType : uint8
{
	Pawn,
	Knight,
	Bishop,
	Rook,
	Queen,
	King
};

UENUM(BlueprintType)
enum class EChessColor : uint8
{
	White,
	Black
};

UCLASS()
class CHESSKIDS_API AChessPiece : public AActor
{
	GENERATED_BODY()

public:
	AChessPiece();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess")
	EChessPieceType PieceType = EChessPieceType::Pawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess")
	EChessColor PieceColor = EChessColor::White;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess")
	int32 BoardFile = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chess")
	int32 BoardRank = 0;

	void Init(EChessPieceType Type, EChessColor Color, int32 File, int32 Rank);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp = nullptr;

private:
	void SetupMeshAndMaterial();
};
