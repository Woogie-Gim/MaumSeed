#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MaumGameInstance.generated.h"

UCLASS()
class MAUMSEED_API UMaumGameInstance : public UGameInstance
{
	GENERATED_BODY()

	public:
	// 레벨을 넘어 유지되는 플레이어 닉네임
	UPROPERTY(BlueprintReadWrite, Category = "Player")
	FString PlayerName = TEXT("정원지기");
};