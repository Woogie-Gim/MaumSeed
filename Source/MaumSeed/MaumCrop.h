

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
	int32 CurrentGrowth;
};
