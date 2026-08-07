#include "MaumTile.h"
#include "MaumCrop.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "MaumPlayerController.h"

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

	// 액터 네이티브 클릭 이벤트 구독
	OnClicked.AddDynamic(this, &AMaumTile::OnTileClicked);
	OnInputTouchBegin.AddDynamic(this, &AMaumTile::OnTileTouched);
}

void AMaumTile::ApplyTool(EMaumInteractMode Tool, FName CropID)
{
	PlayTouchFeedback();

	switch (Tool)
	{
	case EMaumInteractMode::Plant:
		if (IsEmpty())
			PlantCrop(CropID, CropDataTable);
		break;
	case EMaumInteractMode::Water:
		WaterPlantedCrop();
		break;
	case EMaumInteractMode::Fertilize:
		FertilizePlantedCrop();
		break;
	default:
		break;
	}
}

void AMaumTile::WaterPlantedCrop()
{
	if (PlantedCrop)
		PlantedCrop->WaterCrop();
}

void AMaumTile::FertilizePlantedCrop()
{
	if (PlantedCrop)
		PlantedCrop->ApplyFertilizer();
}

void AMaumTile::OnTileClicked(AActor* TouchedActor, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Warning, TEXT("타일 클릭됨!"));

	// PlayerController의 현재 도구를 조회해서 적용
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AMaumPlayerController* MaumPC = Cast<AMaumPlayerController>(PC))
		{
			ApplyTool(MaumPC->CurrentTool, MaumPC->CurrentCropID);
		}
	}
}

// 터치 핸들러 추가 (클릭 핸들러와 같은 동작)
void AMaumTile::OnTileTouched(ETouchIndex::Type FingerIndex, AActor* TouchedActor)
{
	UE_LOG(LogTemp, Warning, TEXT("타일 터치됨!"));

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AMaumPlayerController* MaumPC = Cast<AMaumPlayerController>(PC))
		{
			ApplyTool(MaumPC->CurrentTool, MaumPC->CurrentCropID);
		}
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