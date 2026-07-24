#include "MaumTile.h"
#include "MaumCrop.h"
#include "Engine/World.h"
#include "TimerManager.h"

AMaumTile::AMaumTile()
{
	PrimaryActorTick.bCanEverTick = false;

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	RootComponent = TileMesh;

	TileMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

void AMaumTile::BeginPlay()
{
	Super::BeginPlay();

	OnInputTouchBegin.AddDynamic(this, &AMaumTile::OnTileTouched);
}

void AMaumTile::OnTileTouched(ETouchIndex::Type FingerIndex, AActor* TouchedActor)
{
	PlayTouchFeedback();
	InteractWithTile();
}

void AMaumTile::InteractWithTile()
{
	switch (InteractMode)
	{
	case EMaumInteractMode::Plant:
		if (IsEmpty())
		{
			PlantCrop(SelectedCropID, CropDataTable);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("타일 %d: 이미 작물이 심어져 있습니다."), TileIndex);
		}
		break;

	case EMaumInteractMode::Water:
		if (PlantedCrop)
		{
			PlantedCrop->WaterCrop();
		}
		break;

	case EMaumInteractMode::Fertilize:
		if (PlantedCrop)
		{
			PlantedCrop->ApplyFertilizer();
		}
		break;

	case EMaumInteractMode::Harvest:
		if (PlantedCrop && PlantedCrop->IsHarvestable())
		{
			const int32 Score = PlantedCrop->HarvestCrop();
			OnCropHarvested.Broadcast(Score);

			PlantedCrop->Destroy();
			PlantedCrop = nullptr;
		}
		break;
	}
}

bool AMaumTile::PlantCrop(FName CropID, UDataTable* InDataTable)
{
	if (!IsEmpty() || CropID.IsNone() || !InDataTable || !CropActorClass)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World) return false;

	// 타일 위에 작물 스폰
	const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 10.f);
	FActorSpawnParameters Params;
	Params.Owner = this;

	AMaumCrop* NewCrop = World->SpawnActor<AMaumCrop>(CropActorClass, SpawnLocation, FRotator::ZeroRotator, Params);
	if (!NewCrop) return false;

	NewCrop->InitCrop(CropID, InDataTable);
	PlantedCrop = NewCrop;

	return true;
}

void AMaumTile::PlayTouchFeedback()
{
	SetActorScale3D(FVector(0.9f));

	// 0.15초 후 원래 크기로 복구
	GetWorldTimerManager().SetTimer(
		ScaleRestoreTimer, this, &AMaumTile::RestoreTileScale, 0.15f, false);
}

void AMaumTile::RestoreTileScale()
{
	SetActorScale3D(FVector(1.0f));
}