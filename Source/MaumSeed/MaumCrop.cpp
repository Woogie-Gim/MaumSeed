#include "MaumCrop.h"

AMaumCrop::AMaumCrop()
{
	PrimaryActorTick.bCanEverTick = false;

	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CropMesh"));
	RootComponent = CropMesh;
}

void AMaumCrop::BeginPlay()
{
	Super::BeginPlay();

	// 에디터에서 미리 배치한 경우 대비
	if (!bCropDataValid && CropDataTable && !CurrentCropID.IsNone())
	{
		InitCrop(CurrentCropID, CropDataTable);
	}
}

void AMaumCrop::InitCrop(FName NewCropID, UDataTable* InDataTable)
{
	if (!InDataTable || NewCropID.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("InitCrop: 데이터테이블 또는 작물 ID가 유효하지 않습니다."));
		return;
	}

	CropDataTable = InDataTable;
	CurrentCropID = NewCropID;

	// 값으로 복사 (댕글링 포인터 방지)
	if (const FMaumCropData* Row = InDataTable->FindRow<FMaumCropData>(NewCropID, TEXT("InitCrop")))
	{
		CachedCropData = *Row;
		bCropDataValid = true;

		CurrentGrowth = 0;
		CurrentStage = 0;
		WateredToday = 0;
		bFertilizedToday = false;

		UpdateStageMesh();

		UE_LOG(LogTemp, Log, TEXT("[%s] 작물을 심었습니다."), *CachedCropData.Name);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("InitCrop: '%s' 행을 찾을 수 없습니다."), *NewCropID.ToString());
		bCropDataValid = false;
	}
}

void AMaumCrop::WaterCrop()
{
	if (!bCropDataValid || IsHarvestable()) return;

	WateredToday++;
	UE_LOG(LogTemp, Log, TEXT("[%s] 물주기 (%d/%d회)"),
		*CachedCropData.Name, WateredToday, CachedCropData.WaterPerDay);
}

void AMaumCrop::ApplyFertilizer()
{
	if (!bCropDataValid || IsHarvestable() || bFertilizedToday) return;

	bFertilizedToday = true;
	UE_LOG(LogTemp, Log, TEXT("[%s] 비료를 주었습니다."), *CachedCropData.Name);
}

void AMaumCrop::ProcessDailyGrowth(int32 BlessingValue, EMaumWeather TodayWeather)
{
	if (!bCropDataValid)
	{
		UE_LOG(LogTemp, Error, TEXT("ProcessDailyGrowth: 작물 데이터가 유효하지 않습니다."));
		return;
	}

	if (IsHarvestable())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[%s] 이미 수확 가능 상태입니다."), *CachedCropData.Name);
		return;
	}

	// 성장 판정식
	// 기준치 100에서 시작
	int32 Growth = 100;

	// 물주기 일치도: 부족/과다 모두 감점 (회당 25점)
	const int32 WaterDiff = FMath::Abs(WateredToday - CachedCropData.WaterPerDay);
	const int32 WaterPenalty = WaterDiff * 25;
	Growth -= WaterPenalty;

	// 날씨 일치 보너스
	int32 WeatherBonus = 0;
	if (TodayWeather == CachedCropData.PreferredWeather)
	{
		WeatherBonus = 20;
		Growth += WeatherBonus;
	}

	// 비료 보너스
	int32 FertBonus = 0;
	if (bFertilizedToday)
	{
		FertBonus = CachedCropData.FertilizerBonus;
		Growth += FertBonus;
	}

	// AI 축복치: 0~100 → -10~+10 (전체 영향력 약 10% 유지)
	const int32 BlessingBonus = FMath::Clamp((BlessingValue - 50) / 5, -10, 10);
	Growth += BlessingBonus;

	// 음수 방지
	Growth = FMath::Max(Growth, 0);
	CurrentGrowth += Growth;

	UE_LOG(LogTemp, Warning,
		TEXT("[%s] 일일 성장 +%d (물 -%d / 날씨 +%d / 비료 +%d / 축복 %+d) | 누적: %d"),
		*CachedCropData.Name, Growth, WaterPenalty, WeatherBonus, FertBonus, BlessingBonus, CurrentGrowth);

	// 단계 상승 판정 (누적치가 충분하면 여러 단계 동시 상승)
	while (!IsHarvestable() && CurrentGrowth >= (CurrentStage + 1) * 100)
	{
		AdvanceToNextStage();
	}
}

void AMaumCrop::AdvanceToNextStage()
{
	CurrentStage++;
	UpdateStageMesh();

	if (IsHarvestable())
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] 다 자랐습니다! 수확할 수 있어요."), *CachedCropData.Name);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] %d단계로 성장했습니다."), *CachedCropData.Name, CurrentStage);
	}
}

void AMaumCrop::UpdateStageMesh()
{
	if (StageMeshes.Num() == 0 || !CropMesh) return;

	const int32 MeshIndex = FMath::Clamp(CurrentStage, 0, StageMeshes.Num() - 1);

	if (StageMeshes[MeshIndex])
	{
		CropMesh->SetStaticMesh(StageMeshes[MeshIndex]);
		CropMesh->MarkRenderStateDirty();   // 렌더 상태 강제 갱신

		UE_LOG(LogTemp, Log, TEXT("[%s] 메시 교체: 인덱스 %d"), *CachedCropData.Name, MeshIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("StageMeshes[%d]가 비어 있습니다."), MeshIndex);
	}
}

bool AMaumCrop::IsHarvestable() const
{
	return bCropDataValid && CurrentStage >= CachedCropData.GrowthDays;
}

int32 AMaumCrop::HarvestCrop()
{
	if (!IsHarvestable()) return 0;

	// 품질 보정: 누적 성장치가 기준을 넘을수록 가산점
	const int32 IdealGrowth = CachedCropData.GrowthDays * 100;
	const float QualityRatio = (IdealGrowth > 0)
		? static_cast<float>(CurrentGrowth) / static_cast<float>(IdealGrowth)
		: 1.0f;

	const int32 FinalScore = FMath::RoundToInt(CachedCropData.BaseScore * FMath::Clamp(QualityRatio, 0.5f, 1.5f));

	UE_LOG(LogTemp, Warning, TEXT("[%s] 수확 완료! 획득 점수: %d (품질 배율 %.2f)"),
		*CachedCropData.Name, FinalScore, QualityRatio);

	// 상태 초기화
	bCropDataValid = false;
	CurrentCropID = NAME_None;
	CurrentGrowth = 0;
	CurrentStage = 0;
	WateredToday = 0;
	bFertilizedToday = false;

	if (CropMesh)
	{
		CropMesh->SetStaticMesh(nullptr);
	}

	return FinalScore;
}

void AMaumCrop::ResetDailyState()
{
	WateredToday = 0;
	bFertilizedToday = false;
}

void AMaumCrop::ApplySaveData(FName InCropID, int32 InGrowth, int32 InStage, UDataTable* InDataTable)
{
	if (InCropID.IsNone() || !InDataTable) return;

	InitCrop(InCropID, InDataTable);

	CurrentGrowth = InGrowth;
	CurrentStage = InStage;
	UpdateStageMesh();
}