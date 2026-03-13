#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "SkillTypes.h"

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
class COF_CHARACTER_API ATP_Character : public ACharacter
{
	GENERATED_BODY()

public:
	ATP_Character();

protected:
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


	// ===== Animation =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> PrimaryComboMontage = nullptr;			// 기본 좌클릭 콤보 공격

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> BlockHoldMontage = nullptr;			// 우클릭 방패 들기


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


	// A/B 중 스킬1 뭘 선택했는지
	ESkillVariant Skill1Selected = ESkillVariant::None;		

	// 스킬1_A 돌진
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|A")
	void Skill1A_DashStart();

	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1|A")
	void Skill1A_DashEnd();

	TObjectPtr<UAnimMontage> Skill1MontageA = nullptr;
	float Skill1A_Damage = 0.f;
	float Skill1A_DashDistance = 0.f;
	float Skill1A_DashDuration = 0.f;			// 몇 초 동안 밀고 갈지
	float Skill1A_TraceRange = 0.f;				// 라인트레이스 길이(돌진 판정)
	float Skill1A_Cooldown = 0.f;
	double Skill1A_NextAvailableTime = 0.0;

	// 스킬1_B 도끼찍기
	UFUNCTION(BlueprintCallable, Category = "Skills|Skill1")
	void Skill1B_ApplyAOE();

	TObjectPtr<UAnimMontage> Skill1MontageB = nullptr;
	float Skill1B_Damage = 0.f;
	float Skill1B_Radius = 0.f;
	float Skill1B_Cooldown = 0.f;
	double Skill1B_NextAvailableTime = 0.0;


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
