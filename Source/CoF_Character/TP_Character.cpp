#include "TP_Character.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"

// IA
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Components
#include "HealthComponent.h"
#include "CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"

// LockOn
#include "Kismet/GameplayStatics.h"
// LockOn UI
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

// Character Select
#include "InputCoreTypes.h"
#include "CharacterData.h"

// Animation
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

// ===== Terra Skills =====
#include "Skills/Terra/Terra_Skill1A_Dash.h"
#include "Skills/Terra/Terra_Skill1B_SlamAOE.h"
#include "Skills/Terra/Terra_Skill2A_ShieldPush.h"
#include "Skills/Terra/Terra_Skill2B_Spin.h"
#include "Skills/Terra/Terra_UltA_AllyShield.h"
#include "Skills/Terra/Terra_UltB_SelfShieldBuff.h"

// ===== Kallari Skills =====
#include "Skills/Kallari/Kallari_Skill1A_DashSlash.h"
#include "Skills/Kallari/Kallari_Skill1B_Backflip.h"
#include "Skills/Kallari/Kallari_Skill2A_ShurikenTeleport.h"
#include "Skills/Kallari/Kallari_Skill2B_ShurikenExplosion.h"
#include "Skills/Kallari/Kallari_UltA_BlinkDash.h"
#include "Skills/Kallari/Kallari_UltB_Invincible.h"

// ===== Gideon Skills =====
#include "Projectiles/CoF_CommonProjectile.h"		// 평타 Kallari 투사체 재사용
#include "Skills/Gideon/Gideon_Skill1A_WaterCannon.h"
#include "Skills/Gideon/Gideon_Skill1B_WaterBomb.h"
#include "Skills/Gideon/Gideon_Skill2A_DebuffBall.h"
#include "Skills/Gideon/Gideon_Skill2B_BackDash.h"
#include "Skills/Gideon/Gideon_UltA_MirrorWaterBeam.h"

#include "DrawDebugHelpers.h"

// Debug
static void ScreenDbg(const FString& Msg, float Sec = 1.5f, FColor Color = FColor::Cyan)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Sec, Color, Msg);
	}
}

ATP_Character::ATP_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	// ===== Camera Boom / Follow Camera (ThirdPerson 템플릿 기본) =====
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true; // 마우스 회전에 따라 붐이 돈다

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	// ===== HealthComponent =====
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	// ===== CombatComponent =====
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));

	// ===== 회전/이동 기본 정책 =====
	bUseControllerRotationYaw = false; // 캐릭터는 컨트롤러 yaw로 직접 돌리지 않고
	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로 회전
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
}

void ATP_Character::BeginPlay()
{
	Super::BeginPlay();

	// ===== 캐릭터 데이터는 항상 먼저 적용 =====
	if (CharacterSlots.Num() > 0 && CharacterSlots[0])
	{
		SelectCharacterSlot(0);
	}
	else if (DefaultCharacterData)
	{
		ApplyCharacterData(DefaultCharacterData);
	}

	// ===== 락온 카메라 기본값 저장 =====
	if (CameraBoom)
	{
		DefaultCameraArmLength = CameraBoom->TargetArmLength;
		DefaultCameraSocketOffset = CameraBoom->SocketOffset;
	}

	// ===== 로컬 플레이어(내 화면)에서만 MappingContext 추가 =====
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsys) return;

	if (DefaultMappingContext)
	{
		Subsys->AddMappingContext(DefaultMappingContext, 0);
		// ===== 락온 UI 준비 =====
		EnsureLockOnWidget();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Input] DefaultMappingContext is null. Set it in BP child."));
	}
}

// 입력 잠금
bool ATP_Character::CanMoveInput() const { return bCanMoveInput; }
bool ATP_Character::CanAttackInput() const { return bCanAttackInput; }
bool ATP_Character::CanGuardInput() const { return bCanGuardInput; }
bool ATP_Character::CanSkillInput() const { return bCanSkillInput; }
bool ATP_Character::CanJumpInput() const { return bCanJumpInput; }

void ATP_Character::SetMoveInputEnabled(bool bEnable) { bCanMoveInput = bEnable; }
void ATP_Character::SetAttackInputEnabled(bool bEnable) { bCanAttackInput = bEnable; }
void ATP_Character::SetGuardInputEnabled(bool bEnable) { bCanGuardInput = bEnable; }
void ATP_Character::SetSkillInputEnabled(bool bEnable) { bCanSkillInput = bEnable; }
void ATP_Character::SetJumpInputEnabled(bool bEnable) { bCanJumpInput = bEnable; }

void ATP_Character::SetEveryInputEnabled(bool bEnable)
{
	SetMoveInputEnabled(bEnable);
	SetAttackInputEnabled(bEnable);
	SetGuardInputEnabled(bEnable);
	SetSkillInputEnabled(bEnable);
	SetJumpInputEnabled(bEnable);
}


// Tick(우선 락온에서 사용)
void ATP_Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bLockOnEnabled && !HasValidLockOnTarget())
	{
		ClearLockOn();
	}

	if (bLockOnEnabled && HasValidLockOnTarget())
	{
		UpdateLockOnRotation(DeltaSeconds);
	}

	UpdateLockOnCamera(DeltaSeconds);
	UpdateLockOnWidget();
}

// 락온 유효성 검사
bool ATP_Character::HasValidLockOnTarget() const
{
	if (!IsValid(LockOnTarget))
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), LockOnTarget->GetActorLocation());
	return DistSq <= FMath::Square(LockOnMaxDistance);
}

// 보스 락온 대상 찾기
void ATP_Character::RefreshBossLockOnTarget()
{
	LockOnTarget = nullptr;

	TArray<AActor*> BossActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), BossLockOnTag, BossActors);

	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* Boss : BossActors)
	{
		if (!IsValid(Boss) || Boss == this)
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(GetActorLocation(), Boss->GetActorLocation());
		if (DistSq > FMath::Square(LockOnMaxDistance))
		{
			continue;
		}

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			LockOnTarget = Boss;
		}
	}
}

// 몸체를 락온 대상으로 돌린다.
void ATP_Character::UpdateLockOnRotation(float DeltaSeconds)
{
	if (!HasValidLockOnTarget())
	{
		return;
	}

	FVector ToTarget = LockOnTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRot = ToTarget.Rotation();

	const FRotator NewActorRot = FMath::RInterpTo(
		GetActorRotation(),
		FRotator(0.f, TargetRot.Yaw, 0.f),
		DeltaSeconds,
		LockOnRotateSpeed
	);

	SetActorRotation(NewActorRot);

	if (Controller)
	{
		const FRotator CurrentControlRot = Controller->GetControlRotation();
		const FRotator DesiredControlRot(CurrentControlRot.Pitch, TargetRot.Yaw, CurrentControlRot.Roll);

		const FRotator NewControlRot = FMath::RInterpTo(
			CurrentControlRot,
			DesiredControlRot,
			DeltaSeconds,
			LockOnRotateSpeed
		);

		Controller->SetControlRotation(NewControlRot);
	}
}

// 락온 해제
void ATP_Character::ClearLockOn()
{
	bLockOnEnabled = false;
	LockOnTarget = nullptr;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
	}
	if (LockOnWidgetInstance)
	{
		LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}

// 락온 중이면 카메라를 조금 뒤로 빼고 보정 / 락온 해제 시 원래 카메라로 부드럽게 복귀
void ATP_Character::UpdateLockOnCamera(float DeltaSeconds)
{
	if (!CameraBoom)
	{
		return;
	}

	const bool bUseLockOnCamera = bLockOnEnabled && HasValidLockOnTarget();

	const float DesiredArmLength = bUseLockOnCamera ? LockOnCameraArmLength : DefaultCameraArmLength;
	const FVector DesiredSocketOffset = bUseLockOnCamera ? LockOnCameraSocketOffset : DefaultCameraSocketOffset;

	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		DesiredArmLength,
		DeltaSeconds,
		LockOnCameraInterpSpeed
	);

	CameraBoom->SocketOffset = FMath::VInterpTo(
		CameraBoom->SocketOffset,
		DesiredSocketOffset,
		DeltaSeconds,
		LockOnCameraInterpSpeed
	);
}

