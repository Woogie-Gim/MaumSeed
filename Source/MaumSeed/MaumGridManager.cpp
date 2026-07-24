#include "MaumGridManager.h"
#include "MaumTile.h"
#include "Engine/World.h"

AMaumGridManager::AMaumGridManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMaumGridManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateGrid();
}

void AMaumGridManager::GenerateGrid()
{
	if (!TileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("GridManager: TileClass가 지정되지 않았습니다."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	SpawnedTiles.Empty();

	const FVector Origin = GetActorLocation();
	int32 Index = 0;

	for (int32 X = 0; X < GridSizeX; ++X)
	{
		for (int32 Y = 0; Y < GridSizeY; ++Y)
		{
			// 그리드 매니저 위치 기준 상대 배치
			const FVector SpawnLocation = Origin + FVector(X * TileSpacing, Y * TileSpacing, 0.0f);

			FActorSpawnParameters Params;
			Params.Owner = this;

			AMaumTile* NewTile = World->SpawnActor<AMaumTile>(
				TileClass, SpawnLocation, FRotator::ZeroRotator, Params);

			if (!NewTile) continue;

			// 타일 식별자 부여 (세이브/로드 매칭용)
			NewTile->TileIndex = Index++;

			// 데이터테이블 일괄 주입
			if (CropDataTable)
			{
				NewTile->CropDataTable = CropDataTable;
			}

			SpawnedTiles.Add(NewTile);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GridManager: 타일 %d개 생성 완료 (%dx%d)"),
		SpawnedTiles.Num(), GridSizeX, GridSizeY);
}

TArray<AMaumTile*> AMaumGridManager::GetTiles() const
{
	TArray<AMaumTile*> Result;
	Result.Reserve(SpawnedTiles.Num());

	for (const TObjectPtr<AMaumTile>& Tile : SpawnedTiles)
	{
		Result.Add(Tile.Get());
	}
	return Result;
}

AMaumTile* AMaumGridManager::GetTileByIndex(int32 Index) const
{
	if (SpawnedTiles.IsValidIndex(Index))
	{
		return SpawnedTiles[Index];
	}
	return nullptr;
}