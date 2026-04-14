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

// ===== Terra Skills (logic separated) =====
#include "Skills/Terra/Terra_Skill1A_Dash.h"
#include "Skills/Terra/Terra_Skill1B_SlamAOE.h"
#include "Skills/Terra/Terra_Skill2A_ShieldPush.h"
#include "Skills/Terra/Terra_Skill2B_Spin.h"
#include "Skills/Terra/Terra_UltA_AllyShield.h"
#include "Skills/Terra/Terra_UltB_SelfShieldBuff.h"

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

	// ===== 캐릭터 데이터는 항상 먼저 적용 =====
	if (CharacterSlots.Num() > 0 && CharacterSlots[0])
	{
		SelectCharacterSlot(0);
	}
	else if (DefaultCharacterData)
	{
		ApplyCharacterData(DefaultCharacterData);
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
	// 1) 데미지 적용
	if (HealthComp)
	{
		HealthComp->ApplyDamage_Local(DamageAmount);
	}

	// 2) 피격 애니 재생
	if (!HitReactMontage) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	// (선택) 이미 피격 몽타주가 돌고 있으면 재시작하지 않게
	if (Anim->Montage_IsPlaying(HitReactMontage))
		return;

	// 추후 방향에 따라 섹션 선택(Front/Back/Left/Right) 으로 교체할지 생각해봐야함.
	// 일단 단일 섹션이면 그냥 Play
	PlayAnimMontage(HitReactMontage, HitReactPlayRate);
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
	if (!Terra_Skill1A || !Terra_Skill1B)
		return;

	// 쿨다운 디버깅 메세지
	const double Now = GetWorld()->GetTimeSeconds();

	if (Skill1Selected == ESkillVariant::A) {
		if (Terra_Skill1A->IsInCooldown(Now)) {
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}
	else if (Skill1Selected == ESkillVariant::B) {
		if (Terra_Skill1B->IsInCooldown(Now)) {
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
	if (Skill1Selected == ESkillVariant::A) Terra_Skill1A->StartCooldown(Now, Skill1A_Cooldown);
	else if (Skill1Selected == ESkillVariant::B) Terra_Skill1B->StartCooldown(Now, Skill1B_Cooldown);
}

// ===== Skill1_A 돌진 (wrapper) =====
void ATP_Character::Skill1A_HitStart()
{
	if (Terra_Skill1A) Terra_Skill1A->HitStart();
}

void ATP_Character::Skill1A_HitEnd()		//역시나 당장은 필요없는듯 기존 hitend 돌려쓰는중 나중에 필요하면 바꾸자.
{
	if (Terra_Skill1A) Terra_Skill1A->HitEnd();
}

void ATP_Character::Skill1A_DashStart()
{
	if (Terra_Skill1A) Terra_Skill1A->DashStart();
}

void ATP_Character::Skill1A_DashEnd()
{
	if (Terra_Skill1A) Terra_Skill1A->DashEnd();
}

// ===== skill1_B 적용 AOE(광역 공격) (wrapper) =====
void ATP_Character::Skill1B_ApplyAOE()
{
	if (Terra_Skill1B) Terra_Skill1B->ApplyAOE();
}

// -------- 스킬 2 ---------
// input
void ATP_Character::Input_Skill2Started(const FInputActionValue&)
{
	if (Skill2Selected == ESkillVariant::None)
		return;

	// 스킬 객체 방어
	if (!Terra_Skill2A || !Terra_Skill2B)
		return;

	// 쿨다운 디버깅 메세지
	const double Now = GetWorld()->GetTimeSeconds();

	if (Skill2Selected == ESkillVariant::A) {
		if (Terra_Skill2A->IsInCooldown(Now)) {
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}
	else if (Skill2Selected == ESkillVariant::B) {
		if (Terra_Skill2B->IsInCooldown(Now)) {
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
		Terra_Skill2A->StartCooldown(Now, Skill2A_Cooldown);
	}
	else if (Skill2Selected == ESkillVariant::B)
	{
		// Spin 런타임(EndTime/TimerHandle/Active)은 스킬 객체가 관리
		Terra_Skill2B->BeginSpin(Now, Skill2B_Duration);
		Terra_Skill2B->StartCooldown(Now, Skill2B_Cooldown);
	}
}

// ===== Skill2_A (wrapper) =====
void ATP_Character::Skill2A_HitStart()
{
	if (Terra_Skill2A) Terra_Skill2A->HitStart();
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

// -------- 궁극기 ---------
// input
void ATP_Character::Input_UltStarted(const FInputActionValue&)
{
	if (UltSelected == ESkillVariant::None) return;

	if (!Terra_UltA || !Terra_UltB)
		return;

	// 쿨다운 디버깅 메세지
	const double Now = GetWorld()->GetTimeSeconds();

	if (UltSelected == ESkillVariant::A) {
		if (Terra_UltA->IsInCooldown(Now)) {
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}
	else if (UltSelected == ESkillVariant::B) {
		if (Terra_UltB->IsInCooldown(Now)) {
			ScreenDbg(TEXT("Notify: in cooldown"), 1.5f, FColor::Red);
			return;
		}
	}

	if (const UCharacterMovementComponent* Move = GetCharacterMovement())
		if (Move->IsFalling())
			return;

	if (!CanSkillInput()) return;

	// 캐스팅 동안 입력 잠금
	SetMoveInputEnabled(false);
	SetAttackInputEnabled(false);
	SetGuardInputEnabled(false);
	SetSkillInputEnabled(false);
	SetJumpInputEnabled(false);

	UAnimMontage* Montage = nullptr;
	if (UltSelected == ESkillVariant::A) Montage = UltMontageA;
	else if (UltSelected == ESkillVariant::B) Montage = UltMontageB;

	if (!Montage) return;

	PlayAnimMontage(Montage);

	// 쿨다운 시작 (런타임 상태는 스킬 객체가 관리)
	if (UltSelected == ESkillVariant::A) Terra_UltA->StartCooldown(Now, UltA_Cooldown);
	else if (UltSelected == ESkillVariant::B) Terra_UltB->StartCooldown(Now, UltB_Cooldown);
}

// ===== UltA (wrapper) =====
void ATP_Character::UltA_ShieldStart()
{
	if (Terra_UltA) Terra_UltA->ShieldStart();
}

void ATP_Character::UltA_ShieldEnd()
{
	if (Terra_UltA) Terra_UltA->ShieldEnd();
}

// ===== UltB (wrapper) =====
void ATP_Character::UltB_BuffStart()
{
	if (Terra_UltB) Terra_UltB->BuffStart();
}

void ATP_Character::UltB_BuffEnd()
{
	if (Terra_UltB) Terra_UltB->BuffEnd();
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

	// Blocking Animation Montage
	BlockHoldMontage = Data->BlockHoldMontage;

	// 피격
	HitReactMontage = Data->HitReactMontage;
	HitReactPlayRate = Data->HitReactPlayRate;

	// 스킬1
	Skill1Selected = Data->Skill1Selected;		// 인게임 선택 기본값(DA에 넣은 기본값으로 시작)

	Skill1MontageA = Data->Skill1_Montage_A;
	Skill1MontageB = Data->Skill1_Montage_B;

	Skill1A_Damage = Data->Skill1A_Damage;
	Skill1A_DashDistance = Data->Skill1A_DashDistance;
	Skill1A_DashDuration = Data->Skill1A_DashDuration;
	Skill1A_Cooldown = Data->Skill1A_Cooldown;

	Skill1B_Damage = Data->Skill1B_Damage;
	Skill1B_Radius = Data->Skill1B_Radius;
	Skill1B_Cooldown = Data->Skill1B_Cooldown;

	// 스킬2
	Skill2Selected = Data->Skill2Selected;

	Skill2MontageA = Data->Skill2_Montage_A;
	Skill2MontageB = Data->Skill2_Montage_B;

	Skill2A_Damage = Data->Skill2A_Damage;
	Skill2A_Radius = Data->Skill2A_Radius;
	Skill2A_ForwardOffset = Data->Skill2A_ForwardOffset;
	Skill2A_HalfAngleDeg = Data->Skill2A_HalfAngleDeg;
	Skill2A_Cooldown = Data->Skill2A_Cooldown;

	Skill2B_DamagePerTick = Data->Skill2B_DamagePerTick;
	Skill2B_Radius = Data->Skill2B_Radius;
	Skill2B_TickInterval = Data->Skill2B_TickInterval;
	Skill2B_Duration = Data->Skill2B_Duration;
	Skill2B_Cooldown = Data->Skill2B_Cooldown;

	// 궁극기
	UltSelected = Data->UltSelected;

	UltMontageA = Data->Ult_Montage_A;
	UltMontageB = Data->Ult_Montage_B;

	UltA_Duration = Data->UltA_Duration;
	UltA_Cooldown = Data->UltA_Cooldown;
	UltA_Shield = Data->UltA_Shield;
	UltA_Radius = Data->UltA_Radius;

	UltB_Duration = Data->UltB_Duration;
	UltB_Cooldown = Data->UltB_Cooldown;
	UltB_Shield = Data->UltB_Shield;
	UltB_AttackMultiplier = Data->UltB_AttackMultiplier;

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

	// ===== 런타임 상태 초기화 =====
	// - 캐릭터 교체(슬롯 변경) 시, 이전 캐릭터의 쿨다운/타이머/맵 상태가 남으면 안 됨.
	Terra_Skill1A->ResetRuntime();
	Terra_Skill1B->ResetRuntime();
	Terra_Skill2A->ResetRuntime();
	Terra_Skill2B->ResetRuntime();
	Terra_UltA->ResetRuntime();
	Terra_UltB->ResetRuntime();
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