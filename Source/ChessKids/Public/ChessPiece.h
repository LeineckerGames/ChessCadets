#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/SkeletalMesh.h"
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
	USkeletalMesh* Mesh = nullptr;

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
	virtual void Tick(float DeltaTime) override;

	// Smoothly moves the piece to a world location with a small hop (knights hop
	// higher — they jump!). Replaces the instant teleport for in-game moves.
	void StartGlide(const FVector& TargetLocation, float Duration = 0.3f);

	// Capture feedback: the piece pops upward and shrinks away, then destroys
	// itself. Replaces the instant Destroy() blink-out.
	void StartCaptureFling();

	// Selection feedback: lifts the piece and bobs it gently until deselected,
	// so which piece is "in hand" is unmistakable.
	void SetSelected(bool bSelected);

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
	USkeletalMeshComponent* MeshComp = nullptr;

private:
	void SetupMeshAndMaterial(const FPieceMeshConfig* MeshOverride);

	// Glide state
	FVector GlideStart = FVector::ZeroVector;
	FVector GlideTarget = FVector::ZeroVector;
	float GlideElapsed = 0.f;
	float GlideDuration = 0.f;
	bool bGliding = false;

	// Capture-fling state
	FVector FlingStartScale = FVector::OneVector;
	float FlingElapsed = 0.f;
	bool bFlinging = false;

	// Selection-hover state
	FVector HoverBase = FVector::ZeroVector;   // resting spot to lift from / return to
	float HoverAlpha = 0.f;                    // 0 = grounded, 1 = fully lifted
	float HoverTime = 0.f;
	bool bSelectedHover = false;
};
