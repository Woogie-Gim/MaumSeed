#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MaumGridManager.generated.h"

class AMaumTile;
class UDataTable;

UCLASS()
class MAUMSEED_API AMaumGridManager : public AActor
{
	GENERATED_BODY()

	public:
	AMaumGridManager();

	// 생성된 타일 목록 반환 (DayManager가 호출)
	UFUNCTION(BlueprintPure, Category = "Grid")
	TArray<AMaumTile*> GetTiles() const;

	// 인덱스로 타일 조회 (세이브/로드용)
	UFUNCTION(BlueprintPure, Category = "Grid")
	AMaumTile* GetTileByIndex(int32 Index) const;

protected:
	virtual void BeginPlay() override;

	// 그리드 생성
	void GenerateGrid();

	// 스폰할 타일 클래스
	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<AMaumTile> TileClass;

	// 타일에 공통 적용할 데이터테이블
	UPROPERTY(EditAnywhere, Category = "Grid")
	TObjectPtr<UDataTable> CropDataTable;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 GridSizeX = 4;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 GridSizeY = 4;

	UPROPERTY(EditAnywhere, Category = "Grid")
	float TileSpacing = 150.0f;

	// 생성된 타일 보관
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	TArray<TObjectPtr<AMaumTile>> SpawnedTiles;
};