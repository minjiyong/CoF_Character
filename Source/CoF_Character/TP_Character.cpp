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

// Character Select
#include "InputCoreTypes.h"
#include "CharacterData.h"

// Animation
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"



//312312312 
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

	// 로컬 플레이어(내 화면)에서만 MappingContext 추가
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsys) return;

	// 디폴트 IMC(IA) 적용
	if (DefaultMappingContext)
	{
		Subsys->AddMappingContext(DefaultMappingContext, 0);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Input] DefaultMappingContext is null. Set it in BP child."));
	}

	// 시작 시 0번 캐릭터가 있다면 select
	if (CharacterSlots.Num() > 0 && CharacterSlots[0])
	{
		SelectCharacterSlot(0);
		return;
	}

	// 디폴트 캐릭터 데이터 적용
	if (DefaultCharacterData)
	{
		ApplyCharacterData(DefaultCharacterData);
	}
}

// 입력 잠금
bool ATP_Character::CanMoveInput() const
{
	return bCanMoveInput;
}

bool ATP_Character::CanAttackInput() const
{
	return bCanAttackInput;
}

bool ATP_Character::CanGuardInput() const
{
	return bCanGuardInput;
}

bool ATP_Character::CanSkillInput() const
{
	return bCanSkillInput;
}

bool ATP_Character::CanJumpInput() const
{
	return bCanJumpInput;
}

void ATP_Character::SetMoveInputEnabled(bool bEnable)
{
	bCanMoveInput = bEnable;
}

void ATP_Character::SetAttackInputEnabled(bool bEnable)
{
	bCanAttackInput = bEnable;
}

void ATP_Character::SetGuardInputEnabled(bool bEnable)
{
	bCanGuardInput = bEnable;
}

void ATP_Character::SetSkillInputEnabled(bool bEnable)
{
	bCanSkillInput = bEnable;
}

void ATP_Character::SetJumpInputEnabled(bool bEnable)
{
	bCanJumpInput = bEnable;
}

void ATP_Character::SetEveryInputEnabled(bool bEnable)
{
	SetMoveInputEnabled(bEnable);
	SetAttackInputEnabled(bEnable);
	SetGuardInputEnabled(bEnable);
	SetSkillInputEnabled(bEnable);
	SetJumpInputEnabled(bEnable);
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

	// 1~5 키로 캐릭터 교체
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ATP_Character::SelectSlot1);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ATP_Character::SelectSlot2);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ATP_Character::SelectSlot3);
	PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ATP_Character::SelectSlot4);
	PlayerInputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ATP_Character::SelectSlot5);
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
	SetJumpInputEnabled(true);

	Jump();
}

void ATP_Character::Input_JumpCompleted(const FInputActionValue& Value)
{
	StopJumping();

	SetEveryInputEnabled(true);
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

	SetEveryInputEnabled(true);
}

// Hit 판정할 영역
void ATP_Character::HitStart()
{
	// 이번 타 시작: 1회 히트 가능 상태로 초기화
	if (CombatComp)
	{
		CombatComp->ConfigureTraceHit(CombatComp->Damage, CombatComp->TraceRange);
		CombatComp->BeginHitWindow_OneShot();
	}
}

