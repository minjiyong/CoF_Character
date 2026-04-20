#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "SkillTypes.h"
#include "HitReactInterface.h"
#include "TimerManager.h"

#include "TP_Character.generated.h"

class UCameraComponent;
class USpringArmComponent;

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class UCharacterData;
class UHealthComponent;
class UCombatComponent;

// Animation
class UAnimMontage;

UCLASS()
class COF_CHARACTER_API ATP_Character : public ACharacter, public IHitReactInterface
{
	GENERATED_BODY()

	// ===== Terra Skills (logic separated) =====
	friend class UTerra_Skill1A_Dash;
	friend class UTerra_Skill1B_SlamAOE;
	friend class UTerra_Skill2A_ShieldPush;
	friend class UTerra_Skill2B_Spin;
	friend class UTerra_UltA_AllyShield;
	friend class UTerra_UltB_SelfShieldBuff;

	// ===== Kallari Skills =====
	friend class AKallari_Skill2A_ShurikenProjectile;
	friend class UKallari_Skill2A_ShurikenTeleport;

public:
	ATP_Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Camera (템플릿 구조) =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	// ===== HealthComponent =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

	// ===== CombatComponent =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCombatComponent* CombatComp;

	// ===== Enhanced Input Assets =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;			// 기본 공격(좌클릭)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* BlockAction;			// 우클릭 방패 들기

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* Skill1Action;			// 스킬1

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* Skill2Action;			// 스킬2

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* UltAction;			// 궁극기


	// ===== Character Data =====
	UFUNCTION(BlueprintCallable, Category = "Character")
	void ApplyCharacterData(const UCharacterData* Data);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UCharacterData> DefaultCharacterData;


	// ===== Input callbacks =====
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_JumpStarted(const FInputActionValue& Value);
	void Input_JumpCompleted(const FInputActionValue& Value);

	void Input_AttackStarted(const FInputActionValue& Value);		// 기본 공격(좌클릭)

	void Input_BlockStarted(const FInputActionValue& Value);		// 우클릭 방패 들기
	void Input_BlockCompleted(const FInputActionValue& Value);

	void Input_Skill1Started(const FInputActionValue& Value);		// 스킬1
	void Input_Skill2Started(const FInputActionValue& Value);		// 스킬2
	void Input_UltStarted(const FInputActionValue& Value);			// 궁극기


	// ===== Animation =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> PrimaryComboMontage = nullptr;			// 기본 좌클릭 콤보 공격

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> BlockHoldMontage = nullptr;			// 우클릭 방패 들기

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|HitReact")
	TObjectPtr<UAnimMontage> HitReactMontage = nullptr;				// 피격 애니메이션


	// ======= 입력 잠금 =======
	bool bCanMoveInput = true;
	bool bCanAttackInput = true;
	bool bCanGuardInput = true;
	bool bCanSkillInput = true;
	bool bCanJumpInput = true;

	bool bJumpAccepted = false;		// 점프 중인지 판단

	bool CanMoveInput() const;
	bool CanAttackInput() const;
	bool CanGuardInput() const;
	bool CanSkillInput() const;
	bool CanJumpInput() const;

	void SetMoveInputEnabled(bool bEnable);
	void SetAttackInputEnabled(bool bEnable);
	void SetGuardInputEnabled(bool bEnable);
	void SetSkillInputEnabled(bool bEnable);
	void SetJumpInputEnabled(bool bEnable);
	void SetEveryInputEnabled(bool bEnable);

	// 피격
	float HitReactPlayRate = 1.0f;

	// HitReactInterface 구현
	virtual void OnHitReact_Implementation(float DamageAmount, const FVector& HitPoint, const FVector& HitNormal) override;
	void Debug_ForceHit();			// (더미 없이 테스트용) 강제 피격


	// 기본 공격
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	EPrimaryAttackHitType PrimaryAttackHitType = EPrimaryAttackHitType::LineTrace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float PrimaryAttackSphereRadius = 120.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float PrimaryAttackForwardOffset = 120.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float PrimaryAttackHalfAngleDeg = 60.f;

	// 콤보 상태
	bool bComboWindowOpen = false;
	bool bComboQueued = false;

	bool bAttackPressed = false;		// 콤보를 받는 타이밍(SaveAttack 이후) 에 버튼이 눌렸는가

	// 현재 콤보 단계(0=A, 1=B)
	int32 ComboIndex = 0;

	static constexpr const TCHAR* ComboSections[2] = { TEXT("A"), TEXT("B") };

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void ComboWindowOpen();

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void ComboWindowClose();

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void SaveAttack();   // SaveAttack notify에서 호출

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void ResetCombo();        // ResetCombo notify에서 호출

	// 공격이 실제로 닿는 순간
	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	void HitStart();

	// 기본공격은 단발 공격이라 사용X 일단 만들어둠
	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	void HitEnd();


	// 우클릭 방패 들기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Defense")
	bool bBlocking = false;


	// ===== Terra Skill Logic Objects =====
	// - AnimNotify / BP가 기존에 ATP_Character의 UFUNCTION들을 직접 호출하고 있으므로,
	//   UFUNCTION 시그니처는 그대로 유지하고, 내부 구현만 Terra_* 객체로 위임한다.
	UPROPERTY()
	TObjectPtr<UTerra_Skill1A_Dash> Terra_Skill1A = nullptr;

