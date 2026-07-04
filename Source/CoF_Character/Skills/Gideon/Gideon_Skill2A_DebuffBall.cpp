#include "Skills/Gideon/Gideon_Skill2A_DebuffBall.h"

#include "CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Interfaces/DebuffBallTargetInterface.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "TimerManager.h"
#include "TP_Character.h"

void UGideon_Skill2A_DebuffBall::ResetRuntime()
{
	NextAvailableTime = 0.0;

	ClearAllDebuffFX();
}

void UGideon_Skill2A_DebuffBall::ThrowProjectile()
{
	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return;
	}

	if (!C->CombatComp)
	{
		return;
	}

	UWorld* World = C->GetWorld();

	if (!World)
	{
		return;
	}

	if (!C->Skill2A_ProjectileClass)
	{
		return;
	}

	FVector SpawnLocation =
		C->GetActorLocation()
		+ C->GetActorForwardVector()
		* C->Skill2A_ProjectileSpawnForwardOffset
		+ FVector::UpVector
		* C->Skill2A_ProjectileSpawnZOffset;

	FRotator SpawnRotation = C->GetActorRotation();

	if (USkeletalMeshComponent* MeshComp = C->GetMesh())
	{
		if (
			C->Skill2A_ProjectileSpawnSocket != NAME_None
			&& MeshComp->DoesSocketExist(
				C->Skill2A_ProjectileSpawnSocket
			)
			)
		{
			SpawnLocation =
				MeshComp->GetSocketLocation(
					C->Skill2A_ProjectileSpawnSocket
				);
		}
	}

	// 평타 ThrowProjectile 기본형 그대로
	if (C->HasValidLockOnTarget())
	{
		if (AActor* LockTarget = C->GetLockOnTarget())
		{
			FVector TargetOrigin;
			FVector TargetExtent;

			LockTarget->GetActorBounds(
				true,
				TargetOrigin,
				TargetExtent
			);

			const FVector ShootDir =
				(TargetOrigin - SpawnLocation)
				.GetSafeNormal();

			if (!ShootDir.IsNearlyZero())
			{
				SpawnRotation = ShootDir.Rotation();
			}
		}
	}
	else
	{
		FVector HorizontalDir =
			C->GetActorForwardVector();

		HorizontalDir.Z = 0.f;
		HorizontalDir =
			HorizontalDir.GetSafeNormal();

		if (!HorizontalDir.IsNearlyZero())
		{
			SpawnRotation =
				HorizontalDir.Rotation();
		}
	}

	FActorSpawnParameters Params;
	Params.Owner = C;
	Params.Instigator = C;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACoF_CommonProjectile* Projectile =
		World->SpawnActor<ACoF_CommonProjectile>(
			C->Skill2A_ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

	if (!Projectile)
	{
		return;
	}

	// 직접 데미지는 0이며 적중 시 Debuff만 부여한다.
	Projectile->InitProjectile(
		C,
		C->CombatComp,
		this,
		0.f,
		C->Skill2A_ProjectileSpeed,
		C->Skill2A_ProjectileLifeSeconds,
		C->Skill2A_ProjectileRadius
	);
}

void UGideon_Skill2A_DebuffBall::ApplyDebuffToActor(
	AActor* Target
)
{
	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return;
	}

	if (!Target)
	{
		return;
	}

	if (
		!Target->GetClass()->ImplementsInterface(
			UDebuffBallTargetInterface::StaticClass()
		)
		)
	{
		return;
	}

	// 기존 보스 디버프 로직은 그대로 실행한다.
	IDebuffBallTargetInterface::Execute_ApplyDebuffBall(
		Target,
		C->Skill2A_DebuffDuration,
		C->Skill2A_DebuffIncomingDamageMultiplier
	);

	// 동일한 지속시간을 사용하는 디버프 FX를 대상에게 붙인다.
	SpawnOrRefreshDebuffFX(Target);
}

