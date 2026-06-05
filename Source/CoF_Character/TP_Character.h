#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "SkillTypes.h"
#include "HitReactInterface.h"
#include "Interfaces/DebuffBallTargetInterface.h"
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

// Camera
class UUserWidget;

UCLASS()
class COF_CHARACTER_API ATP_Character : public ACharacter, public IHitReactInterface, public IDebuffBallTargetInterface
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
	friend class UKallari_Skill1A_DashSlash;
	friend class UKallari_Skill1B_Backflip;
	friend class UKallari_Skill2A_ShurikenTeleport;
	friend class UKallari_Skill2B_ShurikenExplosion;
	friend class UKallari_UltA_BlinkDash;
	friend class UKallari_UltB_Invincible;

	// ===== Gideon Skills =====
	friend class UGideon_Skill1A_WaterCannon;
	friend class UGideon_Skill1B_WaterBomb;
	friend class UGideon_Skill2A_DebuffBall;
	friend class UGideon_Skill2B_BackDash;
	friend class UGideon_UltA_MirrorWaterBeam;
	friend class UGideon_UltB_WaterBombDrop;
	friend class AGideon_UltB_WaterBombActor;

	friend class ACoF_CommonProjectile;

public:
	ATP_Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

	// ===== Camera (템플릿 구조) =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	// ===== Core Components =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LockOnAction; // 락온(좌 컨트롤)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction; // 기본 공격(좌클릭)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* BlockAction; // 우클릭 방패 들기

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* Skill1Action; // 스킬1

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* Skill2Action; // 스킬2

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* UltAction; // 궁극기

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

	void Input_LockOnToggle(const FInputActionValue& Value);

	void Input_AttackStarted(const FInputActionValue& Value); // 기본 공격(좌클릭)
	void Input_BlockStarted(const FInputActionValue& Value);  // 우클릭 방패 들기
	void Input_BlockCompleted(const FInputActionValue& Value);

	void Input_Skill1Started(const FInputActionValue& Value); // 스킬1
	void Input_Skill2Started(const FInputActionValue& Value); // 스킬2
	void Input_UltStarted(const FInputActionValue& Value);    // 궁극기

	// ===== Animation =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> PrimaryComboMontage = nullptr; // 기본 좌클릭 콤보 공격

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> BlockHoldMontage = nullptr; // 우클릭 방패 들기

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|HitReact")
	TObjectPtr<UAnimMontage> HitReactMontage = nullptr; // 피격 애니메이션

	// ======= 입력 잠금 =======
	bool bCanMoveInput = true;
	bool bCanAttackInput = true;
	bool bCanGuardInput = true;
	bool bCanSkillInput = true;
	bool bCanJumpInput = true;

	bool bJumpAccepted = false; // 점프 중인지 판단

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

	// ===== 락온 시스템 ======
	bool HasValidLockOnTarget() const;
	void RefreshBossLockOnTarget();
	void ClearLockOn();
	void UpdateLockOnRotation(float DeltaSeconds);
	void UpdateLockOnCamera(float DeltaSeconds); // 락온 중 카메라 보정
	void EnsureLockOnWidget();                   // 락온 UI 생성
	void UpdateLockOnWidget();                   // 락온 UI 위치 갱신

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	AActor* GetLockOnTarget() const { return LockOnTarget; }

	// 피격
	float HitReactPlayRate = 1.0f;

	// HitReactInterface 구현
	virtual void OnHitReact_Implementation(float DamageAmount, const FVector& HitPoint, const FVector& HitNormal) override;

	void Debug_ForceHit(); // (더미 없이 테스트용) 강제 피격

	// 피격 중 입력 잠금 / 무적 처리
	bool bHitReacting = false;
	bool bCanBeHit = true;
	bool bHitReactInputLocked = false;

	UFUNCTION(BlueprintCallable, Category = "Combat|HitReact")
	void HitReactEnd();

	// ===== 기본 공격 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	EPrimaryAttackHitType PrimaryAttackHitType = EPrimaryAttackHitType::LineTrace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float PrimaryAttackSphereRadius = 120.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float PrimaryAttackForwardOffset = 120.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float PrimaryAttackHalfAngleDeg = 60.f;

	TSubclassOf<ACoF_CommonProjectile> PrimaryProjectileClass = nullptr;
	float PrimaryProjectileSpeed = 0.f;
	float PrimaryProjectileLifeSeconds = 0.f;
	float PrimaryProjectileRadius = 0.f;
	float PrimaryProjectileSpawnForwardOffset = 0.f;
	float PrimaryProjectileSpawnZOffset = 0.f;
	FName PrimaryProjectileSocketA = NAME_None;
	FName PrimaryProjectileSocketB = NAME_None;

	// 콤보 상태
	bool bComboWindowOpen = false;
	bool bComboQueued = false;
	bool bAttackPressed = false; // 콤보를 받는 타이밍(SaveAttack 이후) 에 버튼이 눌렸는가

	// 현재 콤보 단계(0=A, 1=B)
	int32 ComboIndex = 0;

	static constexpr const TCHAR* ComboSections[2] = { TEXT("A"), TEXT("B") };

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void ComboWindowOpen();

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void ComboWindowClose();

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void SaveAttack(); // SaveAttack notify에서 호출

	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void ResetCombo(); // ResetCombo notify에서 호출

	// 공격이 실제로 닿는 순간
	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	void HitStart();

	// 기본공격은 단발 공격이라 사용X 일단 만들어둠
	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	void HitEnd();

	// 투사체 평타의 경우
	UFUNCTION(BlueprintCallable, Category = "Combat|Hit")
	void PrimaryAttack_ThrowProjectile();

	// 우클릭 방패 들기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Defense")
	bool bBlocking = false;

	// ===== Terra / Kallari / Gideon Skill Logic Objects =====
	// - AnimNotify / BP가 기존에 ATP_Character의 UFUNCTION들을 직접 호출하고 있으므로,
	// UFUNCTION 시그니처는 그대로 유지하고, 내부 구현만 Terra_* / Kallari_* 객체로 위임한다.
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
	TObjectPtr<UKallari_Skill1A_DashSlash> Kallari_Skill1A = nullptr;

	UPROPERTY()
	TObjectPtr<UKallari_Skill1B_Backflip> Kallari_Skill1B = nullptr;

	UPROPERTY()
	TObjectPtr<UKallari_Skill2A_ShurikenTeleport> Kallari_Skill2A = nullptr;

	UPROPERTY()
	TObjectPtr<UKallari_Skill2B_ShurikenExplosion> Kallari_Skill2B = nullptr;

	UPROPERTY()
	TObjectPtr<UKallari_UltA_BlinkDash> Kallari_UltA = nullptr;

	UPROPERTY()
	TObjectPtr<UKallari_UltB_Invincible> Kallari_UltB = nullptr;

	UPROPERTY()
	TObjectPtr<UGideon_Skill1A_WaterCannon> Gideon_Skill1A = nullptr;

	UPROPERTY()
	TObjectPtr<UGideon_Skill1B_WaterBomb> Gideon_Skill1B = nullptr;

	UPROPERTY()
	TObjectPtr<UGideon_Skill2A_DebuffBall> Gideon_Skill2A = nullptr;

	UPROPERTY()
	TObjectPtr<UGideon_Skill2B_BackDash> Gideon_Skill2B = nullptr;

	UPROPERTY()
	TObjectPtr<UGideon_UltA_MirrorWaterBeam> Gideon_UltA = nullptr;

	UPROPERTY()
	TObjectPtr<UGideon_UltB_WaterBombDrop> Gideon_UltB = nullptr;

	// ===== Skill 1 =====

	// 스킬1 뭘 선택했는지
	ESkillVariant Skill1Selected = ESkillVariant::None;

	// 스킬1_A 돌진 (UFUNCTION은 유지, 내부 구현은 Terra_Skill1A / Kallari_Skill1A로 위임)
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|A")
	void Skill1A_DashStart(); // 실제 돌진 시, 상태를 fly로 만듬(바닥 충돌 때문에)

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|A")
	void Skill1A_DashEnd();

	UFUNCTION(BlueprintCallable)
	void Skill1A_HitStart(); // 실제 히트 호출

	UFUNCTION(BlueprintCallable)
	void Skill1A_HitEnd();

	TObjectPtr<UAnimMontage> Skill1MontageA = nullptr;
	ESkill1AImplementation Skill1A_Implementation = ESkill1AImplementation::TerraDash;

	float Skill1A_Damage = 0.f;
	float Skill1A_DashDistance = 0.f;
	float Skill1A_DashDuration = 0.f; // 몇 초 동안 밀고 갈지
	float Skill1A_Cooldown = 0.f;
	float Skill1A_HitRadius = 80.f;

	float Skill1A_Range = 0.f;
	FName Skill1A_StartSocket = NAME_None;

	// 스킬1_B 도끼찍기 / 위로 상승하면서 돌진 / 물폭탄 던지기
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1")
	void Skill1B_ApplyAOE();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|B")
	void Skill1B_BackflipStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|B")
	void Skill1B_BackflipEnd();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|B")
	void Skill1B_ThrowProjectile();

	TObjectPtr<UAnimMontage> Skill1MontageB = nullptr;
	ESkill1BImplementation Skill1B_Implementation = ESkill1BImplementation::TerraAxeSlam;

	float Skill1B_Damage = 0.f;
	float Skill1B_Radius = 0.f;
	float Skill1B_Cooldown = 0.f;

	float Skill1B_BackflipDuration = 0.f;
	float Skill1B_BackwardDistance = 0.f;

	TSubclassOf<ACoF_CommonProjectile> Skill1B_ProjectileClass = nullptr;
	float Skill1B_ProjectileForwardSpeed = 0.f;
	float Skill1B_ProjectileUpwardSpeed = 0.f;
	float Skill1B_ProjectileGravityScale = 1.0f;
	float Skill1B_ProjectileLockOnArcPeakHeight = 0.f;
	float Skill1B_ProjectileLifeSeconds = 0.f;
	float Skill1B_ProjectileRadius = 0.f;
	float Skill1B_ProjectileSpawnForwardOffset = 0.f;
	float Skill1B_ProjectileSpawnZOffset = 0.f;
	FName Skill1B_StartSocket = NAME_None;

	// ===== Skill 2 =====

	// 스킬2 뭘 선택했는지
	ESkillVariant Skill2Selected = ESkillVariant::None;

	// 스킬 2_A 방패 밀쳐내기 / Kallari 수리검 텔포
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

	TSubclassOf<ACoF_CommonProjectile> Skill2A_ProjectileClass = nullptr;
	float Skill2A_ProjectileSpeed = 0.f;
	float Skill2A_ProjectileLifeSeconds = 0.f;
	float Skill2A_ProjectileRadius = 0.f;
	float Skill2A_ProjectileSpawnForwardOffset = 0.f;
	float Skill2A_ProjectileSpawnZOffset = 0.f;
	FName Skill2A_ProjectileSpawnSocket = NAME_None;

	TObjectPtr<UAnimMontage> Skill2A_TeleportAttackMontage = nullptr;
	float Skill2A_TeleportAttackRadius = 0.f;
	float Skill2A_TeleportOffsetFromMark = 0.f;

	float Skill2A_DebuffDuration = 0.f;
	float Skill2A_DebuffIncomingDamageMultiplier = 1.f;

	// 스킬 2_B 돌기 / Kallari 수리검 던지고 폭발
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|B")
	void Skill2B_HitStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|B")
	void Skill2B_SpinEnd();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|B")
	void Skill2B_ThrowProjectile();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|B")
	void Skill2B_ExplodeAtMark();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|B")
	void Skill2B_BackDashStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill2|B")
	void Skill2B_BackDashEnd();

	TObjectPtr<UAnimMontage> Skill2MontageB = nullptr;
	ESkill2BImplementation Skill2B_Implementation = ESkill2BImplementation::TerraSpin;

	float Skill2B_DamagePerTick = 0.f;
	float Skill2B_Radius = 0.f;
	float Skill2B_TickInterval = 0.f;
	float Skill2B_Duration = 0.f;
	float Skill2B_Cooldown = 0.f;

	TSubclassOf<ACoF_CommonProjectile> Skill2B_ProjectileClass = nullptr;
	float Skill2B_ProjectileSpeed = 0.f;
	float Skill2B_ProjectileLifeSeconds = 0.f;
	float Skill2B_ProjectileRadius = 0.f;
	float Skill2B_ProjectileSpawnForwardOffset = 0.f;
	float Skill2B_ProjectileSpawnZOffset = 0.f;
	FName Skill2B_ProjectileSpawnSocket = NAME_None;

	TObjectPtr<UAnimMontage> Skill2B_ExplosionMontage = nullptr;
	float Skill2B_ExplosionDamage = 0.f;
	float Skill2B_ExplosionRadius = 0.f;

	float Skill2B_BackDashDuration = 0.f;
	float Skill2B_BackwardDistance = 0.f;

	// ===== Ultimate =====
	ESkillVariant UltSelected = ESkillVariant::None;

	// 구현체
	EUltimateAImplementation UltA_Implementation = EUltimateAImplementation::TerraAllyShield;
	EUltimateBImplementation UltB_Implementation = EUltimateBImplementation::TerraSelfBuff;

	// 궁_A
	TObjectPtr<UAnimMontage> UltMontageA = nullptr;

	// Terra
	float UltA_Duration = 0.f;
	float UltA_Cooldown = 0.f;
	float UltA_Shield = 0.f;
	float UltA_Radius = 0.f;

	// Kallari
	float UltA_Damage = 0.f;
	float UltA_DashDistance = 0.f;
	float UltA_DashDuration = 0.f;
	float UltA_HitRadius = 0.f;
	
	// Gideon
	float UltA_BeamDamagePerTick = 0.f;
	float UltA_BeamDuration = 0.f;
	float UltA_BeamTickInterval = 0.f;
	float UltA_BeamRange = 0.f;
	float UltA_BeamRadius = 0.f;
	FName UltA_BeamStartSocket = NAME_None;

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_ShieldStart(); // Terra

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_ShieldEnd(); // Terra

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_BlinkHitStart(); // Kallari

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_BlinkDashStart(); // Kallari

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_BlinkDashEnd(); // Kallari

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_BeamStart();		// Gideon

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|A")
	void UltA_BeamEnd();		// Gideon

	// 궁_B
	TObjectPtr<UAnimMontage> UltMontageB = nullptr;

	// Terra
	float UltB_Duration = 0.f;
	float UltB_Cooldown = 0.f;
	float UltB_Shield = 0.f;
	float UltB_AttackMultiplier = 1.f;

	// Kallari
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills|Ult|B|Kallari")
	float UltB_InvincibleDuration = 0.f;

	float AttackMultiplier = 1.0f;

	// Gideon
	TSubclassOf<AGideon_UltB_WaterBombActor> UltB_WaterBombActorClass = nullptr;
	float UltB_WaterBombDamage = 0.f;
	float UltB_WaterBombRadius = 0.f;
	float UltB_WaterBombTargetDistance = 0.f;
	float UltB_WaterBombFallHeight = 0.f;
	float UltB_WaterBombFallDuration = 0.f;
	float UltB_WaterBombGroundTraceUp = 0.f;
	float UltB_WaterBombGroundTraceDown = 0.f;

	// Gideon UltB 사용 중 캐릭터를 공중에 띄우기 위한 설정값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills|Ult|B|Gideon")
	float UltB_GideonLiftHeight = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills|Ult|B|Gideon")
	float UltB_GideonLiftUpDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills|Ult|B|Gideon")
	float UltB_GideonLiftHoldDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills|Ult|B|Gideon")
	float UltB_GideonLiftDownDuration = 0.25f;

	FTimerHandle GideonUltB_LiftUpTimerHandle;
	FTimerHandle GideonUltB_LiftDownTimerHandle;
	FTimerHandle GideonUltB_LiftHoldTimerHandle;

	FVector GideonUltB_LiftStartLocation = FVector::ZeroVector;
	FVector GideonUltB_LiftTopLocation = FVector::ZeroVector;

	float GideonUltB_LiftElapsedTime = 0.f;

	bool bGideonUltBLifting = false;

	EMovementMode GideonUltB_PrevMovementMode = MOVE_Walking;

	void StartGideonUltBLift();
	void TickGideonUltBLiftUp();
	void HoldGideonUltBLift();
	void TickGideonUltBLiftDown();
	void FinishGideonUltBLift();


	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|B")
	void UltB_BuffStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|B")
	void UltB_BuffEnd();

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|B")
	void UltB_WaterBombDropStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|B")
	void UltB_GideonLiftStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Ult|B")
	void UltB_GideonLiftDownStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Skills|Ult|B|Visual")
	void BP_UltBVisualStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Skills|Ult|B|Visual")
	void BP_UltBVisualEnd();


	// Gideon skill2A 관련 런타임 디버프 상태 변수
	double DebuffBallEndTime = 0.0;
	float DebuffBallIncomingDamageMultiplier = 1.f;
	virtual void ApplyDebuffBall_Implementation(float InDuration, float InIncomingDamageMultiplier) override;
	virtual bool IsDebuffBallActive_Implementation() const override;
	virtual float GetDebuffBallIncomingDamageMultiplier_Implementation() const override;


	// ===== 락온 관련 멤버 변수들 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	FName BossLockOnTag = TEXT("BOSS");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	float LockOnMaxDistance = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	float LockOnRotateSpeed = 4.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	bool bLockOnEnabled = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<AActor> LockOnTarget = nullptr;

	// 락온 카메라/ui 관련 멤버 변수들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Camera")
	float LockOnCameraArmLength = 360.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Camera")
	FVector LockOnCameraSocketOffset = FVector(0.f, 60.f, 40.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|Camera")
	float LockOnCameraInterpSpeed = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|UI")
	TSubclassOf<UUserWidget> LockOnWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn|UI")
	float LockOnWidgetWorldOffsetZ = 120.f;

	UPROPERTY()
	TObjectPtr<UUserWidget> LockOnWidgetInstance = nullptr;

	float DefaultCameraArmLength = 0.f;
	FVector DefaultCameraSocketOffset = FVector::ZeroVector;

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