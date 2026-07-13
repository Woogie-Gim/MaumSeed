
#include "MaumTile.h"

// Sets default values
AMaumTile::AMaumTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 메시 컴포넌트 생성 및 루트 설정
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	RootComponent = TileMesh;

	// 레이캐스트 충돌용 콜리전 설정
	TileMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

}

// Called when the game starts or when spawned
void AMaumTile::BeginPlay()
{
	Super::BeginPlay();
	
	// 터치 이벤트 델리게이트 바인딩
	OnInputTouchBegin.AddDynamic(this, &AMaumTile::OnTileTouched);
}

// Called every frame
void AMaumTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMaumTile::OnTileTouched(ETouchIndex::Type FingerIndex, AActor* TouchedActor)
{
	// 터치 확인용 에디터 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("타일 터치 인식 완료: %s"), *GetName());

	// 상호작용 피드백용 타일 스케일 축소
	SetActorScale3D(FVector(0.9f, 0.9f, 0.9f));
}