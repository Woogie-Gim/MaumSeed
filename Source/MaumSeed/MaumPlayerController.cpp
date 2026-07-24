
#include "MaumPlayerController.h"

AMaumPlayerController::AMaumPlayerController()
{
	// 마우스 커서 표시 및 클릭/터치 이벤트 활성화
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	bEnableMouseOverEvents = true;
	bEnableTouchOverEvents = true;
}


void AMaumPlayerController::BeginPlay()
{
	Super::BeginPlay();
}