#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MaumCropData.generated.h"

// 날씨 상태 정의
UENUM(BlueprintType)
enum class EMaumWeather : uint8
{
	Sunny UMETA(DisplayName = "맑음"),
	Cloudy UMETA(DisplayName = "흐림"),
	Rainy UMETA(DisplayName = "비")
};

// 작물 속성 정의를 위한 기반 구조체
USTRUCT(BlueprintType)
struct FMaumCropData : public FTableRowBase
{
	GENERATED_BODY()

	public:
	// 데이터 고유 식별자
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	FName CropId;

	// UI 출력용 작물 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	FString Name;

	// 완숙까지 필요한 인게임 일수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	int32 GrowthDays = 3;

	// 하루 요구 물 주기 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	int32 WaterPerDay = 1;

	// 성장 보너스를 받는 날씨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	EMaumWeather PreferredWeather = EMaumWeather::Sunny;

	// 비료 사용 시 추가 성장치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	int32 FertilizerBonus = 15;

	// 수확 시 기본 획득 점수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	int32 BaseScore = 10;
};