	UPROPERTY()
	TObjectPtr<UTerra_Skill1B_SlamAOE> Terra_Skill1B = nullptr;

	UPROPERTY()
	TObjectPtr<UTerra_Skill2A_ShieldPush> Terra_Skill2A = nullptr;

	UPROPERTY()
	TObjectPtr<UTerra_Skill2B_Spin> Terra_Skill2B = nullptr;

	UPROPERTY()
	TObjectPtr<UTerra_UltA_AllyShield> Terra_UltA = nullptr;

	UPROPERTY()
	TObjectPtr<UTerra_UltB_SelfShieldBuff> Terra_UltB = nullptr;

	UPROPERTY()
	TObjectPtr<UKallari_Skill2A_ShurikenTeleport> Kallari_Skill2A = nullptr;


	// 스킬1 뭘 선택했는지
	ESkillVariant Skill1Selected = ESkillVariant::None;

	// 스킬1_A 돌진 (UFUNCTION은 유지, 내부 구현은 Terra_Skill1A로 위임)
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|A")
	void Skill1A_DashStart();			// 실제 돌진 시, 상태를 fly로 만듬(바닥 충돌 때문에)

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|A")
	void Skill1A_DashEnd();

	UFUNCTION(BlueprintCallable)
	void Skill1A_HitStart();			// 실제 히트 호출

	UFUNCTION(BlueprintCallable)
	void Skill1A_HitEnd();

	TObjectPtr<UAnimMontage> Skill1MontageA = nullptr;
	float Skill1A_Damage = 0.f;
	float Skill1A_DashDistance = 0.f;
	float Skill1A_DashDuration = 0.f;			// 몇 초 동안 밀고 갈지
	float Skill1A_Cooldown = 0.f;


	// 스킬1_B 도끼찍기
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1")
	void Skill1B_ApplyAOE();

	TObjectPtr<UAnimMontage> Skill1MontageB = nullptr;
	float Skill1B_Damage = 0.f;
	float Skill1B_Radius = 0.f;
	float Skill1B_Cooldown = 0.f;


	// 스킬 2
	ESkillVariant Skill2Selected = ESkillVariant::None;

	// 스킬 2_A 방패 밀쳐내기 전방 광역 공격 / Kallari 수리검 순간이동
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|A")
	void Skill2A_HitStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|A")
	void Skill2A_ThrowProjectile();

	TObjectPtr<UAnimMontage> Skill2MontageA = nullptr;

	ESkill2AImplementation Skill2A_Implementation = ESkill2AImplementation::TerraShieldPush;

	float Skill2A_Damage = 0.f;
	float Skill2A_Radius = 0.f;
	float Skill2A_ForwardOffset = 0.f;
	float Skill2A_HalfAngleDeg = 0.f;
	float Skill2A_Cooldown = 0.f;

	TSubclassOf<AKallari_Skill2A_ShurikenProjectile> Skill2A_ProjectileClass = nullptr;
	float Skill2A_ProjectileSpeed = 0.f;
	float Skill2A_ProjectileLifeSeconds = 0.f;
	float Skill2A_ProjectileRadius = 0.f;
	float Skill2A_ProjectileSpawnForwardOffset = 0.f;
	float Skill2A_ProjectileSpawnZOffset = 0.f;
	FName Skill2A_ProjectileSpawnSocket = NAME_None;


	// 스킬 2_B 돌기
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|B")
	void Skill2B_HitStart();
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|B")
	void Skill2B_SpinEnd();

	TObjectPtr<UAnimMontage> Skill2MontageB = nullptr;
	float Skill2B_DamagePerTick = 0.f;
	float Skill2B_Radius = 0.f;
	float Skill2B_TickInterval = 0.f;
	float Skill2B_Duration = 0.f;
	float Skill2B_Cooldown = 0.f;


	// 궁극기
	ESkillVariant UltSelected = ESkillVariant::None;

	// 궁_A - 아군 쉴드
	TObjectPtr<UAnimMontage> UltMontageA = nullptr;

	float UltA_Duration = 0.f;
	float UltA_Cooldown = 0.f;
	float UltA_Shield = 0.f;
	float UltA_Radius = 0.f;

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_ShieldStart();   // 몽타주 Notify에서 호출 (부여)

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_ShieldEnd();     // 타이머에서 자동 호출 (제거)

	// 궁_B - 자체 쉴드, 공격력 증가
	TObjectPtr<UAnimMontage> UltMontageB = nullptr;

	float UltB_Duration = 0.f;
	float UltB_Cooldown = 0.f;
	float UltB_Shield = 0.f;
	float UltB_AttackMultiplier = 1.f;

	float AttackMultiplier = 1.0f;		// 공격력 배율(전체 공격에 적용)

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|B")
	void UltB_BuffStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|B")
	void UltB_BuffEnd();


	// ===== 캐릭터 선택(런타임 교체) =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Switch")
	TArray<TObjectPtr<UCharacterData>> CharacterSlots; // 0~4 => 1~5키

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Switch")
	int32 CurrentSlotIndex = -1;

	void SelectCharacterSlot(int32 Index);

	void SelectSlot1();
	void SelectSlot2();
	void SelectSlot3();
	void SelectSlot4();
	void SelectSlot5();
};