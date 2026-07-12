

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MaumGridManager.generated.h"

class AMaumTile;

UCLASS()
class MAUMSEED_API AMaumGridManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMaumGridManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 스폰할 타일 클래스 지정
	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<AMaumTile> TileClass;

	// 밭 가로세로 개수
	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 GridSizeX = 4;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 GridSizeY = 4;

	// 타일 간격
	UPROPERTY(EditAnywhere, Category = "Grid")
	float TileSpacing = 150.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
