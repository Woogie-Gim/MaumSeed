

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MaumTile.generated.h"

UCLASS()
class MAUMSEED_API AMaumTile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMaumTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 타일 시각적 형태 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Tile")
	UStaticMeshComponent* TileMesh;

	// 터치 입력 호출 콜백 함수
	UFUNCTION()
	void OnTileTouched(ETouchIndex::Type FingerIndex, AActor* TouchedActor);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
