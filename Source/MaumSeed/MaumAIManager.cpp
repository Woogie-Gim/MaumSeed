#include "MaumAIManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

AMaumAIManager::AMaumAIManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMaumAIManager::BeginPlay()
{
	Super::BeginPlay();
}

void AMaumAIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMaumAIManager::SendDiaryToLLM(const FString& DiaryText, const FString& WeatherText)
{
	UE_LOG(LogTemp, Warning, TEXT("Ollama API 통신 요청 시작!"));

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMaumAIManager::OnLLMResponseReceived);

	FString Endpoint = TEXT("http://127.0.0.1:11434/api/generate");
	Request->SetURL(Endpoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(180.0f);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("model"), TEXT("llama3"));
	JsonObject->SetNumberField(TEXT("keep_alive"), -1);
	JsonObject->SetBoolField(TEXT("stream"), false);
	JsonObject->SetStringField(TEXT("format"), TEXT("json"));   // JSON 모드

	// 프롬프트: 대사 + 축복치를 JSON으로 요청
	FString FullPrompt = FString::Printf(TEXT(
		"너는 '정원의 목소리'라는 힐링 게임 속 다정한 AI 정원사야. "
		"플레이어의 일기를 읽고 아래 JSON 형식으로만 대답해. 다른 말은 절대 하지 마.\n"
		"{\"message\": \"공감과 조언을 담은 따뜻한 한두 문장(한국어)\", \"blessing\": 0부터 100 사이 정수}\n"
		"오늘 날씨: %s\n"
		"일기: %s"), *WeatherText, *DiaryText);

	JsonObject->SetStringField(TEXT("prompt"), FullPrompt);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();
}

void AMaumAIManager::OnLLMResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// 통신 실패 시 폴백 (축복치 50 + 기본 대사)
	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Error, TEXT("LLM 실패 상세: bWasSuccessful=%d, Status=%d, Code=%d"),
			bWasSuccessful,
			(int32)Request->GetStatus(),
			Response.IsValid() ? Response->GetResponseCode() : -1);

		OnAdviceReceived.Broadcast(TEXT("정원사가 잠시 자리를 비웠어요."));
		OnBlessingReceived.Broadcast(50);
		return;
	}

	FString ResponseString = Response->GetContentAsString();
	UE_LOG(LogTemp, Warning, TEXT("서버 원본 응답: %s"), *ResponseString);

	// 1차 파싱: Ollama 응답 봉투에서 response 필드 추출
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(ResponseString);
	TSharedPtr<FJsonObject> OuterJson;

	if (!FJsonSerializer::Deserialize(Reader, OuterJson) || !OuterJson.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("1차 JSON 파싱 실패. 폴백 처리합니다."));
		OnAdviceReceived.Broadcast(TEXT("정원사의 목소리가 잘 들리지 않네요."));
		OnBlessingReceived.Broadcast(50);
		return;
	}

	FString InnerJsonString;
	OuterJson->TryGetStringField(TEXT("response"), InnerJsonString);

	// 2차 파싱: response 안의 실제 게임 데이터 추출
	TSharedRef<TJsonReader<TCHAR>> InnerReader = TJsonReaderFactory<TCHAR>::Create(InnerJsonString);
	TSharedPtr<FJsonObject> InnerJson;

	FString AdviceMessage = TEXT("오늘도 수고했어요.");
	int32 BlessingValue = 50;

	if (FJsonSerializer::Deserialize(InnerReader, InnerJson) && InnerJson.IsValid())
	{
		InnerJson->TryGetStringField(TEXT("message"), AdviceMessage);

		// blessing이 숫자 또는 문자열로 올 수 있어 방어적으로 처리
		double BlessingNum = 50.0;
		if (InnerJson->TryGetNumberField(TEXT("blessing"), BlessingNum))
		{
			BlessingValue = FMath::Clamp((int32)BlessingNum, 0, 100);
		}
		else
		{
			FString BlessingStr;
			if (InnerJson->TryGetStringField(TEXT("blessing"), BlessingStr))
			{
				BlessingValue = FMath::Clamp(FCString::Atoi(*BlessingStr), 0, 100);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("AI 조언: %s | 축복치: %d"), *AdviceMessage, BlessingValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("2차 JSON 파싱 실패. 원문을 대사로 사용합니다: %s"), *InnerJsonString);
		if (!InnerJsonString.IsEmpty())
		{
			AdviceMessage = InnerJsonString;   // 파싱 실패 시 원문이라도 보여줌
		}
	}

	// 대사 먼저, 축복치 나중 (축복치가 정산을 트리거하므로 순서 중요)
	OnAdviceReceived.Broadcast(AdviceMessage);
	OnBlessingReceived.Broadcast(BlessingValue);
}