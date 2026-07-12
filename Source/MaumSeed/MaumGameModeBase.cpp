
#include "MaumGameModeBase.h"
#include "MaumPlayerController.h"
#include "MaumCameraPawn.h"

AMaumGameModeBase::AMaumGameModeBase()
{
	// 기본 컨트롤러 및 폰 클래스 덮어쓰기
	PlayerControllerClass = AMaumPlayerController::StaticClass();
	DefaultPawnClass = AMaumCameraPawn::StaticClass();
}