#include "MaumCrop.h"
#include "Kismet/GameplayStatics.h"
#include "MaumAIManager.h"
#include "MaumSaveGame.h"

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

void AMaumCrop::SaveCropState()
{
	// 세이브 게임 인스턴스 생성
	UMaumSaveGame* SaveGameInst = Cast<UMaumSaveGame>(UGameplayStatics::CreateSaveGameObject(UMaumSaveGame::StaticClass()));
	if (!SaveGameInst) return;

	// 현재 데이터 복사
	SaveGameInst->SavedCropID = CurrentCropID;
	SaveGameInst->SavedGrowth = CurrentGrowth;
	SaveGameInst->SavedStage = CurrentStage;

	// 지정된 슬롯에 데이터 저장
	UGameplayStatics::SaveGameToSlot(SaveGameInst, TEXT("CropSaveSlot"), 0);
	UE_LOG(LogTemp, Log, TEXT("작물 상태가 성공적으로 저장되었습니다."));
}

void AMaumCrop::LoadCropState()
{
	// 세이브 슬롯 존재 여부 확인
	if (UGameplayStatics::DoesSaveGameExist(TEXT("CropSaveSlot"), 0))
	{
		// 슬롯에서 데이터 로드
		UMaumSaveGame* LoadGameInst = Cast<UMaumSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("CropSaveSlot"), 0));
		if (LoadGameInst)
		{
			// 로드된 데이터 적용
			CurrentCropID = LoadGameInst->SavedCropID;
			CurrentGrowth = LoadGameInst->SavedGrowth;
			CurrentStage = LoadGameInst->SavedStage;

			UE_LOG(LogTemp, Log, TEXT("작물 상태를 불러왔습니다. (단계: %d, 누적 성장치: %d)"), CurrentStage, CurrentGrowth);
		}
	}
}

void AMaumCrop::CheatTimeSkip()
{
	UE_LOG(LogTemp, Warning, TEXT("[Cheat] 타임 스킵 발동! 하루 치 성장을 강제로 진행합니다."));

	// 어제 작성한 성장 연산 함수 재사용 (하루 요구치인 100 부여)
	UpdateCropGrowth(100);
}