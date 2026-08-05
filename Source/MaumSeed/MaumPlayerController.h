#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MaumTile.h"   // EMaumInteractMode 사용
#include "MaumPlayerController.generated.h"

UCLASS()
class MAUMSEED_API AMaumPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMaumPlayerController();

	// 현재 선택된 도구 (UI 버튼이 변경)
	UPROPERTY(BlueprintReadWrite, Category = "Interact")
	EMaumInteractMode CurrentTool = EMaumInteractMode::Plant;

	// UI 버튼에서 호출: 도구 변경 + 커서 아이콘 갱신
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetTool(EMaumInteractMode NewTool);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	bool bWasClicking = false;   // 클릭 엣지 감지용

	// 클릭 시 타일을 찾아 현재 도구 적용
	void OnClickPressed();

	// 도구별 커서 아이콘 반영 (BP에서 구현)
	UFUNCTION(BlueprintImplementableEvent, Category = "Interact")
	void UpdateCursorIcon(EMaumInteractMode Tool);
};