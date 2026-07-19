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

void AMaumAIManager::SendDiaryToLLM(const FString& DiaryText)
{
	UE_LOG(LogTemp, Warning, TEXT("Ollama API 통신 요청 시작!"));

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMaumAIManager::OnLLMResponseReceived);

	// 엔드포인트 및 헤더 설정
	FString Endpoint = TEXT("http://127.0.0.1:11434/api/generate");
	Request->SetURL(Endpoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(120.0f);

	// JSON 규격 설정
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("model"), TEXT("llama3"));
	JsonObject->SetNumberField(TEXT("keep_alive"), -1);

	// 프롬프트 병합
	FString FullPrompt = TEXT("너는 일기의 긍정적인 정도를 0에서 100 사이의 수치로 평가하는 시스템이야. 다음 일기 내용을 읽고 오직 '숫자'만 대답해. 다른 인사말이나 부가 설명은 절대 하지 마.\n일기 내용: ") + DiaryText;

	// 옵션 설정
	JsonObject->SetStringField(TEXT("prompt"), FullPrompt);
	JsonObject->SetBoolField(TEXT("stream"), false);

	// 직렬화
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);

	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		UE_LOG(LogTemp, Log, TEXT("JSON 직렬화 성공: %s"), *JsonString);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("JSON 직렬화 실패"));
		return;
	}

	Request->SetContentAsString(JsonString);

	// HTTP 요청
	if (Request->ProcessRequest())
	{
		UE_LOG(LogTemp, Warning, TEXT("HTTP 요청 전송 성공! 응답을 대기합니다..."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HTTP 요청 전송 실패"));
	}
}

void AMaumAIManager::OnLLMResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// 통신 검증
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("실패 상세: bWasSuccessful=%d, Status=%d, Code=%d"),
			bWasSuccessful, (int32)Request->GetStatus(),
			Response.IsValid() ? Response->GetResponseCode() : -1);
		return;
	}

	if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Error, TEXT("LLM 통신 에러! HTTP 상태 코드: %d, 상세 내용: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
		return;
	}

	// 수신 데이터 추출
	FString ResponseString = Response->GetContentAsString();
	UE_LOG(LogTemp, Warning, TEXT("서버 원본 응답: %s"), *ResponseString);

	// 역직렬화
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(ResponseString);
	TSharedPtr<FJsonObject> JsonObject;

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		FString ResultText;

		// 응답 텍스트 추출 및 처리
		if (JsonObject->TryGetStringField(TEXT("response"), ResultText))
		{
			ResultText.TrimStartAndEndInline();
			UE_LOG(LogTemp, Log, TEXT("추출된 AI 응답: %s"), *ResultText);

			// 정수 변환
			int32 BlessingValue = FCString::Atoi(*ResultText);

			// 축복치 브로드캐스트
			OnBlessingReceived.Broadcast(BlessingValue);
		}
	}
}