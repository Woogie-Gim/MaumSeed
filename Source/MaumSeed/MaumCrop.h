

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "MaumCropData.h" // 데이터 구조체 헤더 포함
#include "MaumCrop.generated.h"

UCLASS()
class MAUMSEED_API AMaumCrop : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMaumCrop();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 데이터테이블에서 할당받은 작물 정보 포인터
	FMaumCropData* CropData;

	// 축복치 수신 델리게이트 바인딩 함수
	UFUNCTION()
	void OnBlessingReceivedHandler(int32 BlessingValue);

	// 작물 3D 모델 렌더링 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CropMesh;

	// 작물 데이터테이블 참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	UDataTable* CropDataTable;

	// 현재 심어진 작물의 고유 ID (데이터테이블의 Row Name과 매칭)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CropData")
	FName CurrentCropID;

	// 현재 누적된 작물 성장치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	int32 CurrentGrowth = 0;

	// 현재 작물의 성장 단계 (일차)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CropData")
	int32 CurrentStage = 0;

	// 축복치 수신 및 누적 연산
	UFUNCTION()
	void UpdateCropGrowth(int32 BlessingValue);

	// 성장 단계 상승 및 상태 업데이트
	void AdvanceToNextStage();

	// 현재 작물 상태 저장
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveCropState();

	// 저장된 작물 상태 로드
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadCropState();

	// 디버깅용 하루 치 성장 강제 적용
	UFUNCTION(BlueprintCallable, Category = "Cheat")
	void CheatTimeSkip();
};
