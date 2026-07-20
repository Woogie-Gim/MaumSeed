#include "MaumCrop.h"
#include "Kismet/GameplayStatics.h"
#include "MaumAIManager.h"

AMaumCrop::AMaumCrop()
{
	PrimaryActorTick.bCanEverTick = false;

	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CropMesh"));
	RootComponent = CropMesh;

	CurrentGrowth = 0;
	CurrentStage = 0;
	CropData = nullptr;
}

void AMaumCrop::BeginPlay()
{
	Super::BeginPlay();

	// 데이터테이블과 고유 ID를 이용해 작물 데이터 캐싱
	if (CropDataTable && !CurrentCropID.IsNone())
	{
		CropData = CropDataTable->FindRow<FMaumCropData>(CurrentCropID, TEXT("CropDataCache"));
	}

	if (!CropData)
	{
		UE_LOG(LogTemp, Error, TEXT("AMaumCrop: 작물 데이터를 불러오지 못했습니다. DataTable과 CurrentCropID를 확인하세요."));
	}

	// 레벨 내 AI 매니저 검색 및 델리게이트 바인딩
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMaumAIManager::StaticClass());

	if (AMaumAIManager* AIManager = Cast<AMaumAIManager>(ManagerActor))
	{
		AIManager->OnBlessingReceived.AddDynamic(this, &AMaumCrop::OnBlessingReceivedHandler);
	}
}

void AMaumCrop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMaumCrop::OnBlessingReceivedHandler(int32 BlessingValue)
{
	// 델리게이트 수신 시 성장 판정 함수 호출
	UpdateCropGrowth(BlessingValue);
}

void AMaumCrop::UpdateCropGrowth(int32 BlessingValue)
{
	if (CropData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CropData가 유효하지 않습니다."));
		return;
	}

	// 이미 완숙 상태인 경우 추가 성장 방지
	if (CurrentStage >= CropData->GrowthDays)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 이미 완숙된 상태입니다."), *CropData->Name);
		return;
	}

	// 축복치 누적
	CurrentGrowth += BlessingValue;
	UE_LOG(LogTemp, Warning, TEXT("[%s] 축복치 %d 수신! (현재 누적 성장치: %d)"), *CropData->Name, BlessingValue, CurrentGrowth);

	// 다음 단계로 넘어가기 위한 요구치 (1일차=100, 2일차=200...)
	int32 NextStageRequirement = (CurrentStage + 1) * 100;

	// 요구치 도달 시 성장 단계 상승 처리
	if (CurrentGrowth >= NextStageRequirement)
	{
		AdvanceToNextStage();
	}
}

void AMaumCrop::AdvanceToNextStage()
{
	CurrentStage++;

	if (CropData && CurrentStage >= CropData->GrowthDays)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] 작물이 완전히 자랐습니다! 수확 가능 상태로 전환됩니다."), *CropData->Name);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] 작물이 %d 단계로 성장했습니다!"), *CropData->Name, CurrentStage);
	}
}