#include "MaumDayManager.h"
#include "MaumTile.h"
#include "MaumCrop.h"
#include "MaumWeatherManager.h"
#include "MaumAIManager.h"
#include "MaumWeatherWidget.h"
#include "MaumGridManager.h"
#include "Kismet/GameplayStatics.h"

AMaumDayManager::AMaumDayManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMaumDayManager::BeginPlay()
{
	Super::BeginPlay();

	CollectSceneActors();
	RefreshWeatherUI();

	UE_LOG(LogTemp, Log, TEXT("=== %d일차 시작 ==="), CurrentDay);
}

void AMaumDayManager::CollectSceneActors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 날씨 매니저
	WeatherManager = Cast<AMaumWeatherManager>(
		UGameplayStatics::GetActorOfClass(World, AMaumWeatherManager::StaticClass()));

	if (!WeatherManager)
	{
		UE_LOG(LogTemp, Error, TEXT("DayManager: WeatherManager를 찾지 못했습니다. 레벨에 배치했는지 확인하세요."));
	}

	// AI 매니저 + 델리게이트 바인딩
	AIManager = Cast<AMaumAIManager>(
		UGameplayStatics::GetActorOfClass(World, AMaumAIManager::StaticClass()));

	if (AIManager)
	{
		AIManager->OnBlessingReceived.AddDynamic(this, &AMaumDayManager::HandleBlessingReceived);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DayManager: AIManager를 찾지 못했습니다."));
	}

	// 모든 타일 수집 + 수확 이벤트 바인딩
	GridManager = Cast<AMaumGridManager>(
		UGameplayStatics::GetActorOfClass(World, AMaumGridManager::StaticClass()));

	if (GridManager)
	{
		// GridManager가 아직 타일을 안 만들었을 수 있으므로 다음 틱에 수집
		World->GetTimerManager().SetTimerForNextTick(this, &AMaumDayManager::BindTiles);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DayManager: GridManager를 찾지 못했습니다."));
	}
}

void AMaumDayManager::SubmitDiaryAndEndDay(const FString& DiaryText)
{
	if (bWaitingForBlessing)
	{
		UE_LOG(LogTemp, Warning, TEXT("DayManager: 이미 AI 응답을 기다리는 중입니다."));
		return;
	}

	if (!AIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("DayManager: AIManager가 없어 중립 축복치로 진행합니다."));
		EndDayImmediate(50);
		return;
	}

	bWaitingForBlessing = true;
	AIManager->SendDiaryToLLM(DiaryText);
}

void AMaumDayManager::HandleBlessingReceived(int32 BlessingValue)
{
	if (!bWaitingForBlessing) return;

	bWaitingForBlessing = false;
	ProcessEndOfDay(BlessingValue);
}

void AMaumDayManager::EndDayImmediate(int32 BlessingValue)
{
	if (bWaitingForBlessing)
	{
		UE_LOG(LogTemp, Warning, TEXT("DayManager: AI 응답 대기 중에는 타임스킵할 수 없습니다."));
		return;
	}

	ProcessEndOfDay(BlessingValue);
}

void AMaumDayManager::ProcessEndOfDay(int32 BlessingValue)
{
	const EMaumWeather TodayWeather = WeatherManager
		? WeatherManager->GetCurrentWeather()
		: EMaumWeather::Sunny;

	UE_LOG(LogTemp, Warning, TEXT("--- %d일차 정산 시작 (축복치: %d) ---"), CurrentDay, BlessingValue);

	// 1) 모든 작물 성장 판정
	for (AMaumTile* Tile : AllTiles)
	{
		if (!Tile) continue;

		if (AMaumCrop* Crop = Tile->GetPlantedCrop())
		{
			Crop->ProcessDailyGrowth(BlessingValue, TodayWeather);
			Crop->ResetDailyState();
		}
	}

	// 2) 날씨 갱신
	if (WeatherManager)
	{
		WeatherManager->GenerateNextWeather();
	}

	// 3) 날짜 진행
	CurrentDay++;

	// 4) UI 갱신
	RefreshWeatherUI();

	UE_LOG(LogTemp, Warning, TEXT("--- 정산 완료. %d일차 시작 (누적 점수: %d) ---"), CurrentDay, TotalScore);

	OnDayEnded.Broadcast(CurrentDay, TotalScore);
}

void AMaumDayManager::HandleCropHarvested(int32 Score)
{
	TotalScore += Score;
	UE_LOG(LogTemp, Log, TEXT("수확 점수 +%d (누적: %d)"), Score, TotalScore);
}

void AMaumDayManager::RefreshWeatherUI()
{
	if (WeatherWidget && WeatherManager)
	{
		WeatherWidget->UpdateWeatherUI(
			WeatherManager->GetCurrentWeather(),
			WeatherManager->GetTomorrowWeather());
	}
}

void AMaumDayManager::BindTiles()
{
	if (!GridManager) return;

	AllTiles.Empty();

	for (AMaumTile* Tile : GridManager->GetTiles())
	{
		if (!Tile) continue;

		AllTiles.Add(Tile);
		Tile->OnCropHarvested.AddDynamic(this, &AMaumDayManager::HandleCropHarvested);
	}

	UE_LOG(LogTemp, Log, TEXT("DayManager: 타일 %d개를 인식했습니다."), AllTiles.Num());
}

void AMaumDayManager::SetWeatherWidget(UMaumWeatherWidget* InWidget)
{
	WeatherWidget = InWidget;
	RefreshWeatherUI();
}