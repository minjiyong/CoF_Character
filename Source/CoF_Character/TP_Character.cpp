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

	if (AttackAction)
	{
		EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ATP_Character::Input_AttackStarted);
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
	Jump();
}

void ATP_Character::Input_JumpCompleted(const FInputActionValue& Value)
{
	StopJumping();
}

void ATP_Character::Input_AttackStarted(const FInputActionValue& Value)
{
	bAttackPressed = true;

	// 콤보가 아예 안 돌고 있으면 첫타 시작
	if (!PrimaryComboMontage) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	if (!Anim->Montage_IsPlaying(PrimaryComboMontage))
	{
		ComboIndex = 0;
		bComboQueued = false;

		PlayAnimMontage(PrimaryComboMontage, 1.f, FName(TEXT("A")));
		bAttackPressed = false; // 첫타는 지금 소비
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



// 콤보 동작 - 다음으로 넘기기
void ATP_Character::ComboWindowOpen()
{
	bComboWindowOpen = true;
}

void ATP_Character::ComboWindowClose()
{
	bComboWindowOpen = false;

	// 예약된 입력이 있으면 A->B로 넘어가게 세팅
	if (!bComboQueued || !PrimaryComboMontage)
	{
		bComboQueued = false;
		return;
	}

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim)
	{
		bComboQueued = false;
		return;
	}

	// A(0) -> B(1)
	if (ComboIndex == 0)
	{
		Anim->Montage_SetNextSection(FName(TEXT("A")), FName(TEXT("B")), PrimaryComboMontage);
		ComboIndex = 1;
	}

	bComboQueued = false;
}


// 이 타이밍에 입력이 들어왔으면 다음타 예약
void ATP_Character::SaveAttack()
{
	if (!bAttackPressed) return;

	bAttackPressed = false;
	bComboQueued = true;

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

void ATP_Character::ResetCombo()
{
	// 예약이 없으면 콤보 끊기
	if (!bComboQueued)
	{
		ComboIndex = 0;
	}
	bComboQueued = false;
	bAttackPressed = false;
}

void ATP_Character::HitStart()
{
	// 이번 타 시작: 1회 히트 가능 상태로 초기화
	if (CombatComp)
	{
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