#include "Debug/CoFDebug.h"

#include "Components/LineBatchComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

namespace CoFDebugPrivate
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

	// false: 게임 시작 시 디버그 출력 OFF
	// true : 게임 시작 시 디버그 출력 ON
	bool bEnabled = false;

	// 토글 상태 메시지를 같은 위치에 갱신하기 위한 고정 키
	constexpr uint64 ToggleMessageKey = 0xC0F70001ULL;

#endif
}

bool FCoFDebug::IsEnabled()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	return CoFDebugPrivate::bEnabled;
#else
	return false;
#endif
}

bool FCoFDebug::Toggle(UWorld* World)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	SetEnabled(!CoFDebugPrivate::bEnabled, World);
	return CoFDebugPrivate::bEnabled;
#else
	return false;
#endif
}

void FCoFDebug::SetEnabled(const bool bInEnabled, UWorld* World)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	CoFDebugPrivate::bEnabled = bInEnabled;

	if (!bInEnabled)
	{
		ClearExistingDebug(World);
	}

	PrintToggleState(bInEnabled);
#endif
}

void FCoFDebug::Print(
	const FString& Message,
	const float Duration,
	const FColor& Color,
	const int32 Key)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!CoFDebugPrivate::bEnabled || !GEngine)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		static_cast<uint64>(Key),
		Duration,
		Color,
		Message
	);
#endif
}

void FCoFDebug::DrawSphere(
	const UWorld* World,
	const FVector& Center,
	const float Radius,
	const int32 Segments,
	const FColor& Color,
	const bool bPersistentLines,
	const float LifeTime,
	const uint8 DepthPriority,
	const float Thickness)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!CoFDebugPrivate::bEnabled || !World)
	{
		return;
	}

	::DrawDebugSphere(
		World,
		Center,
		Radius,
		Segments,
		Color,
		bPersistentLines,
		LifeTime,
		DepthPriority,
		Thickness
	);
#endif
}

void FCoFDebug::DrawLine(
	const UWorld* World,
	const FVector& Start,
	const FVector& End,
	const FColor& Color,
	const bool bPersistentLines,
	const float LifeTime,
	const uint8 DepthPriority,
	const float Thickness)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!CoFDebugPrivate::bEnabled || !World)
	{
		return;
	}

	::DrawDebugLine(
		World,
		Start,
		End,
		Color,
		bPersistentLines,
		LifeTime,
		DepthPriority,
		Thickness
	);
#endif
}

void FCoFDebug::DrawPoint(
	const UWorld* World,
	const FVector& Position,
	const float Size,
	const FColor& Color,
	const bool bPersistentLines,
	const float LifeTime,
	const uint8 DepthPriority)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!CoFDebugPrivate::bEnabled || !World)
	{
		return;
	}

	::DrawDebugPoint(
		World,
		Position,
		Size,
		Color,
		bPersistentLines,
		LifeTime,
		DepthPriority
	);
#endif
}

void FCoFDebug::ClearExistingDebug(UWorld* World)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (GEngine)
	{
		GEngine->ClearOnScreenDebugMessages();
	}

	if (!World)
	{
		return;
	}

	if (ULineBatchComponent* WorldLineBatcher =
		World->GetLineBatcher(UWorld::ELineBatcherType::World))
	{
		WorldLineBatcher->Flush();
	}

	if (ULineBatchComponent* PersistentLineBatcher =
		World->GetLineBatcher(UWorld::ELineBatcherType::WorldPersistent))
	{
		PersistentLineBatcher->Flush();
	}

	if (ULineBatchComponent* ForegroundLineBatcher =
		World->GetLineBatcher(UWorld::ELineBatcherType::Foreground))
	{
		ForegroundLineBatcher->Flush();
	}

	if (ULineBatchComponent* ForegroundPersistentLineBatcher =
		World->GetLineBatcher(
			UWorld::ELineBatcherType::ForegroundPersistent))
	{
		ForegroundPersistentLineBatcher->Flush();
	}

	FlushDebugStrings(World);
#endif
}

void FCoFDebug::PrintToggleState(const bool bEnabled)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!GEngine)
	{
		return;
	}

	const FColor MessageColor = bEnabled
		? FColor::Green
		: FColor::Red;

	const FString Message = FString::Printf(
		TEXT("[CoF Debug] Visualization & Text: %s"),
		bEnabled ? TEXT("ON") : TEXT("OFF")
	);

	/*
	 * FCoFDebug::Print()를 사용하면 OFF 상태에서 메시지가 차단되므로
	 * 토글 상태만 직접 출력한다.
	 */
	GEngine->AddOnScreenDebugMessage(
		CoFDebugPrivate::ToggleMessageKey,
		2.0f,
		MessageColor,
		Message
	);
#endif
}