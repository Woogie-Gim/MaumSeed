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

// 수확 임박 자막 중계 (DayManager가 구독)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileHarvestImminent, const FString&, Message);

UCLASS()
class MAUMSEED_API AMaumTile : public AActor
{
	GENERATED_BODY()

public:
	AMaumTile();

	// 현재 도구(모드)로 이 타일에 작용 (PlayerController가 호출)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void ApplyTool(EMaumInteractMode Tool, FName CropID);

	// 물주기 (타일이 심어진 작물에 위임)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void WaterPlantedCrop();

	// 비료 (타일이 심어진 작물에 위임)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void FertilizePlantedCrop();

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

	// 수확 임박 자막 중계 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Tile|Event")
	FOnTileHarvestImminent OnTileHarvestImminent;

	// 세이브 데이터로부터 작물 복원
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void RestoreCrop(FName CropID, int32 Growth, int32 Stage);

protected:
	virtual void BeginPlay() override;

	// 터치 피드백 애니메이션
	void PlayTouchFeedback();

	UFUNCTION()
	void RestoreTileScale();

	UPROPERTY(VisibleAnywhere, Category = "Tile")
	TObjectPtr<UStaticMeshComponent> TileMesh;

	// 이 타일에 심어진 작물
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	TObjectPtr<AMaumCrop> PlantedCrop;

	UFUNCTION()
	void HandleCropSelfHarvested(int32 Score);

	// 작물의 수확 임박 자막을 위로 중계
	UFUNCTION()
	void RelayHarvestImminent(const FString& Message);

	UFUNCTION()
	void OnTileClicked(AActor* TouchedActor, FKey ButtonPressed);

	UFUNCTION()
	void OnTileTouched(ETouchIndex::Type FingerIndex, AActor* TouchedActor);

private:
	FTimerHandle ScaleRestoreTimer;
};