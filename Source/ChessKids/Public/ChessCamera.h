#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessCamera.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AChessBoard;

UCLASS()
class CHESSKIDS_API AChessCamera : public AActor
{
	GENERATED_BODY()

public:
	AChessCamera();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	AChessBoard* TargetBoard = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ArmLength = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float PitchAngle = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float Yaw = -90.f; // -90 = white side, 90 = black side

private:
	void ApplyView();
};
