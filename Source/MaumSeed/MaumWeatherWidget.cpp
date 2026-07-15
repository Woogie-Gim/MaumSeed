
#include "MaumWeatherWidget.h"
#include "Components/Image.h"

void UMaumWeatherWidget::UpdateWeatherUI(EMaumWeather TodayWeather, EMaumWeather TomorrowWeather)
{
	// 오늘 날씨 텍스처 갱신
	if (Img_TodayWeather)
	{
		switch (TodayWeather)
		{
		case EMaumWeather::Sunny: Img_TodayWeather->SetBrushFromTexture(Tex_Sunny); break;
		case EMaumWeather::Cloudy: Img_TodayWeather->SetBrushFromTexture(Tex_Cloudy); break;
		case EMaumWeather::Rainy: Img_TodayWeather->SetBrushFromTexture(Tex_Rain); break;
		}
	}

	// 내일 날씨 텍스처 갱신
	if (Img_TomorrowWeather)
	{
		switch (TomorrowWeather)
		{
		case EMaumWeather::Sunny: Img_TomorrowWeather->SetBrushFromTexture(Tex_Sunny); break;
		case EMaumWeather::Cloudy: Img_TomorrowWeather->SetBrushFromTexture(Tex_Cloudy); break;
		case EMaumWeather::Rainy: Img_TomorrowWeather->SetBrushFromTexture(Tex_Rain); break;
		}
	}
}