#include "MaumAIManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

static const FString ServerBaseURL = TEXT("https://maum-server.onrender.com");

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
	UE_LOG(LogTemp, Warning, TEXT("정원사 서버 통신 요청 시작!"));

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMaumAIManager::OnLLMResponseReceived);

	// 우리 서버 엔드포인트 (개발 중엔 localhost, 배포 시 실제 주소로 교체)
	FString Endpoint = ServerBaseURL + TEXT("/api/blessing");
	Request->SetURL(Endpoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(60.0f);   // Render sleep 대비 여유

	// 요청 바디: diary + weather
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("diary"), DiaryText);
	JsonObject->SetStringField(TEXT("weather"), WeatherText);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();
}

void AMaumAIManager::OnLLMResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Error, TEXT("서버 통신 실패: Status=%d, Code=%d"),
			(int32)Request->GetStatus(),
			Response.IsValid() ? Response->GetResponseCode() : -1);

		OnAdviceReceived.Broadcast(TEXT("정원사가 잠시 자리를 비웠어요."));
		OnBlessingReceived.Broadcast(50);
		return;
	}

	FString ResponseString = Response->GetContentAsString();
	UE_LOG(LogTemp, Warning, TEXT("서버 응답: %s"), *ResponseString);

	// 단일 JSON 파싱 (중첩 없음)
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(ResponseString);
	TSharedPtr<FJsonObject> JsonObject;

	FString AdviceMessage = TEXT("오늘도 수고했어요.");
	int32 BlessingValue = 50;

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		JsonObject->TryGetStringField(TEXT("message"), AdviceMessage);

		double BlessingNum = 50.0;
		if (JsonObject->TryGetNumberField(TEXT("blessing"), BlessingNum))
		{
			BlessingValue = FMath::Clamp((int32)BlessingNum, 0, 100);
		}

		UE_LOG(LogTemp, Warning, TEXT("AI 조언: %s | 축복치: %d"), *AdviceMessage, BlessingValue);
	}

	OnAdviceReceived.Broadcast(AdviceMessage);
	OnBlessingReceived.Broadcast(BlessingValue);
}

void AMaumAIManager::SubmitScore(const FString& PlayerName, int32 Score)
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMaumAIManager::OnScoreSubmitted);

	Request->SetURL(ServerBaseURL + TEXT("/api/score"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(60.0f);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("name"), PlayerName);
	JsonObject->SetNumberField(TEXT("score"), Score);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();

	UE_LOG(LogTemp, Log, TEXT("점수 전송: %s / %d점"), *PlayerName, Score);
}

void AMaumAIManager::OnScoreSubmitted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Log, TEXT("점수 등록 성공: %s"), *Response->GetContentAsString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("점수 등록 실패"));
	}
}

void AMaumAIManager::RequestLeaderboard()
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AMaumAIManager::OnLeaderboardResponse);

	Request->SetURL(ServerBaseURL + TEXT("/api/leaderboard"));
	Request->SetVerb(TEXT("GET"));
	Request->SetTimeout(60.0f);

	Request->ProcessRequest();
}

void AMaumAIManager::OnLeaderboardResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		FString Json = Response->GetContentAsString();
		UE_LOG(LogTemp, Log, TEXT("리더보드 수신: %s"), *Json);
		OnLeaderboardReceived.Broadcast(Json);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("리더보드 조회 실패"));
		OnLeaderboardReceived.Broadcast(TEXT("[]"));
	}
}