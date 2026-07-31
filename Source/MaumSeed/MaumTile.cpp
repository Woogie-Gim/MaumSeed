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
	// 다 자란 작물이 있으면 모드와 무관하게 수확 우선
	if (PlantedCrop && PlantedCrop->IsHarvestable())
	{
		const int32 Score = PlantedCrop->HarvestCrop();
		OnCropHarvested.Broadcast(Score);
		PlantedCrop->Destroy();
		PlantedCrop = nullptr;
		return;
	}

	// 그 외엔 기존 모드별 동작
	switch (InteractMode)
	{
	case EMaumInteractMode::Plant:
		if (IsEmpty())
			PlantCrop(SelectedCropID, CropDataTable);
		break;

	case EMaumInteractMode::Water:
		if (PlantedCrop)
			PlantedCrop->WaterCrop();
		break;

	case EMaumInteractMode::Fertilize:
		if (PlantedCrop)
			PlantedCrop->ApplyFertilizer();
		break;

	default:
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

	// 작물이 스스로 수확되면 타일이 정리하도록 구독
	NewCrop->OnHarvestedSelf.AddDynamic(this, &AMaumTile::HandleCropSelfHarvested);

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

void AMaumTile::RestoreCrop(FName CropID, int32 Growth, int32 Stage)
{
	// 기존 작물이 있으면 제거
	if (PlantedCrop)
	{
		PlantedCrop->Destroy();
		PlantedCrop = nullptr;
	}

	if (CropID.IsNone() || !CropDataTable || !CropActorClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 10.f);
	FActorSpawnParameters Params;
	Params.Owner = this;

	AMaumCrop* NewCrop = World->SpawnActor<AMaumCrop>(
		CropActorClass, SpawnLocation, FRotator::ZeroRotator, Params);
	if (!NewCrop) return;

	// 저장된 상태 적용 (InitCrop + 성장치 복원을 한 번에)
	NewCrop->ApplySaveData(CropID, Growth, Stage, CropDataTable);

	NewCrop->OnHarvestedSelf.AddDynamic(this, &AMaumTile::HandleCropSelfHarvested);

	PlantedCrop = NewCrop;
}

void AMaumTile::HandleCropSelfHarvested(int32 Score)
{
	// DayManager가 점수를 집계하도록 기존 수확 이벤트 브로드캐스트
	OnCropHarvested.Broadcast(Score);

	// 작물 정리
	if (PlantedCrop)
	{
		PlantedCrop->Destroy();
		PlantedCrop = nullptr;
	}
}