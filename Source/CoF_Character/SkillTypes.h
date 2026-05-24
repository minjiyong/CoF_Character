// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillTypes.generated.h"

// 평타 - 라인트레이스 기반도 일단 살려두기
UENUM(BlueprintType)
enum class EPrimaryAttackHitType : uint8
{
	LineTrace UMETA(DisplayName = "LineTrace"),
	Sphere    UMETA(DisplayName = "Sphere"),
	Projectile UMETA(DisplayName = "Projectile")
};

// 스킬 데이터 - A/B 2가지 중 선택
UENUM(BlueprintType)
enum class ESkillVariant : uint8
{
	None UMETA(DisplayName = "None"),
	A    UMETA(DisplayName = "A"),
	B    UMETA(DisplayName = "B"),
};

// 스킬 슬롯
UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Skill1 UMETA(DisplayName = "Skill1"),
	Skill2 UMETA(DisplayName = "Skill2"),
	Ult    UMETA(DisplayName = "Ultimate"),
};

// 스킬1_A 중 어떤것인지
UENUM(BlueprintType)
enum class ESkill1AImplementation : uint8
{
	TerraDash			UMETA(DisplayName = "TerraDash"),
	KallariDashSlash	UMETA(DisplayName = "KallariDashSlash")
};

// 스킬1_B 중 어떤것인지
UENUM(BlueprintType)
enum class ESkill1BImplementation : uint8
{
	TerraAxeSlam		UMETA(DisplayName = "TerraAxeSlam"),
	KallariBackflip		UMETA(DisplayName = "KallariBackflip")
};

// 스킬2_A 중 어떤것인지
UENUM(BlueprintType)
enum class ESkill2AImplementation : uint8
{
	TerraShieldPush				UMETA(DisplayName = "TerraShieldPush"),
	KallariShurikenTeleport		UMETA(DisplayName = "KallariShurikenTeleport")
};

// 스킬2_B 중 어떤것인지
UENUM(BlueprintType)
enum class ESkill2BImplementation : uint8
{
	TerraSpin					UMETA(DisplayName = "TerraSpin"),
	KallariShurikenExplosion	UMETA(DisplayName = "KallariShurikenExplosion")
};

// 궁극기 A 중 어떤 것인지
UENUM(BlueprintType)
enum class EUltimateAImplementation : uint8
{
	TerraAllyShield UMETA(DisplayName = "TerraAllyShield"),
	KallariBlinkDash UMETA(DisplayName = "KallariBlinkDash")
};

// 궁극기 B 중 어떤 것인지
UENUM(BlueprintType)
enum class EUltimateBImplementation : uint8
{
	TerraSelfBuff UMETA(DisplayName = "TerraSelfBuff"),
	KallariInvincible UMETA(DisplayName = "KallariInvincible")
};