
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IHttpRequest.h"
#include "MaumAIManager.generated.h"

UCLASS()
class MAUMSEED_API AMaumAIManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMaumAIManager();

	// 사용자 일기 데이터 LLM 서버 전송
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SendDiaryToLLM(const FString& DiaryText);

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
