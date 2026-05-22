#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SkillTypes.h"

#include "CharacterData.generated.h"


class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;

class AKallari_Skill2A_ShurikenProjectile;

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

	// ===== Attack =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	EPrimaryAttackHitType PrimaryAttackHitType = EPrimaryAttackHitType::LineTrace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float TraceRange = 800.f;		// (Trace 기반)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float PrimaryAttackSphereRadius = 120.f;	// (Sphere 기반)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float PrimaryAttackForwardOffset = 50.f;	// 전방 부채꼴 범위로 제한 - 앞으로 얼마나

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PrimaryAttackHalfAngleDeg = 60.f;		// 전방 부채꼴 범위로 제한 - 각도


	// Combat Animation - 기본 콤보 공격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Anim")
	TObjectPtr<UAnimMontage> PrimaryComboMontage;

	// ===== HitReact =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitReact")
	TObjectPtr<UAnimMontage> HitReactMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitReact")
	float HitReactPlayRate = 1.0f;

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

	// 1_A 중 어떤건지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	ESkill1AImplementation Skill1A_Implementation = ESkill1AImplementation::TerraDash;

	// 1-A(돌진) 파라미터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_DashDistance = 800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_DashDuration = 0.25f;   // 몇 초 동안 밀고 갈지

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_Cooldown = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|A")
	float Skill1A_HitRadius = 80.f;
	

	// 1_B 중 어떤건지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B")
	ESkill1BImplementation Skill1B_Implementation = ESkill1BImplementation::TerraAxeSlam;

	// 1-B(도끼찍기) 파라미터: 지금은 광역기라 radius/damage만 있으면 됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B")
	float Skill1B_Damage = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B")
	float Skill1B_Radius = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B")
	float Skill1B_Cooldown = 5.f;

	// 1-B Kallari - 위로 상승하는 돌진 공격
	// 1-B Kallari - 공중제비 회피
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B|Kallari")
	float Skill1B_BackflipDuration = 0.16f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B|Kallari")
	float Skill1B_BackwardDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1|B|Kallari")
	float Skill1B_UpwardDistance = 150.f;


	// ------------ Skill 2 ------------
	// 어떤 걸 선택했는지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2")
	ESkillVariant Skill2Selected = ESkillVariant::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2")
	TObjectPtr<UAnimMontage> Skill2_Montage_A = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2")
	TObjectPtr<UAnimMontage> Skill2_Montage_B = nullptr;

	// 2_A 중 어떤건지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A")
	ESkill2AImplementation Skill2A_Implementation = ESkill2AImplementation::TerraShieldPush;

	// 2_A 공용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A")
	float Skill2A_Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A")
	float Skill2A_Cooldown = 6.f;

	// 2_A Terra 방패 밀쳐내기 (전방공격)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Terra")
	float Skill2A_Radius = 220.f;              // 폭(구 반경)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Terra")
	float Skill2A_ForwardOffset = 150.f;        // 전방 중심 오프셋

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Terra")
	float Skill2A_HalfAngleDeg = 60.f;          // 부채꼴 반각(총 120도)

	// 2_A Kallari - 수리검 던지고 텔레포트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	TSubclassOf<AKallari_Skill2A_ShurikenProjectile> Skill2A_ProjectileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	float Skill2A_ProjectileSpeed = 2600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	float Skill2A_ProjectileLifeSeconds = 1.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	float Skill2A_ProjectileRadius = 18.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	float Skill2A_ProjectileSpawnForwardOffset = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	float Skill2A_ProjectileSpawnZOffset = 35.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	FName Skill2A_ProjectileSpawnSocket = NAME_None;

	// 2_A Kallari - 텔레포트 후 돌며 공격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	TObjectPtr<UAnimMontage> Skill2A_TeleportAttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	float Skill2A_TeleportAttackRadius = 270.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|A|Kallari")
	float Skill2A_TeleportOffsetFromMark = 60.f;


	// 2_B 구현 타입
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B")
	ESkill2BImplementation Skill2B_Implementation = ESkill2BImplementation::TerraSpin;

	// 2_B 공용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B")
	float Skill2B_Cooldown = 8.f;

	// 2_B Terra - 회전 공격
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Terra")
	float Skill2B_DamagePerTick = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Terra")
	float Skill2B_Radius = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Terra")
	float Skill2B_TickInterval = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Terra")
	float Skill2B_Duration = 1.2f;

	// 2_B Kallari - 수리검 던지고 폭발
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	TSubclassOf<AKallari_Skill2A_ShurikenProjectile> Skill2B_ProjectileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	float Skill2B_ProjectileSpeed = 2600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	float Skill2B_ProjectileLifeSeconds = 1.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	float Skill2B_ProjectileRadius = 18.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	float Skill2B_ProjectileSpawnForwardOffset = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	float Skill2B_ProjectileSpawnZOffset = 35.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	FName Skill2B_ProjectileSpawnSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	TObjectPtr<UAnimMontage> Skill2B_ExplosionMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	float Skill2B_ExplosionDamage = 24.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill2|B|Kallari")
	float Skill2B_ExplosionRadius = 220.f;


	// ===== Ultimate =====
	// 어떤 걸 선택했는지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult")
	ESkillVariant UltSelected = ESkillVariant::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult")
	TObjectPtr<UAnimMontage> Ult_Montage_A = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult")
	TObjectPtr<UAnimMontage> Ult_Montage_B = nullptr;

	// ===== Ult_A =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A")
	EUltimateAImplementation UltA_Implementation = EUltimateAImplementation::TerraAllyShield;

	// ===== Ult_A : Terra =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A|Terra")
	float UltA_Duration = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A|Terra")
	float UltA_Cooldown = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A|Terra")
	float UltA_Shield = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A|Terra")
	float UltA_Radius = 600.f;

	// ===== Ult_A : Kallari =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A|Kallari")
	float UltA_Damage = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A|Kallari")
	float UltA_DashDistance = 1800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A|Kallari")
	float UltA_DashDuration = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|A|Kallari")
	float UltA_HitRadius = 120.f;

	// ===== Ult_B =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B")
	EUltimateBImplementation UltB_Implementation = EUltimateBImplementation::TerraSelfBuff;

	// ===== Ult_B : Terra =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B|Terra")
	float UltB_Duration = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B|Terra")
	float UltB_Cooldown = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B|Terra")
	float UltB_Shield = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B|Terra")
	float UltB_AttackMultiplier = 1.3f;

	// ===== Ult_B : Kallari =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Ult|B|Kallari")
	float UltB_InvincibleDuration = 3.f;
};
