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
	// 수확은 자동으로 처리되므로, 타일 터치는 심기·물주기·비료만 담당
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
	// 작물의 수확 임박 자막을 위로 중계
	NewCrop->OnHarvestImminent.AddDynamic(this, &AMaumTile::RelayHarvestImminent);

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

	NewCrop->OnHarvestImminent.AddDynamic(this, &AMaumTile::RelayHarvestImminent);

	PlantedCrop = NewCrop;
}

void AMaumTile::HandleCropSelfHarvested(int32 Score)
{
	// DayManager가 점수를 집계하도록 기존 수확 이벤트 브로드캐스트
	OnCropHarvested.Broadcast(Score);

	// 작물이 페이드아웃하는 동안 기다렸다가 정리
	if (PlantedCrop)
	{
		// PlantedCrop 참조만 끊고, 작물은 스스로 페이드 후 남아있다 정리
		AMaumCrop* Crop = PlantedCrop;
		PlantedCrop = nullptr;

		// 페이드(0.4초)보다 살짝 뒤에 파괴
		FTimerHandle DestroyTimer;
		FTimerDelegate DestroyDelegate;
		DestroyDelegate.BindLambda([Crop]()
			{
				if (IsValid(Crop))
				{
					Crop->Destroy();
				}
			});
		GetWorldTimerManager().SetTimer(DestroyTimer, DestroyDelegate, 0.5f, false);
	}
}

void AMaumTile::RelayHarvestImminent(const FString& Message)
{
	OnTileHarvestImminent.Broadcast(Message);
}