
#include "MaumAIManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

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

// 일기 데이터 전송
void AMaumAIManager::SendDiaryToLLM(const FString& DiaryText)
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMaumAIManager::OnLLMResponseReceived);

	// API 주소 및 키 할당
	FString ApiKey = TEXT("API Key");
	FString Endpoint = FString::Printf(TEXT("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=%s"), *ApiKey);

	// HTTP 헤더 설정
	Request->SetURL(Endpoint);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	// 텍스트 파트 구성
	TSharedPtr<FJsonObject> TextPart = MakeShareable(new FJsonObject());
	TextPart->SetStringField("text", DiaryText);

	// Parts 배열 구성
	TArray<TSharedPtr<FJsonValue>> PartsArray;
	PartsArray.Add(MakeShareable(new FJsonValueObject(TextPart)));

	// Contents 객체 구성
	TSharedPtr<FJsonObject> ContentsObj = MakeShareable(new FJsonObject());
	ContentsObj->SetArrayField("parts", PartsArray);

	// Contents 배열 구성
	TArray<TSharedPtr<FJsonValue>> ContentsArray;
	ContentsArray.Add(MakeShareable(new FJsonValueObject(ContentsObj)));

	// 최종 JSON 객체 생성
	TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject());
	RequestObj->SetArrayField("contents", ContentsArray);

	// 문자열 직렬화
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

	// 페이로드 탑재 및 전송
	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();
}

// LLM 서버 응답 수신 및 처리
void AMaumAIManager::OnLLMResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LLM 응답 성공: %s"), *Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LLM 통신 실패"));
	}
}