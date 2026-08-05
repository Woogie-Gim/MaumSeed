#include "MaumPlayerController.h"
#include "MaumTile.h"

AMaumPlayerController::AMaumPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	bEnableMouseOverEvents = true;
	bEnableTouchOverEvents = true;
}

void AMaumPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("PlayerController BeginPlay 호출됨"));
	UpdateCursorIcon(CurrentTool);   // 시작 시 기본 도구 커서
}

void AMaumPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent 호출됨"));

	// 클릭 이벤트 시스템으로 좌클릭 처리
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AMaumPlayerController::OnClickPressed);
}

void AMaumPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bNow = IsInputKeyDown(EKeys::LeftMouseButton);
	if (bNow && !bWasClicking)   // 눌린 순간만
	{
		OnClickPressed();
	}
	bWasClicking = bNow;
}

void AMaumPlayerController::OnClickPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("클릭 감지됨"));

	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		UE_LOG(LogTemp, Warning, TEXT("히트: %s"), *GetNameSafe(Hit.GetActor()));

		if (AMaumTile* Tile = Cast<AMaumTile>(Hit.GetActor()))
		{
			Tile->ApplyTool(CurrentTool);
		}
	}
}

void AMaumPlayerController::SetTool(EMaumInteractMode NewTool)
{
	CurrentTool = NewTool;
	UpdateCursorIcon(NewTool);
}