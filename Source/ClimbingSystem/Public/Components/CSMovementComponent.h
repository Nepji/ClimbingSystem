// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CSMovementComponent.generated.h"

class ACSCharacter;
class UAnimMontage;
class UAnimInstance;


UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		MOVE_Climb UMETA(DisplayName = "Climb Mode")
	};
}

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLIMBINGSYSTEM_API UCSMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCSMovementComponent();
	
	void ToggleClimbing(bool bEnableClimb);
	bool IsClimbing() const;
	FORCEINLINE FVector GetClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal;}
	FVector GetUntrotatedClimbVelocity() const;


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;

	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;

private:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbableSurfaceTraceTypes;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing Advanced", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing Advanced", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceHalfHeight = 70.0f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing Advanced", meta = (AllowPrivateAccess = "true"))
	float EyeTraceDistance = 100.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing Advanced", meta = (AllowPrivateAccess = "true"))
	float CapsuleHalfHeight = 48.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing Advanced", meta = (AllowPrivateAccess = "true"))
	float MaxDegreeToSurface = 10.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxBreakClimbDeceleration = 400.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbAcceleration = 300.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* IdleToClimbMontage;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* IdleToHangMontage;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbToTopMontage;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Character Movement: Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* VaultMontage;

private:
	TArray<FHitResult> ClimbableSurfacesTraceResults;
	
	FVector CurrentClimbableSurfaceLocation;
	FVector CurrentClimbableSurfaceNormal;

	UPROPERTY()
	UAnimInstance* CharacterAnimInstance;

	UPROPERTY()
	ACSCharacter* PlayerCharacter;

	
private:
	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool bDrawPersistentShapes = false);
	FHitResult DoLineSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool bDrawPersistentShapes = false);

	bool TraceClimbableSurfaces();
	bool TraceHangSurfaces(float TraceDistance, float TraceStartOffset = 0.f,bool bShowDebugShape = false,bool bDrawPersistantShapes = false);
	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.f,bool bShowDebugShape = false,bool bDrawPersistantShapes = false);
	bool CanStartClimbing();
	bool CanStartHanging();

	void StartClimbing();
	void StopClimbing();

	void PhysClimb(float deltaTime, int32 Iterations);
	void ProcessClimbableSurfaceInfo();
	
	bool ShouldStopClimbing();
	bool CheckHasReachedFloor();
	bool CheckHasReachedLedge();

	void TryStartVaulting();
	bool CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition);

	FQuat GetClimbRotation(float DeltaTime);
	void SnapMovementToClimbableSurface(float DeltaTime);

	void PlayMontage(UAnimMontage* MontageToPlay);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition);
};
