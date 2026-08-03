#include "MaumCrop.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/WidgetComponent.h"

AMaumCrop::AMaumCrop()
{
	PrimaryActorTick.bCanEverTick = true;

	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CropMesh"));
	RootComponent = CropMesh;

	// 자동 수확 방식이라 클릭 콜리전은 불필요, 물리만 확실히 끔
	CropMesh->SetSimulatePhysics(false);
	CropMesh->SetEnableGravity(false);
	CropMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StageWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StageWidget"));
	StageWidget->SetupAttachment(RootComponent);
	StageWidget->SetWidgetSpace(EWidgetSpace::World);
	StageWidget->SetRelativeLocation(FVector(0.f, 0.f, 90.f));   // 작물 위 90 유닛
	StageWidget->SetDrawSize(FVector2D(150.f, 60.f));
	StageWidget->SetTickMode(ETickMode::Automatic);
}

void AMaumCrop::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);   // 평소엔 Tick 꺼둠 (페이드 때만 켬)

	// 에디터에서 미리 배치한 경우 대비
	if (!bCropDataValid && CropDataTable && !CurrentCropID.IsNone())
	{
		InitCrop(CurrentCropID, CropDataTable);
	}
}

void AMaumCrop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsFadingOut) return;

	FadeElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(FadeElapsed / FadeOutDuration, 0.0f, 1.0f);

	// 1.0 → 0.0으로 스케일 축소
	const float NewScale = FMath::Lerp(1.0f, 0.0f, Alpha);
	SetActorScale3D(FVector(NewScale));

	// 다 줄어들면 메시 정리 + Tick 끄기
	if (Alpha >= 1.0f)
	{
		bIsFadingOut = false;
		SetActorTickEnabled(false);

		if (CropMesh)
		{
			CropMesh->SetStaticMesh(nullptr);
		}
		SetActorScale3D(FVector(1.0f));   // 스케일 원복 (다음 작물 위해)
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

	// 물주기 효과음
	if (WaterSound)
	{
		UGameplayStatics::PlaySound2D(this, WaterSound);
	}
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

	// --- 성장 판정식 ---
	int32 Growth = 100;

	// 1) 물주기 일치도: 부족은 크게, 과다는 작게 감점
	const int32 WaterDiff = WateredToday - CachedCropData.WaterPerDay;
	const int32 WaterPenalty = (WaterDiff < 0)
		? FMath::Abs(WaterDiff) * 30
		: WaterDiff * 10;
	Growth -= WaterPenalty;

	// 2) 날씨 일치 보너스
	int32 WeatherBonus = 0;
	if (TodayWeather == CachedCropData.PreferredWeather)
	{
		WeatherBonus = 20;
		Growth += WeatherBonus;
	}

	// 3) 비료 보너스
	int32 FertBonus = 0;
	if (bFertilizedToday)
	{
		FertBonus = CachedCropData.FertilizerBonus;
		Growth += FertBonus;
	}

	// 4) AI 축복치: 0~100 → -10~+10 (보조 보정)
	const int32 BlessingBonus = FMath::Clamp((BlessingValue - 50) / 5, -10, 10);
	Growth += BlessingBonus;

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
		UE_LOG(LogTemp, Log, TEXT("[%s] 다 자랐습니다! 곧 수확됩니다."), *CachedCropData.Name);

		// 수확 임박 자막 표시 요청
		const FString Msg = FString::Printf(TEXT("%s이(가) 다 자랐어요! 곧 수확됩니다..."), *CachedCropData.Name);
		OnHarvestImminent.Broadcast(Msg);

		// 3초 후 자동 수확
		GetWorldTimerManager().SetTimer(
			AutoHarvestTimer, this, &AMaumCrop::AutoHarvest, 3.0f, false);
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
		CropMesh->MarkRenderStateDirty();
	}
}

bool AMaumCrop::IsHarvestable() const
{
	return bCropDataValid && CurrentStage >= CachedCropData.GrowthDays;
}

void AMaumCrop::AutoHarvest()
{
	if (!IsHarvestable()) return;

	const int32 Score = HarvestCrop();   // 효과음·점수 계산 포함
	OnHarvestedSelf.Broadcast(Score);    // 타일이 받아서 정리
}

int32 AMaumCrop::HarvestCrop()
{
	if (!IsHarvestable()) return 0;

	// 수확 효과음
	if (HarvestSound)
	{
		UGameplayStatics::PlaySound2D(this, HarvestSound);
	}

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

	// 메시를 바로 없애지 않고 페이드아웃 시작
	bIsFadingOut = true;
	FadeElapsed = 0.0f;
	SetActorTickEnabled(true);   // 페이드 동안만 Tick 켜기

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

	// 이미 다 자란 상태로 로드되면 자동 수확 예약
	if (IsHarvestable())
	{
		const FString Msg = FString::Printf(TEXT("%s이(가) 다 자랐어요! 곧 수확됩니다..."), *CachedCropData.Name);
		OnHarvestImminent.Broadcast(Msg);

		GetWorldTimerManager().SetTimer(
			AutoHarvestTimer, this, &AMaumCrop::AutoHarvest, 3.0f, false);
	}
}

void AMaumCrop::GetSaveData(FName& OutCropID, int32& OutGrowth, int32& OutStage) const
{
	OutCropID = CurrentCropID;
	OutGrowth = CurrentGrowth;
	OutStage = CurrentStage;
}