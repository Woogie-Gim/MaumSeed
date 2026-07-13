
#include "MaumPlayerController.h"

void AMaumPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 커서 활성화
	bShowMouseCursor = true;

	// 클릭 이벤트 활성화
	bEnableClickEvents = true;

	// 터치 이벤트 활성화
	bEnableTouchEvents = true;
}