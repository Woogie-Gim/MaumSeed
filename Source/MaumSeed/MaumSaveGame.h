#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MaumCropData.h"   // EMaumWeather 사용
#include "MaumSaveGame.generated.h"

// 타일 1칸의 작물 상태
USTRUCT()
struct FMaumCropSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 TileIndex = 0;

	UPROPERTY()
	FName CropID;

	UPROPERTY()
	int32 Growth = 0;

	UPROPERTY()
	int32 Stage = 0;
};

UCLASS()
class MAUMSEED_API UMaumSaveGame : public USaveGame
{
	GENERATED_BODY()

	public:
	// 심어진 작물들 (빈 타일은 저장 안 함)
	UPROPERTY()
	TArray<FMaumCropSaveData> Crops;

	// 게임 진행 상태
	UPROPERTY()
	int32 CurrentDay = 1;

	UPROPERTY()
	int32 TotalScore = 0;

	UPROPERTY()
	EMaumWeather TodayWeather = EMaumWeather::Sunny;

	UPROPERTY()
	EMaumWeather TomorrowWeather = EMaumWeather::Sunny;
};