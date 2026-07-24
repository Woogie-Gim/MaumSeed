

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MaumPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MAUMSEED_API AMaumPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	AMaumPlayerController();
};
