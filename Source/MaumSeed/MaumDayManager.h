#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MaumCropData.h"
#include "MaumDayManager.generated.h"

class AMaumTile;
class AMaumWeatherManager;
class AMaumAIManager;
class UMaumWeatherWidget;

// 하루 종료 알림 (UI 정산 화면용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDayEnded, int32, NewDay, int32, TotalScore);

UCLASS()
class MAUMSEED_API AMaumDayManager : public AActor
{
	GENERATED_BODY()

public:
	AMaumDayManager();

	// 일기 제출 → AI 요청 → 응답 도착 시 하루 정산
	UFUNCTION(BlueprintCallable, Category = "Day")
	void SubmitDiaryAndEndDay(const FString& DiaryText);

	// AI 없이 즉시 하루 종료 (디버그/타임스킵)
	UFUNCTION(BlueprintCallable, Category = "Day")
	void EndDayImmediate(int32 BlessingValue = 50);

	UFUNCTION(BlueprintPure, Category = "Day")
	int32 GetCurrentDay() const { return CurrentDay; }

	UFUNCTION(BlueprintPure, Category = "Day")
	int32 GetTotalScore() const { return TotalScore; }

	UFUNCTION(BlueprintCallable, Category = "Day|UI")
	void SetWeatherWidget(UMaumWeatherWidget* InWidget);

	// 하루 종료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Day|Event")
	FOnDayEnded OnDayEnded;

	// 날씨 UI 위젯 (블루프린트에서 할당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day|UI")
	TObjectPtr<UMaumWeatherWidget> WeatherWidget;

protected:
	virtual void BeginPlay() override;

	// AI 축복치 수신 콜백
	UFUNCTION()
	void HandleBlessingReceived(int32 BlessingValue);

	// 수확 점수 집계 콜백
	UFUNCTION()
	void HandleCropHarvested(int32 Score);

	// 실제 하루 정산 처리
	void ProcessEndOfDay(int32 BlessingValue);

	// 레벨 내 액터 수집 및 바인딩
	void CollectSceneActors();

	// 날씨 UI 갱신
	void RefreshWeatherUI();

	UFUNCTION()
	void BindTiles();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day")
	int32 CurrentDay = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day")
	int32 TotalScore = 0;

	// 중복 정산 방지
	UPROPERTY(VisibleAnywhere, Category = "Day")
	bool bWaitingForBlessing = false;

	UPROPERTY()
	TObjectPtr<AMaumWeatherManager> WeatherManager;

	UPROPERTY()
	TObjectPtr<AMaumAIManager> AIManager;

	UPROPERTY()
	TArray<TObjectPtr<AMaumTile>> AllTiles;

	UPROPERTY()
	TObjectPtr<class AMaumGridManager> GridManager;
};