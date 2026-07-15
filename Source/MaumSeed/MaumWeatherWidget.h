

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MaumCropData.h"
#include "MaumWeatherWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class MAUMSEED_API UMaumWeatherWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 날씨 UI 업데이트
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void UpdateWeatherUI(EMaumWeather TodayWeather, EMaumWeather TomorrowWeather);

protected:
	// 오늘 날씨 이미지 위젯 연동
	UPROPERTY(meta = (BindWidget))
	UImage* Img_TodayWeather;

	// 내일 날씨 이미지 위젯 연동
	UPROPERTY(meta = (BindWidget))
	UImage* Img_TomorrowWeather;

	// 맑음 아이콘 텍스처
	UPROPERTY(EditDefaultsOnly, Category = "Weather Icons")
	UTexture2D* Tex_Sunny;

	// 흐림 아이콘 텍스처
	UPROPERTY(EditDefaultsOnly, Category = "Weather Icons")
	UTexture2D* Tex_Cloudy;

	// 비 아이콘 텍스처
	UPROPERTY(EditDefaultsOnly, Category = "Weather Icons")
	UTexture2D* Tex_Rain;
};