// 락온 마커 위젯을 생성해서 뷰포트에 올림
void ATP_Character::EnsureLockOnWidget()
{
	if (LockOnWidgetInstance || !LockOnWidgetClass)
	{
		return;
	}

	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	LockOnWidgetInstance = CreateWidget<UUserWidget>(PC, LockOnWidgetClass);
	if (!LockOnWidgetInstance)
	{
		return;
	}

	LockOnWidgetInstance->AddToViewport(50);

	// 위젯 중심이 화면 좌표 기준점이 되도록 설정
	LockOnWidgetInstance->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));

	// 작은 마커 위젯 크기
	LockOnWidgetInstance->SetDesiredSizeInViewport(FVector2D(40.f, 40.f));

	LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
}

// 락온 위젯의 위치를 플레이어의 위치로 업데이트한다.
void ATP_Character::UpdateLockOnWidget()
{
	EnsureLockOnWidget();

	if (!LockOnWidgetInstance)
	{
		return;
	}

	if (!(bLockOnEnabled && HasValidLockOnTarget()))
	{
		LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const FVector WorldPos = LockOnTarget->GetActorLocation();

	FVector2D ScreenPos;
	const bool bProjected =
		UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldPos, ScreenPos, false);

	if (!bProjected)
	{
		LockOnWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	LockOnWidgetInstance->SetPositionInViewport(ScreenPos, false);
	LockOnWidgetInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
}

// 플레이어 세팅
void ATP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 기본 IA
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Error, TEXT("[Input] EnhancedInputComponent missing. Check Enhanced Input plugin / project setup."));
		return;
	}

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATP_Character::Input_Move);
	}

	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATP_Character::Input_Look);
	}

	// Jump는 Started/Completed로 나눠서 처리하는 게 일반적
	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ATP_Character::Input_JumpStarted);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATP_Character::Input_JumpCompleted);
	}

	// 락온 좌컨트롤
	if (LockOnAction)
	{
		EIC->BindAction(LockOnAction, ETriggerEvent::Started, this, &ATP_Character::Input_LockOnToggle);
	}

	// 좌클릭 콤보 공격
	if (AttackAction)
	{
		EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ATP_Character::Input_AttackStarted);
	}

	// 우클릭 방패 들기
	if (BlockAction)
	{
		EIC->BindAction(BlockAction, ETriggerEvent::Started, this, &ATP_Character::Input_BlockStarted);
		EIC->BindAction(BlockAction, ETriggerEvent::Completed, this, &ATP_Character::Input_BlockCompleted);
	}

	// 스킬1
	if (Skill1Action)
	{
		EIC->BindAction(Skill1Action, ETriggerEvent::Started, this, &ATP_Character::Input_Skill1Started);
	}

	// 스킬2
	if (Skill2Action)
	{
		EIC->BindAction(Skill2Action, ETriggerEvent::Started, this, &ATP_Character::Input_Skill2Started);
	}

	// 궁극기
	if (UltAction)
	{
		EIC->BindAction(UltAction, ETriggerEvent::Started, this, &ATP_Character::Input_UltStarted);
	}

	// 1~5 키로 캐릭터 교체
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ATP_Character::SelectSlot1);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ATP_Character::SelectSlot2);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ATP_Character::SelectSlot3);
	PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ATP_Character::SelectSlot4);
	PlayerInputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ATP_Character::SelectSlot5);

	// 피격 디버깅
	PlayerInputComponent->BindKey(EKeys::F7, IE_Pressed, this, &ATP_Character::Debug_ForceHit);		// 지금은 F7키에 바인딩
}

// IA 관련
void ATP_Character::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Move = Value.Get<FVector2D>();
	if (!Controller) return;

	if (!CanMoveInput())
		return;

	const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Move.Y);
	AddMovementInput(Right, Move.X);
}

void ATP_Character::Input_Look(const FInputActionValue& Value)
{
	if (bHitReactInputLocked)
		return;
	
	const FVector2D Look = Value.Get<FVector2D>();

	AddControllerYawInput(Look.X);
	AddControllerPitchInput(Look.Y);
}

void ATP_Character::Input_JumpStarted(const FInputActionValue& Value)
{
	if (!CanJumpInput())
		return;

	SetMoveInputEnabled(true);
	SetAttackInputEnabled(true);
	SetGuardInputEnabled(true);
	SetSkillInputEnabled(false);

	bJumpAccepted = true;

	Jump();
}

void ATP_Character::Input_JumpCompleted(const FInputActionValue& Value)
{
	StopJumping();

	if (bJumpAccepted) SetEveryInputEnabled(true);

	bJumpAccepted = false;
}

void ATP_Character::Input_LockOnToggle(const FInputActionValue& Value)
{
	if (bHitReactInputLocked)
		return;

	if (bLockOnEnabled)
	{
		ClearLockOn();
		return;
	}

	RefreshBossLockOnTarget();

	if (!HasValidLockOnTarget())
	{
		return;
	}

	bLockOnEnabled = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = false;
	}
}

void ATP_Character::Input_AttackStarted(const FInputActionValue& Value)
{
	// 콤보가 아예 안 돌고 있으면 첫타 시작
	if (!PrimaryComboMontage) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	if (!CanAttackInput())
		return;

	SetMoveInputEnabled(true);
	SetAttackInputEnabled(true);
	SetGuardInputEnabled(false);
	SetSkillInputEnabled(false);
	SetJumpInputEnabled(true);

	// 애니메이션이 아예 안 돌고 있다면
	if (!Anim->Montage_IsPlaying(PrimaryComboMontage))
	{
		ComboIndex = 0;
		bComboQueued = false;

		PlayAnimMontage(PrimaryComboMontage, 1.f, FName(TEXT("A")));
		bAttackPressed = false; // 첫타는 지금 소비
		return;
	}

	// 애니메이션이 도는 중 IA를 받음 -> 콤보 영역이라면
	if (bComboWindowOpen) {
		bAttackPressed = true;
		bComboQueued = true;			// 예약됨 표시용,, 지금은 크게 안 중요함.
		ScreenDbg(TEXT("Notify: attack queued"), 1.5f, FColor::Yellow);
	}
}

// 콤보 받는 범위 - 이 타이밍에 입력이 들어왔으면 다음타 예약
void ATP_Character::ComboWindowOpen()
{
	bComboWindowOpen = true;
	ScreenDbg(TEXT("Notify: ComboWindowOpen"), 1.5f, FColor::Green);
}

void ATP_Character::ComboWindowClose()
{
	bComboWindowOpen = false;
	ScreenDbg(TEXT("Notify: ComboWindowClose"), 1.5f, FColor::Red);
}

// -------기본 공격(콤보) Notify-------
void ATP_Character::SaveAttack()
{
	if (!bAttackPressed) return;

	bAttackPressed = false;
	//bComboQueued = false;			// 예약됨 표시용,, 지금은 크게 안 중요함.

	// A->B
	if (PrimaryComboMontage && ComboIndex == 0)
	{
		if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			Anim->Montage_SetNextSection(FName(TEXT("A")), FName(TEXT("B")), PrimaryComboMontage);
			ComboIndex = 1;
		}
	}
}

