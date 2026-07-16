
#include "MaumAIManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

// Sets default values
AMaumAIManager::AMaumAIManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMaumAIManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMaumAIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 가상의 REST API 서버로 GET 요청 전송
void AMaumAIManager::SendTestRequest()
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMaumAIManager::OnTestResponseReceived);
	Request->SetURL("https://jsonplaceholder.typicode.com/todos/1");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

// 서버 응답 검증 및 출력 로그 표시
void AMaumAIManager::OnTestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("통신 성공! 수신된 데이터: %s"), *Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("통신 실패. 인터넷 연결이나 URL을 확인하세요."));
	}
}