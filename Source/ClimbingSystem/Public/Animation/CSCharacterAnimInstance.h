// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CSCharacterAnimInstance.generated.h"

class UCSMovementComponent;
class ACSCharacter;


UCLASS()
class CLIMBINGSYSTEM_API UCSCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;


protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = Reference, meta = (AllowPrivateAccess = "true"))
	float GroundSpeed;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = Reference, meta = (AllowPrivateAccess = "true"))
	float AirSpeed;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = Reference, meta = (AllowPrivateAccess = "true"))
	bool bShouldMove;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = Reference, meta = (AllowPrivateAccess = "true"))
	bool bIsFalling ;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = Reference, meta = (AllowPrivateAccess = "true"))
	bool bIsClimbing;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = Reference, meta = (AllowPrivateAccess = "true"))
	FVector ClimbingVelocity;

private:
	UPROPERTY()
	ACSCharacter* ClimbingSystemCharacter = nullptr;

	UPROPERTY()
	UCSMovementComponent* ClimbingMovementComponent = nullptr;

private:
	void GetGroundSpeed();
	void GetAirSpeed();
	void GetShouldMove();
	void GetIsFalling();
	void GetIsClimbing();
	void GetClimbingVelocity();
	
};
