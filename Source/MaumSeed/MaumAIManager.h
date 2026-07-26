
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IHttpRequest.h"
#include "MaumAIManager.generated.h"

// 델리게이트 선언: 축복치 전달
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlessingReceived, int32, BlessingValue);

// AI 조언가 대사 전달
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAdviceReceived, const FString&, AdviceMessage);

UCLASS()
class MAUMSEED_API AMaumAIManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMaumAIManager();

	// 사용자 일기 데이터 LLM 서버 전송
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SendDiaryToLLM(const FString& DiaryText, const FString& WeatherText);

	// 이벤트 디스패처: 축복치 전달
	UPROPERTY(BlueprintAssignable, Category = "AI|Event")
	FOnBlessingReceived OnBlessingReceived;

	// 조언가 대사 수신 이벤트
	UPROPERTY(BlueprintAssignable, Category = "AI|Event")
	FOnAdviceReceived OnAdviceReceived;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// LLM 서버 응답 수신 및 처리
	void OnLLMResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