void UGideon_Skill2A_DebuffBall::SpawnOrRefreshDebuffFX(
	AActor* Target
)
{
	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return;
	}

	if (!Target)
	{
		return;
	}

	if (!C->GideonSkill2A_DebuffFXClass)
	{
		return;
	}

	if (C->Skill2A_DebuffDuration <= 0.f)
	{
		return;
	}

	UWorld* World = C->GetWorld();

	if (!World)
	{
		return;
	}

	const TWeakObjectPtr<AActor> TargetKey(Target);

	AActor* FXActor = nullptr;

	// 같은 대상에게 이미 이펙트가 붙어 있다면 기존 이펙트를 유지한다.
	if (
		TWeakObjectPtr<AActor>* ExistingFX =
		DebuffFXByTarget.Find(TargetKey)
		)
	{
		FXActor = ExistingFX->Get();

		if (!IsValid(FXActor))
		{
			DebuffFXByTarget.Remove(TargetKey);
			FXActor = nullptr;
		}
	}

	// 기존 FX가 없는 경우에만 새로 생성한다.
	if (!FXActor)
	{
		USkeletalMeshComponent* TargetMesh = nullptr;

		if (ACharacter* TargetCharacter =
			Cast<ACharacter>(Target))
		{
			TargetMesh = TargetCharacter->GetMesh();
		}
		else
		{
			TargetMesh =
				Target->FindComponentByClass<
				USkeletalMeshComponent
				>();
		}

		if (!TargetMesh)
		{
			return;
		}

		const FName SocketName =
			C->GideonSkill2A_DebuffFXSocketName;

		const bool bHasSocket =
			SocketName != NAME_None
			&& TargetMesh->DoesSocketExist(SocketName);

		// 지정한 소켓이 있으면 소켓 Transform을 사용하고,
		// 없으면 대상 Mesh의 Transform을 사용한다.
		const FTransform SpawnTransform =
			bHasSocket
			? TargetMesh->GetSocketTransform(
				SocketName,
				RTS_World
			)
			: TargetMesh->GetComponentTransform();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = C;
		SpawnParams.Instigator = C;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FXActor = World->SpawnActor<AActor>(
			C->GideonSkill2A_DebuffFXClass,
			SpawnTransform.GetLocation(),
			SpawnTransform.Rotator(),
			SpawnParams
		);

		if (!FXActor)
		{
			return;
		}

		bool bAttached = false;

		if (bHasSocket)
		{
			bAttached = FXActor->AttachToComponent(
				TargetMesh,
				FAttachmentTransformRules::
				SnapToTargetNotIncludingScale,
				SocketName
			);
		}
		else
		{
			bAttached = FXActor->AttachToComponent(
				TargetMesh,
				FAttachmentTransformRules::
				SnapToTargetNotIncludingScale
			);
		}

		if (!bAttached)
		{
			FXActor->Destroy();
			return;
		}

		FXActor->SetActorRelativeLocation(
			FVector::ZeroVector
		);

		FXActor->SetActorRelativeRotation(
			FRotator::ZeroRotator
		);

		DebuffFXByTarget.Add(
			TargetKey,
			FXActor
		);
	}

	// 같은 대상에게 디버프가 다시 적용되면
	// 기존 제거 타이머를 취소하고 지속시간을 처음부터 다시 계산한다.
	FTimerHandle& FXTimerHandle =
		DebuffFXTimerByTarget.FindOrAdd(TargetKey);

	World->GetTimerManager().ClearTimer(
		FXTimerHandle
	);

	FTimerDelegate RemoveFXDelegate;

	RemoveFXDelegate.BindUObject(
		this,
		&UGideon_Skill2A_DebuffBall::RemoveDebuffFX,
		TargetKey
	);

	World->GetTimerManager().SetTimer(
		FXTimerHandle,
		RemoveFXDelegate,
		C->Skill2A_DebuffDuration,
		false
	);
}

void UGideon_Skill2A_DebuffBall::RemoveDebuffFX(
	TWeakObjectPtr<AActor> TargetKey
)
{
	ATP_Character* C = GetOwnerChar();

	UWorld* World =
		C
		? C->GetWorld()
		: nullptr;

	if (
		FTimerHandle* TimerHandle =
		DebuffFXTimerByTarget.Find(TargetKey)
		)
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(
				*TimerHandle
			);
		}

		DebuffFXTimerByTarget.Remove(TargetKey);
	}

	if (
		TWeakObjectPtr<AActor>* FXPointer =
		DebuffFXByTarget.Find(TargetKey)
		)
	{
		AActor* FXActor = FXPointer->Get();

		if (IsValid(FXActor))
		{
			FXActor->Destroy();
		}

		DebuffFXByTarget.Remove(TargetKey);
	}
}

void UGideon_Skill2A_DebuffBall::ClearAllDebuffFX()
{
	ATP_Character* C = GetOwnerChar();

	UWorld* World =
		C
		? C->GetWorld()
		: nullptr;

	if (World)
	{
		for (auto& TimerPair : DebuffFXTimerByTarget)
		{
			World->GetTimerManager().ClearTimer(
				TimerPair.Value
			);
		}
	}

	DebuffFXTimerByTarget.Reset();

	for (auto& FXPair : DebuffFXByTarget)
	{
		AActor* FXActor = FXPair.Value.Get();

		if (IsValid(FXActor))
		{
			FXActor->Destroy();
		}
	}

	DebuffFXByTarget.Reset();
}