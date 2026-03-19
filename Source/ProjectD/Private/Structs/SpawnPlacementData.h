#pragma once

#include "CoreMinimal.h"
#include "Skill/PDDamageableSkillActor.h"
#include "SpawnPlacementData.generated.h"

USTRUCT(BlueprintType)
struct FSpawnPlacementData
{
	GENERATED_BODY()

	/** 실제 스폰될 액터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement")
	TSubclassOf<AActor> SpawnActorClass = nullptr;

	/** 프리뷰에 사용할 공용 메시 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Preview")
	TObjectPtr<UStaticMesh> PreviewStaticMesh = nullptr;

	/** 프리뷰 머티리얼 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Preview")
	TObjectPtr<UMaterialInterface> PreviewMaterial = nullptr;

	/** 프리뷰 스케일 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Preview")
	FVector PreviewScale = FVector(1.f, 1.f, 1.f);

	/** 실제 스폰 스케일 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Spawn")
	FVector SpawnScale = FVector(1.f, 1.f, 1.f);

	/** 플레이어 전방 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Trace")
	float ForwardDistance = 200.f;

	/** Trace 시작 높이 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Trace")
	float TraceStartHeight = 120.f;

	/** 아래로 쏘는 깊이 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Trace")
	float TraceDepth = 500.f;

	/** Hit 위치에 월드 기준으로 더해지는 위치 오프셋 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Spawn")
	FVector SpawnLocationOffset = FVector::ZeroVector;

	/** 회전 오프셋 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Spawn")
	FRotator SpawnRotationOffset = FRotator::ZeroRotator;

	/** 플레이어 Yaw 따라갈지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Spawn")
	bool bUseOwnerYaw = true;

	/** 자동 파괴 시간. 0 이하면 자동 파괴 안 함 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Lifetime")
	float LifeTime = 0.f;

	/** 바닥만 맞으면 거의 다 설치 가능하게 갈 것이므로 1차는 단순 처리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Validation")
	bool bRequireGroundHit = true;

	/** 아래는 APDDamageableSkillActor 계열 초기화용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Damageable")
	bool bInitializeAsDamageableSkillActor = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Damageable", meta=(EditCondition="bInitializeAsDamageableSkillActor"))
	float MaxHealth = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Damageable", meta=(EditCondition="bInitializeAsDamageableSkillActor"))
	EPDShieldType DamageableType = EPDShieldType::Wall;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Damageable", meta=(EditCondition="bInitializeAsDamageableSkillActor"))
	TObjectPtr<UStaticMesh> SpawnStaticMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement|Damageable", meta=(EditCondition="bInitializeAsDamageableSkillActor"))
	TObjectPtr<UMaterialInterface> SpawnBaseMaterial = nullptr;

	bool IsValidData() const
	{
		return SpawnActorClass != nullptr;
	}

	bool IsValidForPlacement() const
	{
		return IsValidData() && PreviewStaticMesh != nullptr;
	}
};