// 끝
void ATP_Character::ResetCombo()
{
	// 예약이 없으면 콤보 끊기
	if (!bComboQueued)
	{
		ComboIndex = 0;
	}
	bComboQueued = false;
	bAttackPressed = false;

	// Skill2B가 활성인 경우에만 종료 정리 (런타임은 스킬 객체가 관리)
	if (Terra_Skill2B && Terra_Skill2B->IsActive())
	{
		// 판정 종료
		if (CombatComp)
			CombatComp->EndHitWindow();

		// 타이머/EndTime/Active 정리
		Terra_Skill2B->CancelSpin();
	}

	SetEveryInputEnabled(true);		// input 받기
}

// Hit 판정할 영역
void ATP_Character::HitStart()
{
	if (!CombatComp) return;

	switch (PrimaryAttackHitType)
	{
	case EPrimaryAttackHitType::Projectile:
		PrimaryAttack_ThrowProjectile();
		return;

	case EPrimaryAttackHitType::Sphere:
		CombatComp->ConfigureAOEForwardHit(
			CombatComp->Damage,
			PrimaryAttackSphereRadius,
			PrimaryAttackForwardOffset,
			PrimaryAttackHalfAngleDeg
		);
		break;

	case EPrimaryAttackHitType::LineTrace:
	default:
		CombatComp->ConfigureTraceHit(CombatComp->Damage, CombatComp->TraceRange);
		break;
	}

	// 이번 타 시작: 1회 히트 가능 상태로 초기화
	CombatComp->BeginHitWindow_OneShot();
}

void ATP_Character::HitEnd()
{
	if (CombatComp)
	{
		CombatComp->EndHitWindow();
	}
}

// 투사체 평타
void ATP_Character::PrimaryAttack_ThrowProjectile()
{
	if (!CombatComp) return;
	if (!PrimaryProjectileClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const bool bSecondCombo = (ComboIndex == 1);
	const FName SpawnSocket = bSecondCombo ? PrimaryProjectileSocketB : PrimaryProjectileSocketA;

	FVector SpawnLocation =
		GetActorLocation()
		+ GetActorForwardVector() * PrimaryProjectileSpawnForwardOffset
		+ FVector(0.f, 0.f, PrimaryProjectileSpawnZOffset);

	FRotator SpawnRotation = GetActorRotation();

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (SpawnSocket != NAME_None && MeshComp->DoesSocketExist(SpawnSocket))
		{
			SpawnLocation = MeshComp->GetSocketLocation(SpawnSocket);
			SpawnRotation = MeshComp->GetSocketRotation(SpawnSocket);
		}
	}

	// 락온 대상이 있으면 대상에게 발사
	if (HasValidLockOnTarget())
	{
		AActor* LockTarget = GetLockOnTarget();
		if (IsValid(LockTarget))
		{
			FVector TargetOrigin, TargetExtent;
			LockTarget->GetActorBounds(true, TargetOrigin, TargetExtent);

			const FVector ShootDir = (TargetOrigin - SpawnLocation).GetSafeNormal();
			if (!ShootDir.IsNearlyZero())
			{
				SpawnRotation = ShootDir.Rotation();
			}
		}
	}
	// 락온이 없으면 지면 수평 발사
	else
	{
		FVector HorizontalDir = GetActorForwardVector();
		HorizontalDir.Z = 0.f;
		HorizontalDir = HorizontalDir.GetSafeNormal();

		if (!HorizontalDir.IsNearlyZero())
		{
			SpawnRotation = HorizontalDir.Rotation();
		}
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACoF_CommonProjectile* Projectile =
		World->SpawnActor<ACoF_CommonProjectile>(
			PrimaryProjectileClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

	if (!Projectile) return;

	Projectile->InitProjectile(
		this,
		CombatComp,
		nullptr, // 기본 평타는 OwningSkill 불필요
		CombatComp->Damage,
		PrimaryProjectileSpeed,
		PrimaryProjectileLifeSeconds,
		PrimaryProjectileRadius
	);

#if !(UE_BUILD_SHIPPING)
	DrawDebugSphere(World, SpawnLocation, PrimaryProjectileRadius, 16, FColor::Cyan, false, 1.5f, 0, 1.5f);
	DrawDebugLine(World, SpawnLocation, SpawnLocation + SpawnRotation.Vector() * 200.f, FColor::Blue, false, 1.5f, 0, 1.5f);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, FString::Printf(TEXT("[PrimaryProjectile] Damage=%.1f Speed=%.1f Radius=%.1f"), CombatComp->Damage, PrimaryProjectileSpeed, PrimaryProjectileRadius));
	}
#endif
}

// --------- 우클릭 방패 들기 ---------
void ATP_Character::Input_BlockStarted(const FInputActionValue&)
{
	if (!CanGuardInput())
		return;

	if (bBlocking) return;
	bBlocking = true;

	// 데미지 감소 적용
	//if (HealthComp)
	//	HealthComp->SetDamageMultiplier(BlockDamageMultiplier);

	// 상체 방패 몽타주 재생
	if (BlockHoldMontage)
		PlayAnimMontage(BlockHoldMontage);

	// 이동속도 살짝 낮추기
	GetCharacterMovement()->MaxWalkSpeed *= 0.65f;
}

void ATP_Character::Input_BlockCompleted(const FInputActionValue&)
{
	if (!bBlocking) return;
	bBlocking = false;

	//if (HealthComp)
	//	HealthComp->SetDamageMultiplier(1.0f);

	// 몽타주 중단(블렌드아웃)
	if (BlockHoldMontage)
		StopAnimMontage(BlockHoldMontage);

	// 이동속도 다시 복구
	GetCharacterMovement()->MaxWalkSpeed = DefaultCharacterData->MaxWalkSpeed;
}

// 피격
void ATP_Character::OnHitReact_Implementation(float DamageAmount, const FVector& HitPoint, const FVector& HitNormal)
{
	// 이미 피격 중이면 무시 (잠시 무적)
	if (!bCanBeHit)
		return;

	// 1) 데미지 적용
	if (HealthComp)
	{
		HealthComp->ApplyDamage_Local(DamageAmount);
	}

	// 2) 피격 애니 재생
	if (!HitReactMontage) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	// 이미 피격 몽타주가 돌고 있으면 재시작하지 않게
	if (Anim->Montage_IsPlaying(HitReactMontage))
		return;

	// 피격 중에는 입력 잠금 + 추가 피격 무시
	bHitReacting = true;
	bCanBeHit = false;
	bHitReactInputLocked = true;
	SetEveryInputEnabled(false);

	// 추후 방향에 따라 섹션 선택(Front/Back/Left/Right) 으로 교체할지 생각해봐야함.
	// 일단 단일 섹션이면 그냥 Play
	PlayAnimMontage(HitReactMontage, HitReactPlayRate);
}

void ATP_Character::HitReactEnd()
{
	bHitReacting = false;
	bCanBeHit = true;
	bHitReactInputLocked = false;
	SetEveryInputEnabled(true);
}

void ATP_Character::Debug_ForceHit()
{
	// 임의 데미지, 임의 히트포인트/노멀
	const float DamageAmount = 10.f;
	const FVector HitPoint = GetActorLocation() + GetActorForwardVector() * 50.f;
	const FVector HitNormal = -GetActorForwardVector();

	OnHitReact_Implementation(DamageAmount, HitPoint, HitNormal);
}

