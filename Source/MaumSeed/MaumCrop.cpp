#include "MaumCrop.h"
#include "Kismet/GameplayStatics.h"
#include "MaumAIManager.h"

AMaumCrop::AMaumCrop()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트 및 메시 컴포넌트 생성
	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CropMesh"));
	RootComponent = CropMesh;

	// 기능: 누적 성장치 초기화
	CurrentGrowth = 0;
}

void AMaumCrop::BeginPlay()
{
	Super::BeginPlay();

	// 레벨 내 AI 매니저 검색
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMaumAIManager::StaticClass());

	if (AMaumAIManager* AIManager = Cast<AMaumAIManager>(ManagerActor))
	{
		// AI 매니저 델리게이트에 수신 함수 바인딩
		AIManager->OnBlessingReceived.AddDynamic(this, &AMaumCrop::OnBlessingReceivedHandler);
	}
}

void AMaumCrop::OnBlessingReceivedHandler(int32 BlessingValue)
{
	// 데이터테이블 및 고유 ID 유효성 검증
	if (!CropDataTable || CurrentCropID.IsNone()) return;

	// 데이터테이블 작물 정보 검색
	FMaumCropData* CropData = CropDataTable->FindRow<FMaumCropData>(CurrentCropID, TEXT("CropGrowthCalc"));

	if (CropData)
	{
		// 축복치 수치만큼 현재 성장치 누적
		CurrentGrowth += BlessingValue;

		// 성장 상황 콘솔 로그 출력
		UE_LOG(LogTemp, Warning, TEXT("[%s] 축복치 %d 수신! (현재 성장치: %d)"), *CropData->Name, BlessingValue, CurrentGrowth);

		// 최대 성장치(임시로 GrowthDays 사용) 도달 판정
		if (CurrentGrowth >= CropData->GrowthDays * 100)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] 수확 가능 상태 도달!"), *CropData->Name);
		}
	}
}