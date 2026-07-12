

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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
