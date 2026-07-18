
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

// 일기 데이터 전송
void AMaumAIManager::SendDiaryToLLM(const FString& DiaryText)
{
	UE_LOG(LogTemp, Warning, TEXT("제미나이 API 통신 요청 시작!"));

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMaumAIManager::OnLLMResponseReceived);

	// 1. API 키 로드 검증
	FString ApiKey;
	bool bHasKey = GConfig->GetString(TEXT("MaumSeed.API"), TEXT("GeminiAPIKey"), ApiKey, GGameIni);

	if (!bHasKey || ApiKey.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultGame.ini에서 API 키를 불러오지 못했습니다! ini 설정을 확인해주세요."));
		return;
	}

	FString Endpoint = FString::Printf(TEXT("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.0-pro:generateContent?key=%s"), *ApiKey);
	UE_LOG(LogTemp, Log, TEXT("엔드포인트 세팅 완료 (API 키 길이: %d)"), ApiKey.Len());

	Request->SetURL(Endpoint);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	// 텍스트 파트 구성
	TSharedPtr<FJsonObject> TextPart = MakeShareable(new FJsonObject());
	TextPart->SetStringField("text", DiaryText);

	TArray<TSharedPtr<FJsonValue>> PartsArray;
	PartsArray.Add(MakeShareable(new FJsonValueObject(TextPart)));

	TSharedPtr<FJsonObject> ContentsObj = MakeShareable(new FJsonObject());
	ContentsObj->SetArrayField("parts", PartsArray);

	TArray<TSharedPtr<FJsonValue>> ContentsArray;
	ContentsArray.Add(MakeShareable(new FJsonValueObject(ContentsObj)));

	TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject());
	RequestObj->SetArrayField("contents", ContentsArray);

	// JSON 직렬화 검증
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);

	if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer))
	{
		UE_LOG(LogTemp, Log, TEXT("JSON 직렬화 성공: %s"), *JsonString);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("JSON 직렬화에 실패했습니다!"));
		return;
	}

	Request->SetContentAsString(JsonString);

	// HTTP 전송 시작 검증
	bool bRequestSent = Request->ProcessRequest();
	if (bRequestSent)
	{
		UE_LOG(LogTemp, Warning, TEXT("HTTP 요청 전송 성공! 응답을 대기합니다..."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HTTP 요청 전송 자체에 실패했습니다!"));
	}
}

// LLM 서버 응답 수신 및 처리
void AMaumAIManager::OnLLMResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// 통신 유효성 검증
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("LLM 통신 실패"));
		return;
	}

	// 수신된 원본 데이터 추출
	FString ResponseString = Response->GetContentAsString();

	// 파싱 전 서버 원본 응답 상태 강제 출력
	UE_LOG(LogTemp, Warning, TEXT("서버 원본 응답: %s"), *ResponseString);

	// JSON 역직렬화 객체 생성
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(ResponseString);
	TSharedPtr<FJsonObject> JsonObject;

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// 응답 텍스트 추출 (정상 응답 시)
		const TArray<TSharedPtr<FJsonValue>>* CandidatesArray;
		if (JsonObject->TryGetArrayField(TEXT("candidates"), CandidatesArray) && CandidatesArray->Num() > 0)
		{
			TSharedPtr<FJsonObject> ContentObject = (*CandidatesArray)[0]->AsObject()->GetObjectField(TEXT("content"));
			const TArray<TSharedPtr<FJsonValue>>* PartsArray;

			if (ContentObject->TryGetArrayField(TEXT("parts"), PartsArray) && PartsArray->Num() > 0)
			{
				FString ResultText = (*PartsArray)[0]->AsObject()->GetStringField(TEXT("text"));
				UE_LOG(LogTemp, Log, TEXT("추출된 AI 응답: %s"), *ResultText);
			}
		}
	}
}