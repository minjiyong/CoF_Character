#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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
};
