#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "MaumCropData.h"
#include "MaumCrop.generated.h"

UCLASS()
class MAUMSEED_API AMaumCrop : public AActor
{
	GENERATED_BODY()

	public:
	AMaumCrop();

	// 작물 데이터 초기화 (심을 때 호출)
	UFUNCTION(BlueprintCallable, Category = "Crop")
	void InitCrop(FName NewCropID, UDataTable* InDataTable);

	// 하루 치 성장 판정 (DayManager가 호출)
	UFUNCTION(BlueprintCallable, Category = "Crop")
	void ProcessDailyGrowth(int32 BlessingValue, EMaumWeather TodayWeather);

	// 물주기
	UFUNCTION(BlueprintCallable, Category = "Crop")
	void WaterCrop();

	// 비료 주기
	UFUNCTION(BlueprintCallable, Category = "Crop")
	void ApplyFertilizer();

	// 수확 (획득 점수 반환, 실패 시 0)
	UFUNCTION(BlueprintCallable, Category = "Crop")
	int32 HarvestCrop();

	// 하루 종료 시 일일 상태 초기화
	void ResetDailyState();

	// 상태 조회
	UFUNCTION(BlueprintPure, Category = "Crop")
	bool IsHarvestable() const;

	UFUNCTION(BlueprintPure, Category = "Crop")
	int32 GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintPure, Category = "Crop")
	int32 GetCurrentGrowth() const { return CurrentGrowth; }

	UFUNCTION(BlueprintPure, Category = "Crop")
	FName GetCropID() const { return CurrentCropID; }

	UFUNCTION(BlueprintPure, Category = "Crop")
	bool IsDataValid() const { return bCropDataValid; }

	// 세이브/로드용
	void ApplySaveData(FName InCropID, int32 InGrowth, int32 InStage, UDataTable* InDataTable);

	// 작물 3D 모델
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CropMesh;

	// 성장 단계별 메시 (씨앗/새싹/성장/개화)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TArray<TObjectPtr<UStaticMesh>> StageMeshes;

protected:
	virtual void BeginPlay() override;

	// 성장 단계 상승 처리
	void AdvanceToNextStage();

	// 현재 단계에 맞는 메시로 교체
	void UpdateStageMesh();

	// 데이터테이블 참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	TObjectPtr<UDataTable> CropDataTable;

	// 현재 작물 고유 ID
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	FName CurrentCropID;

	// 데이터테이블에서 복사한 작물 정보 (포인터 아닌 값)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	FMaumCropData CachedCropData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	bool bCropDataValid = false;

	// 누적 성장치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	int32 CurrentGrowth = 0;

	// 현재 성장 단계
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	int32 CurrentStage = 0;

	// 오늘 물 준 횟수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	int32 WateredToday = 0;

	// 오늘 비료 사용 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	bool bFertilizedToday = false;
};