
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

	// 테스트 HTTP 요청 전송
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SendTestRequest();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// HTTP 응답 수신 콜백
	void OnTestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
