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

	// A/B 각각의 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1")
	TObjectPtr<UAnimMontage> Skill1_Montage_A = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1")
	TObjectPtr<UAnimMontage> Skill1_Montage_B = nullptr;

	// 파라미터: 지금은 광역기라 radius/damage만 있으면 됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1")
	float Skill1_Damage = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1")
	float Skill1_Radius = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills|Skill1")
	float Skill1_Cooldown = 5.f;
};
