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

USTRUCT(BlueprintType)
struct FPieceMeshConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowAbstract = "false"))
	TSubclassOf<AActor> PieceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* MaterialOverride = nullptr;
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

	void Init(EChessPieceType Type, EChessColor Color, int32 File, int32 Rank, const FPieceMeshConfig* MeshOverride = nullptr);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp = nullptr;

private:
	void SetupMeshAndMaterial(const FPieceMeshConfig* MeshOverride);
};
