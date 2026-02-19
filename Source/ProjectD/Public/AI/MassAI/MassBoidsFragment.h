// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassBoidsFragment.generated.h"

/**
 * PROJECTD_API
 */
USTRUCT()
struct PROJECTD_API FMassBoidsFragment : public FMassSharedFragment
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    float MinMoveSpeed = 100.0f;

    UPROPERTY(EditAnywhere)
    float MaxMoveSpeed = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float RotationSpeed = 5.0f;
    
    // 주변 이웃을 감지할 반경
    UPROPERTY(EditAnywhere)
    float DetectionRadius = 500.0f;

    // 주변 이웃 피하는 반경
    UPROPERTY(EditAnywhere)
    float AvoidRadius = 150.0f;

    // 계산 중, 최댓값에 대한 제한용도
    UPROPERTY(EditAnywhere)
    float MaxSteerWeight = 50.0f;

    // 장애물 관련 확인 Dist
    UPROPERTY(EditAnywhere, Category = "Obstacle")
    float ObstacleCheckDistance = 500.0f;

    // 장애물 피하는 광선의 개수
    UPROPERTY(EditAnywhere, Category = "Obstacle")
    int32 ObstacleAvoidanceLineCount = 5;

    // 장애물 피하는 광선의 각도
    UPROPERTY(EditAnywhere, Category = "Obstacle")
    float ObstacleAvoidanceDegree = 30.0f;

    // 장애물 피하는 우선순위
    UPROPERTY(EditAnywhere, Category = "Obstacle")
    float ObstacleAvoidanceWeight = 10.0f;

    // 장애물 CollisionType
    UPROPERTY(EditAnywhere, Category = "Obstacle")
    TEnumAsByte<ECollisionChannel> ObstacleTraceChannel = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, Category = "Obstacle")
    float SpiralFactor = 0.5f;

    // 활동 반경 (이 범위를 벗어나면 돌아옴)
    UPROPERTY(EditAnywhere, Category = "Boids|Bounds")
    float BoundsRadius = 1000.0f;

    // 중심으로 돌아오려는 힘의 세기
    UPROPERTY(EditAnywhere, Category = "Boids|Bounds")
    float BoundsWeight = 1.0f;

public:
    // Boids 계산에 필요한 파라미터 (설정값)
    UPROPERTY(EditAnywhere)
    float SeparationWeight = 1.0f;

    UPROPERTY(EditAnywhere)
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere)
    float CohesionWeight = 1.0f;
};