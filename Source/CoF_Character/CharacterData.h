#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SkillTypes.h"

#include "CharacterData.generated.h"


class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;


UCLASS(BlueprintType)
class COF_CHARACTER_API UCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ===== Visual / Animation =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSubclassOf<UAnimInstance> AnimClass;

	// ===== Stats =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxHp = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxWalkSpeed = 500.f;

	// ===== Combat (Trace 기반) =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float TraceRange = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float Damage = 10.f;

	// ===== HitReact =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitReact")
	TObjectPtr<UAnimMontage> HitReactMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitReact")
	float HitReactPlayRate = 1.0f;

	// Combat Animation - 기본 콤보 공격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> PrimaryComboMontage;

	// Blocking Animation 방패 들기 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> BlockHoldMontage = nullptr;


	// ------------ Skill 1 ------------
	// 어떤 걸 선택했는지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1")
	ESkillVariant Skill1Selected = ESkillVariant::None;

	// 각각의 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1")
	TObjectPtr<UAnimMontage> Skill1_Montage_A = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1")
	TObjectPtr<UAnimMontage> Skill1_Montage_B = nullptr;


	// 1-A(돌진) 파라미터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_DashDistance = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_DashDuration = 0.25f;   // 몇 초 동안 밀고 갈지

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_Cooldown = 5.f;
	

	// 1-B(도끼찍기) 파라미터: 지금은 광역기라 radius/damage만 있으면 됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B")
	float Skill1B_Damage = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B")
	float Skill1B_Radius = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B")
	float Skill1B_Cooldown = 5.f;


	// ------------ Skill 2 ------------
	// 어떤 걸 선택했는지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2")
	ESkillVariant Skill2Selected = ESkillVariant::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2")
	TObjectPtr<UAnimMontage> Skill2_Montage_A = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2")
	TObjectPtr<UAnimMontage> Skill2_Montage_B = nullptr;


	// 2_A 방패 밀쳐내기 (전방공격)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A")
	float Skill2A_Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A")
	float Skill2A_Radius = 220.f;              // 폭(구 반경)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A")
	float Skill2A_ForwardOffset = 150.f;        // 전방 중심 오프셋

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A")
	float Skill2A_HalfAngleDeg = 60.f;          // 부채꼴 반각(총 120도)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A")
	float Skill2A_Cooldown = 6.f;

	// 2_B 돌기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B")
	float Skill2B_DamagePerTick = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B")
	float Skill2B_Radius = 230.f;          // 몸통+칼 범위(튜닝)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B")
	float Skill2B_TickInterval = 0.2f;     // 가렌 E 느낌 틱당 데미지(0.2~0.25)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B")
	float Skill2B_Duration = 2.5f;         // duration 후 자동 종료

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B")
	float Skill2B_Cooldown = 8.f;


	// ===== Ultimate =====
	// 어떤 걸 선택했는지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult")
	ESkillVariant UltSelected = ESkillVariant::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult")
	TObjectPtr<UAnimMontage> Ult_Montage_A = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult")
	TObjectPtr<UAnimMontage> Ult_Montage_B = nullptr;

	// ===== Ult_A : Ally Shield =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A")
	float UltA_Duration = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A")
	float UltA_Cooldown = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A")
	float UltA_Shield = 200.f;   // 아군에게 부여할 보호막 양

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A")
	float UltA_Radius = 600.f;   // 주변 범위(구)

	// ===== Ult_B : Self Buff =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B")
	float UltB_Duration = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B")
	float UltB_Cooldown = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B")
	float UltB_Shield = 200.f;		// 체력 증가: "최대체력 증가량(절대값)"

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B")
	float UltB_AttackMultiplier = 1.3f; //공격력 증가: 30% 증가
};