// -------- 스킬 1 ---------
// input
void ATP_Character::Input_Skill1Started(const FInputActionValue&)
{
	if (Skill1Selected == ESkillVariant::None)
		return;

	// 스킬 객체가 아직 없으면(초기화 전에 입력 들어오는 경우) 방어
	const bool bSkill1AReady =
		(Skill1A_Implementation == ESkill1AImplementation::TerraDash && Terra_Skill1A) ||
		(Skill1A_Implementation == ESkill1AImplementation::KallariDashSlash && Kallari_Skill1A) ||
		(Skill1A_Implementation == ESkill1AImplementation::GideonWaterCannon && Gideon_Skill1A);

	const bool bSkill1BReady =
		(Skill1B_Implementation == ESkill1BImplementation::TerraAxeSlam && Terra_Skill1B) ||
		(Skill1B_Implementation == ESkill1BImplementation::KallariBackflip && Kallari_Skill1B) ||
		(Skill1B_Implementation == ESkill1BImplementation::GideonWaterBomb && Gideon_Skill1B);

	if ((Skill1Selected == ESkillVariant::A && !bSkill1AReady) ||
		(Skill1Selected == ESkillVariant::B && !bSkill1BReady))
	{
		return;
	}

	// 쿨다운 디버깅 메세지
	const double Now = GetWorld()->GetTimeSeconds();

	if (Skill1Selected == ESkillVariant::A)
	{
		bool bInCooldown = false;

		if (Skill1A_Implementation == ESkill1AImplementation::TerraDash && Terra_Skill1A)
		{
			bInCooldown = Terra_Skill1A->IsInCooldown(Now);
		}
		else if (Skill1A_Implementation == ESkill1AImplementation::KallariDashSlash && Kallari_Skill1A)
		{
			bInCooldown = Kallari_Skill1A->IsInCooldown(Now);
		}
		else if (Skill1A_Implementation == ESkill1AImplementation::GideonWaterCannon && Gideon_Skill1A)
		{
			bInCooldown = Gideon_Skill1A->IsInCooldown(Now);
		}

		if (bInCooldown)
		{
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}
	else if (Skill1Selected == ESkillVariant::B)
	{
		bool bInCooldown = false;

		if (Skill1B_Implementation == ESkill1BImplementation::TerraAxeSlam && Terra_Skill1B)
		{
			bInCooldown = Terra_Skill1B->IsInCooldown(Now);
		}
		else if (Skill1B_Implementation == ESkill1BImplementation::KallariBackflip && Kallari_Skill1B)
		{
			bInCooldown = Kallari_Skill1B->IsInCooldown(Now);
		}
		else if (Skill1B_Implementation == ESkill1BImplementation::GideonWaterBomb && Gideon_Skill1B)
		{
			bInCooldown = Gideon_Skill1B->IsInCooldown(Now);
		}

		if (bInCooldown)
		{
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}

	if (const UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->IsFalling())			// 공중 상태일 때 막기를 따로 - 입력을 안받아도 떨어지는 경우 시전 등...
			return;
	}

	StopJumping();

	if (!CanSkillInput())
		return;

	SetMoveInputEnabled(false);
	SetAttackInputEnabled(false);
	SetGuardInputEnabled(false);
	SetSkillInputEnabled(false);
	SetJumpInputEnabled(false);

	UAnimMontage* Montage = nullptr;
	if (Skill1Selected == ESkillVariant::A) Montage = Skill1MontageA;
	else if (Skill1Selected == ESkillVariant::B) Montage = Skill1MontageB;

	if (!Montage) return;

	PlayAnimMontage(Montage);

	// 쿨다운 시작 (런타임 상태는 스킬 객체가 관리)
	if (Skill1Selected == ESkillVariant::A)
	{
		if (Skill1A_Implementation == ESkill1AImplementation::TerraDash && Terra_Skill1A)
		{
			Terra_Skill1A->StartCooldown(Now, Skill1A_Cooldown);
		}
		else if (Skill1A_Implementation == ESkill1AImplementation::KallariDashSlash && Kallari_Skill1A)
		{
			Kallari_Skill1A->StartCooldown(Now, Skill1A_Cooldown);
		}
		else if (Skill1A_Implementation == ESkill1AImplementation::GideonWaterCannon && Gideon_Skill1A)
		{
			Gideon_Skill1A->StartCooldown(Now, Skill1A_Cooldown);
		}
	}
	else if (Skill1Selected == ESkillVariant::B)
	{
		if (Skill1B_Implementation == ESkill1BImplementation::TerraAxeSlam && Terra_Skill1B)
		{
			Terra_Skill1B->StartCooldown(Now, Skill1B_Cooldown);
		}
		else if (Skill1B_Implementation == ESkill1BImplementation::KallariBackflip && Kallari_Skill1B)
		{
			Kallari_Skill1B->StartCooldown(Now, Skill1B_Cooldown);
		}
		else if (Skill1B_Implementation == ESkill1BImplementation::GideonWaterBomb && Gideon_Skill1B)
		{
			Gideon_Skill1B->StartCooldown(Now, Skill1B_Cooldown);
		}
	}
}

// ===== Skill1_A 돌진 (wrapper) =====
void ATP_Character::Skill1A_HitStart()
{
	if (Skill1A_Implementation == ESkill1AImplementation::TerraDash && Terra_Skill1A)
	{
		Terra_Skill1A->HitStart();
	}

	else if (Skill1A_Implementation == ESkill1AImplementation::KallariDashSlash && Kallari_Skill1A)
	{
		Kallari_Skill1A->HitStart();
	}

	else if (Skill1A_Implementation == ESkill1AImplementation::GideonWaterCannon && Gideon_Skill1A)
	{
		Gideon_Skill1A->HitStart();
	}
}

void ATP_Character::Skill1A_HitEnd()		//역시나 당장은 필요없는듯 기존 hitend 돌려쓰는중 나중에 필요하면 바꾸자.
{
	if (Skill1A_Implementation == ESkill1AImplementation::TerraDash && Terra_Skill1A)
	{
		Terra_Skill1A->HitEnd();
	}

	else if (Skill1A_Implementation == ESkill1AImplementation::KallariDashSlash && Kallari_Skill1A)
	{
		Kallari_Skill1A->HitEnd();
	}

	else if (Skill1A_Implementation == ESkill1AImplementation::GideonWaterCannon && Gideon_Skill1A)
	{
		Gideon_Skill1A->HitEnd();
	}
}

void ATP_Character::Skill1A_DashStart()
{
	if (Skill1A_Implementation == ESkill1AImplementation::TerraDash && Terra_Skill1A)
	{
		Terra_Skill1A->DashStart();
		return;
	}

	if (Skill1A_Implementation == ESkill1AImplementation::KallariDashSlash && Kallari_Skill1A)
	{
		Kallari_Skill1A->DashStart();
	}
}

void ATP_Character::Skill1A_DashEnd()
{
	if (Skill1A_Implementation == ESkill1AImplementation::TerraDash && Terra_Skill1A)
	{
		Terra_Skill1A->DashEnd();
		return;
	}

	if (Skill1A_Implementation == ESkill1AImplementation::KallariDashSlash && Kallari_Skill1A)
	{
		Kallari_Skill1A->DashEnd();
	}
}

// ===== skill1_B 적용 AOE(광역 공격) (wrapper) =====
void ATP_Character::Skill1B_ApplyAOE()
{
	if (Terra_Skill1B) Terra_Skill1B->ApplyAOE();
}

// ===== skill1_B Kallari 돌진 공격 (wrapper) =====
void ATP_Character::Skill1B_BackflipStart()
{
	if (Skill1B_Implementation == ESkill1BImplementation::KallariBackflip && Kallari_Skill1B)
	{
		Kallari_Skill1B->DashStart();
	}
}

void ATP_Character::Skill1B_BackflipEnd()
{
	if (Skill1B_Implementation == ESkill1BImplementation::KallariBackflip && Kallari_Skill1B)
	{
		Kallari_Skill1B->DashEnd();
	}
}

// ===== skill1_B Gideon 물폭탄 공격 (wrapper) =====
void ATP_Character::Skill1B_ThrowProjectile()
{
	if (Skill1B_Implementation == ESkill1BImplementation::GideonWaterBomb && Gideon_Skill1B)
	{
		Gideon_Skill1B->ThrowProjectile();
	}
}


// -------- 스킬 2 ---------
// input
void ATP_Character::Input_Skill2Started(const FInputActionValue&)
{
	if (Skill2Selected == ESkillVariant::None)
		return;

	const bool bSkill2AReady =
		(Skill2A_Implementation == ESkill2AImplementation::TerraShieldPush && Terra_Skill2A) ||
		(Skill2A_Implementation == ESkill2AImplementation::KallariShurikenTeleport && Kallari_Skill2A) ||
		(Skill2A_Implementation == ESkill2AImplementation::GideonDebuffBall && Gideon_Skill2A);

	const bool bSkill2BReady =
		(Skill2B_Implementation == ESkill2BImplementation::TerraSpin && Terra_Skill2B) ||
		(Skill2B_Implementation == ESkill2BImplementation::KallariShurikenExplosion && Kallari_Skill2B) ||
		(Skill2B_Implementation == ESkill2BImplementation::GideonBackDash && Gideon_Skill2B);

	// 스킬 객체 방어
	if ((Skill2Selected == ESkillVariant::A && !bSkill2AReady) ||
		(Skill2Selected == ESkillVariant::B && !bSkill2BReady))
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();

	// Kallari Skill2_A : 표식이 있으면 두 번째 입력으로 텔레포트 공격
	if (Skill2Selected == ESkillVariant::A
		&& Skill2A_Implementation == ESkill2AImplementation::KallariShurikenTeleport
		&& Kallari_Skill2A
		&& Kallari_Skill2A->HasTeleportMark())
	{
		if (const UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			if (Move->IsFalling())
				return;
		}

		StopJumping();

		if (!CanSkillInput())
			return;

		SetMoveInputEnabled(false);
		SetAttackInputEnabled(false);
		SetGuardInputEnabled(false);
		SetSkillInputEnabled(false);
		SetJumpInputEnabled(false);

		if (!Kallari_Skill2A->TeleportToMarkAndAttack())
		{
			SetEveryInputEnabled(true);
		}
		return;
	}

	// Kallari Skill2_B : 표식이 있으면 두 번째 입력으로 폭발 몽타주
	if (Skill2Selected == ESkillVariant::B
		&& Skill2B_Implementation == ESkill2BImplementation::KallariShurikenExplosion
		&& Kallari_Skill2B
		&& Kallari_Skill2B->HasExplosionMark())
	{
		if (const UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			if (Move->IsFalling())
				return;
		}

		StopJumping();

		if (!CanSkillInput())
			return;

		SetMoveInputEnabled(false);
		SetAttackInputEnabled(false);
		SetGuardInputEnabled(false);
		SetSkillInputEnabled(false);
		SetJumpInputEnabled(false);

		if (!Kallari_Skill2B->PlayExplosionMontage())
		{
			SetEveryInputEnabled(true);
		}
		return;
	}

	// 쿨다운 디버깅 메세지
	if (Skill2Selected == ESkillVariant::A)
	{
		bool bInCooldown = false;

		if (Skill2A_Implementation == ESkill2AImplementation::TerraShieldPush && Terra_Skill2A)
		{
			bInCooldown = Terra_Skill2A->IsInCooldown(Now);
		}
		else if (Skill2A_Implementation == ESkill2AImplementation::KallariShurikenTeleport && Kallari_Skill2A)
		{
			bInCooldown = Kallari_Skill2A->IsInCooldown(Now);
		}
		else if (Skill2A_Implementation == ESkill2AImplementation::GideonDebuffBall && Gideon_Skill2A)
		{
			bInCooldown = Gideon_Skill2A->IsInCooldown(Now);
		}

		if (bInCooldown)
		{
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}
	else if (Skill2Selected == ESkillVariant::B)
	{
		bool bInCooldown = false;

		if (Skill2B_Implementation == ESkill2BImplementation::TerraSpin && Terra_Skill2B)
		{
			bInCooldown = Terra_Skill2B->IsInCooldown(Now);
		}
		else if (Skill2B_Implementation == ESkill2BImplementation::KallariShurikenExplosion && Kallari_Skill2B)
		{
			bInCooldown = Kallari_Skill2B->IsInCooldown(Now);
		}
		else if (Skill2B_Implementation == ESkill2BImplementation::GideonBackDash && Gideon_Skill2B)
		{
			bInCooldown = Gideon_Skill2B->IsInCooldown(Now);
		}

		if (bInCooldown)
		{
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}

	if (const UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->IsFalling())			// 공중 상태일 때 입력 막기를 따로 - 입력을 안받아도 떨어지는 경우 시전 등...
			return;
	}

	StopJumping();

	if (!CanSkillInput())
		return;

	SetMoveInputEnabled(true);
	SetAttackInputEnabled(false);
	SetGuardInputEnabled(false);
	SetSkillInputEnabled(false);
	SetJumpInputEnabled(false);

	UAnimMontage* Montage = nullptr;
	if (Skill2Selected == ESkillVariant::A) Montage = Skill2MontageA;
	else if (Skill2Selected == ESkillVariant::B) Montage = Skill2MontageB;

	if (!Montage) return;

	PlayAnimMontage(Montage);

	if (Skill2Selected == ESkillVariant::A)
	{
		if (Skill2A_Implementation == ESkill2AImplementation::TerraShieldPush && Terra_Skill2A)
		{
			Terra_Skill2A->StartCooldown(Now, Skill2A_Cooldown);
		}
		else if (Skill2A_Implementation == ESkill2AImplementation::KallariShurikenTeleport && Kallari_Skill2A)
		{
			Kallari_Skill2A->StartCooldown(Now, Skill2A_Cooldown);
		}
		else if (Skill2A_Implementation == ESkill2AImplementation::GideonDebuffBall && Gideon_Skill2A)
		{
			Gideon_Skill2A->StartCooldown(Now, Skill2A_Cooldown);
		}
	}
	else if (Skill2Selected == ESkillVariant::B)
	{
		if (Skill2B_Implementation == ESkill2BImplementation::TerraSpin && Terra_Skill2B)
		{
			Terra_Skill2B->BeginSpin(Now, Skill2B_Duration);
			Terra_Skill2B->StartCooldown(Now, Skill2B_Cooldown);
		}
		else if (Skill2B_Implementation == ESkill2BImplementation::KallariShurikenExplosion && Kallari_Skill2B)
		{
			Kallari_Skill2B->StartCooldown(Now, Skill2B_Cooldown);
		}
		else if (Skill2B_Implementation == ESkill2BImplementation::GideonBackDash && Gideon_Skill2B)
		{
			Gideon_Skill2B->StartCooldown(Now, Skill2B_Cooldown);
		}
	}
}

// ===== Skill2_A (wrapper) =====
void ATP_Character::Skill2A_HitStart()
{
	if (Skill2A_Implementation == ESkill2AImplementation::TerraShieldPush && Terra_Skill2A)
	{
		Terra_Skill2A->HitStart();
	}

	// Kallari Skill2_A : 텔레포트 후 단발성 sphere 공격
	if (Skill2A_Implementation == ESkill2AImplementation::KallariShurikenTeleport && Kallari_Skill2A)
	{
		Kallari_Skill2A->HitStart();
	}
}

void ATP_Character::Skill2A_ThrowProjectile()
{
	if (Skill2A_Implementation == ESkill2AImplementation::KallariShurikenTeleport && Kallari_Skill2A)
	{
		Kallari_Skill2A->ThrowProjectile();
		return;
	}

	if (Skill2A_Implementation == ESkill2AImplementation::GideonDebuffBall && Gideon_Skill2A)
	{
		Gideon_Skill2A->ThrowProjectile();
		return;
	}
}

// ===== Skill2_B (wrapper) =====
void ATP_Character::Skill2B_HitStart()
{
	if (Terra_Skill2B) Terra_Skill2B->HitStart();
}

void ATP_Character::Skill2B_SpinEnd()	// 돌기 시간 끝나면 End로
{
	if (Terra_Skill2B) Terra_Skill2B->SpinEnd();
}

// Kallari Skill2_B 1타 : 수리검을 던져 좌표를 남김
void ATP_Character::Skill2B_ThrowProjectile()
{
	if (Skill2B_Implementation == ESkill2BImplementation::KallariShurikenExplosion && Kallari_Skill2B)
	{
		Kallari_Skill2B->ThrowProjectile();
	}
}

// Kallari Skill2_B 2타 : 저장된 좌표에 폭발을 발생시킴
void ATP_Character::Skill2B_ExplodeAtMark()
{
	if (Skill2B_Implementation == ESkill2BImplementation::KallariShurikenExplosion && Kallari_Skill2B)
	{
		Kallari_Skill2B->ExplodeAtMark();
	}
}

// Gideon Skill2_B : 백대쉬 스킬
void ATP_Character::Skill2B_BackDashStart()
{
	if (Skill2B_Implementation == ESkill2BImplementation::GideonBackDash && Gideon_Skill2B)
	{
		Gideon_Skill2B->DashStart();
	}
}

void ATP_Character::Skill2B_BackDashEnd()
{
	if (Skill2B_Implementation == ESkill2BImplementation::GideonBackDash && Gideon_Skill2B)
	{
		Gideon_Skill2B->DashEnd();
	}
}


// -------- 궁극기 ---------
// input
void ATP_Character::Input_UltStarted(const FInputActionValue&)
{
	if (UltSelected == ESkillVariant::None)
		return;

	const bool bUltAReady =
		(UltA_Implementation == EUltimateAImplementation::TerraAllyShield && Terra_UltA) ||
		(UltA_Implementation == EUltimateAImplementation::KallariBlinkDash && Kallari_UltA) ||
		(UltA_Implementation == EUltimateAImplementation::GideonMirrorWaterBeam && Gideon_UltA);

	const bool bUltBReady =
		(UltB_Implementation == EUltimateBImplementation::TerraSelfBuff && Terra_UltB) ||
		(UltB_Implementation == EUltimateBImplementation::KallariInvincible && Kallari_UltB);

	if ((UltSelected == ESkillVariant::A && !bUltAReady) ||
		(UltSelected == ESkillVariant::B && !bUltBReady))
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();

	// 쿨다운 디버깅 메세지
	if (UltSelected == ESkillVariant::A)
	{
		bool bInCooldown = false;

		if (UltA_Implementation == EUltimateAImplementation::TerraAllyShield && Terra_UltA)
		{
			bInCooldown = Terra_UltA->IsInCooldown(Now);
		}
		else if (UltA_Implementation == EUltimateAImplementation::KallariBlinkDash && Kallari_UltA)
		{
			bInCooldown = Kallari_UltA->IsInCooldown(Now);
		}
		else if (UltA_Implementation == EUltimateAImplementation::GideonMirrorWaterBeam && Gideon_UltA)
		{
			bInCooldown = Gideon_UltA->IsInCooldown(Now);
		}

		if (bInCooldown)
		{
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}
	else if (UltSelected == ESkillVariant::B)
	{
		bool bInCooldown = false;

		if (UltB_Implementation == EUltimateBImplementation::TerraSelfBuff && Terra_UltB)
		{
			bInCooldown = Terra_UltB->IsInCooldown(Now);
		}
		else if (UltB_Implementation == EUltimateBImplementation::KallariInvincible && Kallari_UltB)
		{
			bInCooldown = Kallari_UltB->IsInCooldown(Now);
		}

		if (bInCooldown)
		{
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}

	if (const UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->IsFalling())
			return;
	}

	StopJumping();

	if (!CanSkillInput())
		return;

	SetMoveInputEnabled(false);
	SetAttackInputEnabled(false);
	SetGuardInputEnabled(false);
	SetSkillInputEnabled(false);
	SetJumpInputEnabled(false);

	UAnimMontage* Montage = nullptr;
	if (UltSelected == ESkillVariant::A) Montage = UltMontageA;
	else if (UltSelected == ESkillVariant::B) Montage = UltMontageB;

	if (!Montage)
	{
		SetEveryInputEnabled(true);
		return;
	}

	PlayAnimMontage(Montage);

	// 쿨다운 시작 (런타임 상태는 스킬 객체가 관리)
	if (UltSelected == ESkillVariant::A)
	{
		if (UltA_Implementation == EUltimateAImplementation::TerraAllyShield && Terra_UltA)
		{
			Terra_UltA->StartCooldown(Now, UltA_Cooldown);
		}
		else if (UltA_Implementation == EUltimateAImplementation::KallariBlinkDash && Kallari_UltA)
		{
			Kallari_UltA->StartCooldown(Now, UltA_Cooldown);
		}
		else if (UltA_Implementation == EUltimateAImplementation::GideonMirrorWaterBeam && Gideon_UltA)
		{
			Gideon_UltA->StartCooldown(Now, UltA_Cooldown);
		}
	}
	else if (UltSelected == ESkillVariant::B)
	{
		if (UltB_Implementation == EUltimateBImplementation::TerraSelfBuff && Terra_UltB)
		{
			Terra_UltB->StartCooldown(Now, UltB_Cooldown);
		}
		else if (UltB_Implementation == EUltimateBImplementation::KallariInvincible && Kallari_UltB)
		{
			Kallari_UltB->StartCooldown(Now, UltB_Cooldown);
		}
	}
}

// ===== UltA Terra =====
void ATP_Character::UltA_ShieldStart()
{
	if (Terra_UltA) Terra_UltA->ShieldStart();
}

void ATP_Character::UltA_ShieldEnd()
{
	if (Terra_UltA) Terra_UltA->ShieldEnd();
}

// ===== UltA Kallari =====
void ATP_Character::UltA_BlinkHitStart()
{
	if (UltA_Implementation == EUltimateAImplementation::KallariBlinkDash && Kallari_UltA)
	{
		Kallari_UltA->HitStart();
	}
}

void ATP_Character::UltA_BlinkDashStart()
{
	if (UltA_Implementation == EUltimateAImplementation::KallariBlinkDash && Kallari_UltA)
	{
		Kallari_UltA->DashStart();
	}
}

void ATP_Character::UltA_BlinkDashEnd()
{
	if (UltA_Implementation == EUltimateAImplementation::KallariBlinkDash && Kallari_UltA)
	{
		Kallari_UltA->DashEnd();
	}
}

void ATP_Character::UltA_BeamStart()
{
	if (UltA_Implementation == EUltimateAImplementation::GideonMirrorWaterBeam && Gideon_UltA)
	{
		Gideon_UltA->BeamStart();
	}
}

void ATP_Character::UltA_BeamEnd()
{
	if (UltA_Implementation == EUltimateAImplementation::GideonMirrorWaterBeam && Gideon_UltA)
	{
		Gideon_UltA->BeamEnd();
	}
}

// ===== UltB (wrapper) =====
void ATP_Character::UltB_BuffStart()
{
	if (UltB_Implementation == EUltimateBImplementation::TerraSelfBuff && Terra_UltB)
	{
		Terra_UltB->BuffStart();
		return;
	}

	if (UltB_Implementation == EUltimateBImplementation::KallariInvincible && Kallari_UltB)
	{
		Kallari_UltB->BuffStart();
	}
}

void ATP_Character::UltB_BuffEnd()
{
	if (UltB_Implementation == EUltimateBImplementation::TerraSelfBuff && Terra_UltB)
	{
		Terra_UltB->BuffEnd();
		return;
	}

	if (UltB_Implementation == EUltimateBImplementation::KallariInvincible && Kallari_UltB)
	{
		Kallari_UltB->BuffEnd();
	}
}



// Character Settings 
void ATP_Character::ApplyCharacterData(const UCharacterData* Data)
{
	if (!Data) return;

	// Visual
	if (Data->Mesh)
	{
		GetMesh()->SetSkeletalMesh(Data->Mesh);
	}
	if (Data->AnimClass)
	{
		GetMesh()->SetAnimInstanceClass(Data->AnimClass);
	}

	// Movement
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = Data->MaxWalkSpeed;
	}

	// Combat
	if (CombatComp)
	{
		CombatComp->TraceRange = Data->TraceRange;
		CombatComp->Damage = Data->Damage;
	}

	// Health
	if (HealthComp)
	{
		HealthComp->MaxHp = Data->MaxHp;
		HealthComp->ResetHp();
	}

	// Combo Animation Montage
	PrimaryComboMontage = Data->PrimaryComboMontage;

	// 기본 공격
	PrimaryAttackHitType = Data->PrimaryAttackHitType;
	PrimaryAttackSphereRadius = Data->PrimaryAttackSphereRadius;
	PrimaryAttackForwardOffset = Data->PrimaryAttackForwardOffset;
	PrimaryAttackHalfAngleDeg = Data->PrimaryAttackHalfAngleDeg;

	PrimaryProjectileClass = Data->PrimaryProjectileClass;
	PrimaryProjectileSpeed = Data->PrimaryProjectileSpeed;
	PrimaryProjectileLifeSeconds = Data->PrimaryProjectileLifeSeconds;
	PrimaryProjectileRadius = Data->PrimaryProjectileRadius;
	PrimaryProjectileSpawnForwardOffset = Data->PrimaryProjectileSpawnForwardOffset;
	PrimaryProjectileSpawnZOffset = Data->PrimaryProjectileSpawnZOffset;
	PrimaryProjectileSocketA = Data->PrimaryProjectileSocketA;
	PrimaryProjectileSocketB = Data->PrimaryProjectileSocketB;

	// Blocking Animation Montage
	BlockHoldMontage = Data->BlockHoldMontage;

	// 피격
	HitReactMontage = Data->HitReactMontage;
	HitReactPlayRate = Data->HitReactPlayRate;

	// 스킬1
	Skill1Selected = Data->Skill1Selected;		// 인게임 선택 기본값(DA에 넣은 기본값으로 시작)

	Skill1MontageA = Data->Skill1_Montage_A;
	Skill1MontageB = Data->Skill1_Montage_B;

	Skill1A_Implementation = Data->Skill1A_Implementation;
	Skill1B_Implementation = Data->Skill1B_Implementation;

	Skill1A_Damage = Data->Skill1A_Damage;
	Skill1A_DashDistance = Data->Skill1A_DashDistance;
	Skill1A_DashDuration = Data->Skill1A_DashDuration;
	Skill1A_Cooldown = Data->Skill1A_Cooldown;
	Skill1A_HitRadius = Data->Skill1A_HitRadius;

	Skill1A_Range = Data->Skill1A_Range;
	Skill1A_StartSocket = Data->Skill1A_StartSocket;

	Skill1B_Damage = Data->Skill1B_Damage;
	Skill1B_Radius = Data->Skill1B_Radius;
	Skill1B_Cooldown = Data->Skill1B_Cooldown;

	Skill1B_BackflipDuration = Data->Skill1B_BackflipDuration;
	Skill1B_BackwardDistance = Data->Skill1B_BackwardDistance;

	Skill1B_ProjectileClass = Data->Skill1B_ProjectileClass;
	Skill1B_ProjectileForwardSpeed = Data->Skill1B_ProjectileForwardSpeed;
	Skill1B_ProjectileUpwardSpeed = Data->Skill1B_ProjectileUpwardSpeed;
	Skill1B_ProjectileGravityScale = Data->Skill1B_ProjectileGravityScale;
	Skill1B_ProjectileLockOnArcPeakHeight = Data->Skill1B_ProjectileLockOnArcPeakHeight;
	Skill1B_ProjectileLifeSeconds = Data->Skill1B_ProjectileLifeSeconds;
	Skill1B_ProjectileRadius = Data->Skill1B_ProjectileRadius;
	Skill1B_ProjectileSpawnForwardOffset = Data->Skill1B_ProjectileSpawnForwardOffset;
	Skill1B_ProjectileSpawnZOffset = Data->Skill1B_ProjectileSpawnZOffset;
	Skill1B_StartSocket = Data->Skill1B_StartSocket;

	// 스킬2
	Skill2Selected = Data->Skill2Selected;

	Skill2MontageA = Data->Skill2_Montage_A;
	Skill2MontageB = Data->Skill2_Montage_B;

	Skill2A_Implementation = Data->Skill2A_Implementation;
	Skill2B_Implementation = Data->Skill2B_Implementation;

	Skill2A_Damage = Data->Skill2A_Damage;
	Skill2A_Radius = Data->Skill2A_Radius;
	Skill2A_ForwardOffset = Data->Skill2A_ForwardOffset;
	Skill2A_HalfAngleDeg = Data->Skill2A_HalfAngleDeg;
	Skill2A_Cooldown = Data->Skill2A_Cooldown;

	Skill2A_ProjectileClass = Data->Skill2A_ProjectileClass;
	Skill2A_ProjectileSpeed = Data->Skill2A_ProjectileSpeed;
	Skill2A_ProjectileLifeSeconds = Data->Skill2A_ProjectileLifeSeconds;
	Skill2A_ProjectileRadius = Data->Skill2A_ProjectileRadius;
	Skill2A_ProjectileSpawnForwardOffset = Data->Skill2A_ProjectileSpawnForwardOffset;
	Skill2A_ProjectileSpawnZOffset = Data->Skill2A_ProjectileSpawnZOffset;
	Skill2A_ProjectileSpawnSocket = Data->Skill2A_ProjectileSpawnSocket;

	Skill2A_TeleportAttackMontage = Data->Skill2A_TeleportAttackMontage;
	Skill2A_TeleportAttackRadius = Data->Skill2A_TeleportAttackRadius;
	Skill2A_TeleportOffsetFromMark = Data->Skill2A_TeleportOffsetFromMark;

	Skill2A_DebuffDuration = Data->Skill2A_DebuffDuration;
	Skill2A_DebuffIncomingDamageMultiplier = Data->Skill2A_DebuffIncomingDamageMultiplier;

	Skill2B_DamagePerTick = Data->Skill2B_DamagePerTick;
	Skill2B_Radius = Data->Skill2B_Radius;
	Skill2B_TickInterval = Data->Skill2B_TickInterval;
	Skill2B_Duration = Data->Skill2B_Duration;
	Skill2B_Cooldown = Data->Skill2B_Cooldown;

	Skill2B_ProjectileClass = Data->Skill2B_ProjectileClass;
	Skill2B_ProjectileSpeed = Data->Skill2B_ProjectileSpeed;
	Skill2B_ProjectileLifeSeconds = Data->Skill2B_ProjectileLifeSeconds;
	Skill2B_ProjectileRadius = Data->Skill2B_ProjectileRadius;
	Skill2B_ProjectileSpawnForwardOffset = Data->Skill2B_ProjectileSpawnForwardOffset;
	Skill2B_ProjectileSpawnZOffset = Data->Skill2B_ProjectileSpawnZOffset;
	Skill2B_ProjectileSpawnSocket = Data->Skill2B_ProjectileSpawnSocket;

	Skill2B_ExplosionMontage = Data->Skill2B_ExplosionMontage;
	Skill2B_ExplosionDamage = Data->Skill2B_ExplosionDamage;
	Skill2B_ExplosionRadius = Data->Skill2B_ExplosionRadius;

	Skill2B_BackDashDuration = Data->Skill2B_BackDashDuration;
	Skill2B_BackwardDistance = Data->Skill2B_BackwardDistance;

	// 궁극기
	UltSelected = Data->UltSelected;

	UltA_Implementation = Data->UltA_Implementation;
	UltMontageA = Data->Ult_Montage_A;
	UltA_Duration = Data->UltA_Duration;
	UltA_Cooldown = Data->UltA_Cooldown;
	UltA_Shield = Data->UltA_Shield;
	UltA_Radius = Data->UltA_Radius;

	UltA_Damage = Data->UltA_Damage;
	UltA_DashDistance = Data->UltA_DashDistance;
	UltA_DashDuration = Data->UltA_DashDuration;
	UltA_HitRadius = Data->UltA_HitRadius;

	UltA_BeamDamagePerTick = Data->UltA_BeamDamagePerTick;
	UltA_BeamDuration = Data->UltA_BeamDuration;
	UltA_BeamTickInterval = Data->UltA_BeamTickInterval;
	UltA_BeamRange = Data->UltA_BeamRange;
	UltA_BeamRadius = Data->UltA_BeamRadius;
	UltA_BeamStartSocket = Data->UltA_BeamStartSocket;

	UltB_Implementation = Data->UltB_Implementation;
	UltMontageB = Data->Ult_Montage_B;
	UltB_Duration = Data->UltB_Duration;
	UltB_Cooldown = Data->UltB_Cooldown;
	UltB_Shield = Data->UltB_Shield;
	UltB_AttackMultiplier = Data->UltB_AttackMultiplier;

	UltB_InvincibleDuration = Data->UltB_InvincibleDuration;

	// ===== Terra Skill Objects init =====
	// - 캐릭터가 Terra든 Gideon이든, 일단은 Terra 구현이 연결돼 있어도
	//   SkillSelected가 None이면 실행되지 않으니 안전.
	// - 나중에 Gideon 스킬 분리하면 여기에서 캐릭터 타입에 따라 교체하면 됨.
	if (!Terra_Skill1A) { Terra_Skill1A = NewObject<UTerra_Skill1A_Dash>(this); Terra_Skill1A->Init(this); }
	if (!Terra_Skill1B) { Terra_Skill1B = NewObject<UTerra_Skill1B_SlamAOE>(this); Terra_Skill1B->Init(this); }
	if (!Terra_Skill2A) { Terra_Skill2A = NewObject<UTerra_Skill2A_ShieldPush>(this); Terra_Skill2A->Init(this); }
	if (!Terra_Skill2B) { Terra_Skill2B = NewObject<UTerra_Skill2B_Spin>(this); Terra_Skill2B->Init(this); }
	if (!Terra_UltA) { Terra_UltA = NewObject<UTerra_UltA_AllyShield>(this); Terra_UltA->Init(this); }
	if (!Terra_UltB) { Terra_UltB = NewObject<UTerra_UltB_SelfShieldBuff>(this); Terra_UltB->Init(this); }

	if (!Kallari_Skill1A) { Kallari_Skill1A = NewObject<UKallari_Skill1A_DashSlash>(this); Kallari_Skill1A->Init(this); }
	if (!Kallari_Skill1B) { Kallari_Skill1B = NewObject<UKallari_Skill1B_Backflip>(this); Kallari_Skill1B->Init(this); }
	if (!Kallari_Skill2A) { Kallari_Skill2A = NewObject<UKallari_Skill2A_ShurikenTeleport>(this); Kallari_Skill2A->Init(this); }
	if (!Kallari_Skill2B) { Kallari_Skill2B = NewObject<UKallari_Skill2B_ShurikenExplosion>(this); Kallari_Skill2B->Init(this); }
	if (!Kallari_UltA) { Kallari_UltA = NewObject<UKallari_UltA_BlinkDash>(this); Kallari_UltA->Init(this); }
	if (!Kallari_UltB) { Kallari_UltB = NewObject<UKallari_UltB_Invincible>(this); Kallari_UltB->Init(this); }

	if (!Gideon_Skill1A) { Gideon_Skill1A = NewObject<UGideon_Skill1A_WaterCannon>(this); Gideon_Skill1A->Init(this); }
	if (!Gideon_Skill1B) { Gideon_Skill1B = NewObject<UGideon_Skill1B_WaterBomb>(this); Gideon_Skill1B->Init(this); }
	if (!Gideon_Skill2A) { Gideon_Skill2A = NewObject<UGideon_Skill2A_DebuffBall>(this); Gideon_Skill2A->Init(this); }
	if (!Gideon_Skill2B) { Gideon_Skill2B = NewObject<UGideon_Skill2B_BackDash>(this); Gideon_Skill2B->Init(this); }
	if (!Gideon_UltA) { Gideon_UltA = NewObject<UGideon_UltA_MirrorWaterBeam>(this); Gideon_UltA->Init(this); }

	// ===== 런타임 상태 초기화 =====
	// - 캐릭터 교체(슬롯 변경) 시, 이전 캐릭터의 쿨다운/타이머/맵 상태가 남으면 안 됨.
	if (!Terra_Skill1A) Terra_Skill1A->ResetRuntime();
	if (!Terra_Skill1B) Terra_Skill1B->ResetRuntime();
	if (!Terra_Skill2A) Terra_Skill2A->ResetRuntime();
	if (!Terra_Skill2B) Terra_Skill2B->ResetRuntime();
	if (!Terra_UltA) Terra_UltA->ResetRuntime();
	if (!Terra_UltB) Terra_UltB->ResetRuntime();

	if (Kallari_Skill1A) Kallari_Skill1A->ResetRuntime();
	if (Kallari_Skill1B) Kallari_Skill1B->ResetRuntime();
	if (Kallari_Skill2A) Kallari_Skill2A->ResetRuntime();
	if (Kallari_Skill2B) Kallari_Skill2B->ResetRuntime();
	if (Kallari_UltA) Kallari_UltA->ResetRuntime();
	if (Kallari_UltB) Kallari_UltB->ResetRuntime();

	if (Gideon_Skill1A) Gideon_Skill1A->ResetRuntime();
	if (Gideon_Skill1B) Gideon_Skill1B->ResetRuntime();
	if (Gideon_Skill2A) Gideon_Skill2A->ResetRuntime();
	if (Gideon_Skill2B) Gideon_Skill2B->ResetRuntime();
	if (Gideon_UltA) Gideon_UltA->ResetRuntime();
}

// Gideon Skill2A 부조화 구슬 디버프 인터페이스
void ATP_Character::ApplyDebuffBall_Implementation(float InDuration, float InIncomingDamageMultiplier)
{
	UWorld* World = GetWorld();
	if (!World) return;

	DebuffBallEndTime = World->GetTimeSeconds() + InDuration;
	DebuffBallIncomingDamageMultiplier = FMath::Max(InIncomingDamageMultiplier, 1.f);
}

bool ATP_Character::IsDebuffBallActive_Implementation() const
{
	const UWorld* World = GetWorld();
	if (!World) return false;

	return World->GetTimeSeconds() < DebuffBallEndTime;
}

float ATP_Character::GetDebuffBallIncomingDamageMultiplier_Implementation() const
{
	if (IsDebuffBallActive_Implementation())
	{
		return DebuffBallIncomingDamageMultiplier;
	}

	return 1.f;
}

// Character Select
void ATP_Character::SelectCharacterSlot(int32 Index)
{
	if (!CharacterSlots.IsValidIndex(Index) || !CharacterSlots[Index])
	{
		UE_LOG(LogTemp, Warning, TEXT("[CharSwitch] Invalid slot %d"), Index + 1);
		return;
	}

	ApplyCharacterData(CharacterSlots[Index]);
	CurrentSlotIndex = Index;

	UE_LOG(LogTemp, Warning, TEXT("[CharSwitch] Switched to slot %d (%s)"),
		Index + 1,
		*CharacterSlots[Index]->GetName());
}

void ATP_Character::SelectSlot1() { SelectCharacterSlot(0); }
void ATP_Character::SelectSlot2() { SelectCharacterSlot(1); }
void ATP_Character::SelectSlot3() { SelectCharacterSlot(2); }
void ATP_Character::SelectSlot4() { SelectCharacterSlot(3); }
void ATP_Character::SelectSlot5() { SelectCharacterSlot(4); }