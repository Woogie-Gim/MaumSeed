#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "MaumTile.generated.h"   

class AMaumCrop;
class UDataTable;

// 타일 상호작용 모드
UENUM(BlueprintType)
enum class EMaumInteractMode : uint8
{
	Plant   UMETA(DisplayName = "심기"),
	Water   UMETA(DisplayName = "물주기"),
	Fertilize UMETA(DisplayName = "비료"),
	Harvest UMETA(DisplayName = "수확")
};

// 수확 알림 델리게이트 (DayManager가 점수 집계)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCropHarvested, int32, Score);

UCLASS()
class MAUMSEED_API AMaumTile : public AActor
{
	GENERATED_BODY()

	public:
	AMaumTile();

	// 타일 상호작용 진입점
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void InteractWithTile();

	// 작물 심기
	UFUNCTION(BlueprintCallable, Category = "Tile")
	bool PlantCrop(FName CropID, UDataTable* InDataTable);

	UFUNCTION(BlueprintPure, Category = "Tile")
	AMaumCrop* GetPlantedCrop() const { return PlantedCrop; }

	UFUNCTION(BlueprintPure, Category = "Tile")
	bool IsEmpty() const { return PlantedCrop == nullptr; }

	// 타일 인덱스 (세이브/로드 식별용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	int32 TileIndex = 0;

	// 현재 상호작용 모드 (UI에서 변경)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	EMaumInteractMode InteractMode = EMaumInteractMode::Water;

	// 심을 작물 ID (UI에서 선택)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FName SelectedCropID;

	// 작물 데이터테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	TObjectPtr<UDataTable> CropDataTable;

	// 심을 작물 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	TSubclassOf<AMaumCrop> CropActorClass;

	// 수확 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Tile|Event")
	FOnCropHarvested OnCropHarvested;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTileTouched(ETouchIndex::Type FingerIndex, AActor* TouchedActor);

	// 터치 피드백 애니메이션
	void PlayTouchFeedback();

	UFUNCTION()
	void RestoreTileScale();

	UPROPERTY(VisibleAnywhere, Category = "Tile")
	TObjectPtr<UStaticMeshComponent> TileMesh;

	// 이 타일에 심어진 작물
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	TObjectPtr<AMaumCrop> PlantedCrop;

private:
	FTimerHandle ScaleRestoreTimer;
};