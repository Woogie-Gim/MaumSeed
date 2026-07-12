
#include "MaumCameraPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AMaumCameraPawn::AMaumCameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 루트 컴포넌트 생성
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// 스프링암 부착 및 쿼터뷰 각도 설정
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1500.0f; // 카메라 거리
	SpringArm->SetRelativeRotation(FRotator(-45.0f, 45.0f, 0.0f)); // 2.5D 쿼터뷰 각도
	SpringArm->bDoCollisionTest = false; // 카메라 충돌 방지 (최적화)

	// 카메라 부착
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	// 가짜 직교 투영을 위해 시야각을 좁혀 평탄화
	Camera->FieldOfView = 30.0f;

}

// Called when the game starts or when spawned
void AMaumCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMaumCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMaumCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

