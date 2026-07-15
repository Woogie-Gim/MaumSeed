
#include "MaumWeatherManager.h"
#include "Math/UnrealMathUtility.h"

// Sets default values
AMaumWeatherManager::AMaumWeatherManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 초기 상태 설정
	CurrentWeather = EMaumWeather::Sunny;
	TomorrowWeather = EMaumWeather::Sunny;
}

// Called when the game starts or when spawned
void AMaumWeatherManager::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 날씨 2회 갱신하여 오늘과 내일 데이터 초기화
	GenerateNextWeather();
	GenerateNextWeather();
	
}

// Called every frame
void AMaumWeatherManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMaumWeatherManager::GenerateNextWeather()
{
	// 내일 날씨를 오늘 날씨로 이관
	CurrentWeather = TomorrowWeather;

	float RandomValue = FMath::FRand();

	// 비 온 다음 날 맑을 확률 가중치 적용 (70%)
	if (CurrentWeather == EMaumWeather::Rainy)
	{
		if (RandomValue < 0.7f) TomorrowWeather = EMaumWeather::Sunny;
		else if (RandomValue < 0.9f) TomorrowWeather = EMaumWeather::Cloudy;
		else TomorrowWeather = EMaumWeather::Rainy;
	}
	// 일반적인 날씨 확률 (맑음 50%, 흐림 30%, 비 20%)
	else
	{
		if (RandomValue < 0.5f) TomorrowWeather = EMaumWeather::Sunny;
		else if (RandomValue < 0.8f) TomorrowWeather = EMaumWeather::Cloudy;
		else TomorrowWeather = EMaumWeather::Rainy;
	}
}

EMaumWeather AMaumWeatherManager::GetCurrentWeather() const
{
	return CurrentWeather;
}

EMaumWeather AMaumWeatherManager::GetTomorrowWeather() const
{
	return TomorrowWeather;
}