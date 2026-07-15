#pragma once

#include "CoreMinimal.h"

class UWorld;

/**
 * 프로젝트 전체 C++ 디버그 시각화 및 화면 메시지를 관리하는 공통 유틸리티.
 *
 * Development / Debug 빌드:
 * - F7 입력으로 전체 출력 상태를 켜고 끌 수 있다.
 *
 * Shipping / Test 빌드:
 * - 모든 함수가 자동으로 아무 동작도 하지 않는다.
 */
class COF_CHARACTER_API FCoFDebug final
{
public:
	/** 현재 디버그 출력 활성화 여부 */
	static bool IsEnabled();

	/** 현재 상태를 반전하고 변경된 상태를 반환 */
	static bool Toggle(UWorld* World = nullptr);

	/** 디버그 출력 상태를 직접 지정 */
	static void SetEnabled(bool bInEnabled, UWorld* World = nullptr);

	/** 화면 디버그 텍스트 출력 */
	static void Print(
		const FString& Message,
		float Duration = 1.5f,
		const FColor& Color = FColor::Cyan,
		int32 Key = -1
	);

	/** 디버그 구체 출력 */
	static void DrawSphere(
		const UWorld* World,
		const FVector& Center,
		float Radius,
		int32 Segments,
		const FColor& Color,
		bool bPersistentLines = false,
		float LifeTime = -1.f,
		uint8 DepthPriority = 0,
		float Thickness = 0.f
	);

	/** 디버그 선 출력 */
	static void DrawLine(
		const UWorld* World,
		const FVector& Start,
		const FVector& End,
		const FColor& Color,
		bool bPersistentLines = false,
		float LifeTime = -1.f,
		uint8 DepthPriority = 0,
		float Thickness = 0.f
	);

	/** 디버그 포인트 출력 */
	static void DrawPoint(
		const UWorld* World,
		const FVector& Position,
		float Size,
		const FColor& Color,
		bool bPersistentLines = false,
		float LifeTime = -1.f,
		uint8 DepthPriority = 0
	);

private:
	/** 현재 출력되어 있는 디버그 선과 텍스트를 제거 */
	static void ClearExistingDebug(UWorld* World);

	/** ON/OFF 변경 상태 메시지는 토글이 OFF여도 직접 출력 */
	static void PrintToggleState(bool bEnabled);
};