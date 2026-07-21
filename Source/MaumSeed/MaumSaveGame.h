
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MaumSaveGame.generated.h"

UCLASS()
class MAUMSEED_API UMaumSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	// 기능: 저장할 작물의 고유 ID
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FName SavedCropID;

	// 기능: 저장할 누적 축복치
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 SavedGrowth;

	// 기능: 저장할 현재 성장 단계
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 SavedStage;
};
