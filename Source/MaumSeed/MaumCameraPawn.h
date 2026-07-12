

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MaumCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class MAUMSEED_API AMaumCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMaumCameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 카메라 거리 및 각도 조절 지지대
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	// 메인 렌더링 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
