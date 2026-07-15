

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MaumCropData.h"
#include "MaumWeatherManager.generated.h"

UCLASS()
class MAUMSEED_API AMaumWeatherManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMaumWeatherManager();

	// 다음 날씨 갱신 및 내일 예보 생성
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void GenerateNextWeather();

	// 현재 날씨 반환
	UFUNCTION(BlueprintPure, Category = "Weather")
	EMaumWeather GetCurrentWeather() const;

	// 내일 날씨 반환
	UFUNCTION(BlueprintPure, Category = "Weather")
	EMaumWeather GetTomorrowWeather() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// 상태 저장 변수
	EMaumWeather CurrentWeather;
	EMaumWeather TomorrowWeather;

};
