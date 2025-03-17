// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/CSCharacterAnimInstance.h"

#include "CSCharacter.h"
#include "Components/CSMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
void UCSCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ClimbingSystemCharacter = Cast<ACSCharacter>(TryGetPawnOwner());

	if (ClimbingSystemCharacter)
	{
		ClimbingMovementComponent = ClimbingSystemCharacter->GetCustomMovementComponent();
	}
}
void UCSCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!ClimbingSystemCharacter || !ClimbingMovementComponent)
	{
		return;
	}

	GetGroundSpeed();
	GetAirSpeed();
	GetShouldMove();
	GetIsFalling();
}
void UCSCharacterAnimInstance::GetGroundSpeed()
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(ClimbingSystemCharacter->GetVelocity());
}
void UCSCharacterAnimInstance::GetAirSpeed()
{
	AirSpeed = ClimbingSystemCharacter->GetVelocity().Z;
}
void UCSCharacterAnimInstance::GetShouldMove()
{
	const float MinGroundSpeed = 5.f;
	bShouldMove = ClimbingMovementComponent->GetCurrentAcceleration().Size() > 0 && GroundSpeed > MinGroundSpeed && !bIsFalling;
}
void UCSCharacterAnimInstance::GetIsFalling()
{
	bIsFalling = ClimbingMovementComponent->IsFalling();
}