
#include "MaumGridManager.h"
#include "MaumTile.h"

// Sets default values
AMaumGridManager::AMaumGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMaumGridManager::BeginPlay()
{
	Super::BeginPlay();

	// 설정된 타일 클래스가 없으면 스폰 중단
	if (!TileClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 2차원 배열 형태 타일 배치
	for (int32 X = 0; X < GridSizeX; ++X)
	{
		for (int32 Y = 0; Y < GridSizeY; ++Y)
		{
			FVector SpawnLocation = FVector(X * TileSpacing, Y * TileSpacing, 0.0f);
			World->SpawnActor<AMaumTile>(TileClass, SpawnLocation, FRotator::ZeroRotator);
		}
	}
	
}

// Called every frame
void AMaumGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