void ATP_Character::HitEnd()
{
	if (CombatComp)
	{
		CombatComp->EndHitWindow();
	}
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


// -------- 스킬 1 ---------
// input
void ATP_Character::Input_Skill1Started(const FInputActionValue&)
{
	if (Skill1Selected == ESkillVariant::None)
		return;

	// 쿨다운 디버깅 메세지
	const double Now = GetWorld()->GetTimeSeconds();
	if (Skill1Selected == ESkillVariant::A) {
		if (Now < Skill1A_NextAvailableTime) {
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}
	else if (Skill1Selected == ESkillVariant::B) {
		if (Now < Skill1B_NextAvailableTime) {
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}

	if (const UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->IsFalling())			// 공중 상태일 때 막기를 따로 - 입력을 안받아도 떨어지는 경우 시전 등...
			return;
	}

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

	if (Skill1Selected == ESkillVariant::A) Skill1A_NextAvailableTime = Now + Skill1A_Cooldown;
	else if (Skill1Selected == ESkillVariant::B) Skill1B_NextAvailableTime = Now + Skill1B_Cooldown;
}


// Skill1_A 돌진
void ATP_Character::Skill1A_HitStart()
{
	if (!CombatComp)
		return;

	CombatComp->ConfigureDashHit(Skill1A_Damage, Skill1A_TraceRange, Skill1A_DashDuration);

	CombatComp->BeginHitWindow_OneShot();
}

void ATP_Character::Skill1A_HitEnd()		//역시나 당장은 필요없는듯 기존 hitend 돌려쓰는중 나중에 필요하면 바꾸자.
{
	if (!CombatComp)
		return;

	CombatComp->EndHitWindow();
}

void ATP_Character::Skill1A_DashStart()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp || bSkillDashMoving)
		return;

	bSkillDashMoving = true;

	SavedGroundFriction = MoveComp->GroundFriction;
	SavedBrakingFrictionFactor = MoveComp->BrakingFrictionFactor;
	SavedBrakingDecelerationWalking = MoveComp->BrakingDecelerationWalking;
	SavedBrakingDecelerationFlying = MoveComp->BrakingDecelerationFlying;

	bSavedOrientRotationToMovement = MoveComp->bOrientRotationToMovement;
	bSavedUseControllerRotationYaw = bUseControllerRotationYaw;

	MoveComp->GroundFriction = 0.f;
	MoveComp->BrakingFrictionFactor = 0.f;
	MoveComp->BrakingDecelerationWalking = 0.f;
	MoveComp->BrakingDecelerationFlying = 0.f;

	MoveComp->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	MoveComp->SetMovementMode(MOVE_Flying);

	const FVector Dir = GetActorForwardVector().GetSafeNormal2D();
	const float Speed = (Skill1A_DashDuration > 0.f)
		? (Skill1A_DashDistance / Skill1A_DashDuration)
		: 0.f;

	MoveComp->Velocity = Dir * Speed;
}

void ATP_Character::Skill1A_DashEnd()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp || !bSkillDashMoving)
		return;

	bSkillDashMoving = false;

	MoveComp->StopMovementImmediately();
	MoveComp->SetMovementMode(MOVE_Walking);

	MoveComp->GroundFriction = SavedGroundFriction;
	MoveComp->BrakingFrictionFactor = SavedBrakingFrictionFactor;
	MoveComp->BrakingDecelerationWalking = SavedBrakingDecelerationWalking;
	MoveComp->BrakingDecelerationFlying = SavedBrakingDecelerationFlying;

	MoveComp->bOrientRotationToMovement = bSavedOrientRotationToMovement;
	bUseControllerRotationYaw = bSavedUseControllerRotationYaw;
}


// skill1_B 적용 AOE(광역 공격)
void ATP_Character::Skill1B_ApplyAOE()
{
	if (!CombatComp) return;

	// 여기서 스킬1 내려찍기(검) 의 광역 판정 1회 실행
	CombatComp->ConfigureAOEHit(Skill1B_Damage, Skill1B_Radius);
	CombatComp->BeginHitWindow_OneShot();
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

	// Blocking Animation Montage
	BlockHoldMontage = Data->BlockHoldMontage;

	// 스킬1
	Skill1Selected = Data->Skill1Selected;		// 인게임 선택 기본값(DA에 넣은 기본값으로 시작)

	Skill1MontageA = Data->Skill1_Montage_A;
	Skill1MontageB = Data->Skill1_Montage_B;

	Skill1A_Damage = Data->Skill1A_Damage;
	Skill1A_DashDistance = Data->Skill1A_DashDistance;
	Skill1A_DashDuration = Data->Skill1A_DashDuration;
	Skill1A_TraceRange = Data->Skill1A_TraceRange;
	Skill1A_Cooldown = Data->Skill1A_Cooldown;
	Skill1A_NextAvailableTime = 0.0;

	Skill1B_Damage = Data->Skill1B_Damage;
	Skill1B_Radius = Data->Skill1B_Radius;
	Skill1B_Cooldown = Data->Skill1B_Cooldown;
	Skill1B_NextAvailableTime = 0.0;
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

