// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillTypes.generated.h"


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