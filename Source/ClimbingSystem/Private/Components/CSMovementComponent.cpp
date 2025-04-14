
#include "Components/CSMovementComponent.h"

#include "CSCharacter.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Dev/DebugHelper.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void UCSMovementComponent::ToggleClimbing(bool bEnableClimb)
{
	if (bEnableClimb)
	{
		if (CanStartClimbing())
		{
			PlayMontage(IdleToClimbMontage);
		}
		else if (CanStartHanging())
		{
			PlayMontage(IdleToHangMontage);
		}
		else
		{
			TryStartVaulting();
		}
	}
	if (!bEnableClimb)
	{
		Debug::Print(TEXT("Can NOT Start  climbing"));
		StopClimbing();
	}
}
void UCSMovementComponent::RequestHopping()
{
	const FVector UnrotatedLastInputVector = UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(),GetLastInputVector());

	const float DotResult = FVector::DotProduct(UnrotatedLastInputVector.GetSafeNormal(),FVector::UpVector);

	//Point the direction Up\Dawn
	const float DotAccuracy = 0.9f;
	if(DotResult >= DotAccuracy)
	{
		HandleHopUp();
	}
	else if(DotResult <= -DotAccuracy)
	{
		HandleHopDawn();
	}

}
bool UCSMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_Climb;
}
UCSMovementComponent::UCSMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}
FVector UCSMovementComponent::GetUntrotatedClimbVelocity() const
{
	return UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), Velocity);
}
void UCSMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if (CharacterAnimInstance)
	{
		CharacterAnimInstance->OnMontageEnded.AddDynamic(this, &UCSMovementComponent::OnClimbMontageEnded);
		CharacterAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCSMovementComponent::OnClimbMontageEnded);
	}

	PlayerCharacter = Cast<ACSCharacter>(CharacterOwner);
}

void UCSMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
void UCSMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(CapsuleHalfHeight);

		OnEnterClimbState.ExecuteIfBound();
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(CapsuleHalfHeight * 2);
		const FRotator DirtyRotator = UpdatedComponent->GetComponentRotation();
		const FRotator CleanStandRotation = FRotator(0.f, DirtyRotator.Yaw, 0.f);
		UpdatedComponent->SetRelativeRotation(CleanStandRotation);
		StopMovementImmediately();

		OnExitClimbState.ExecuteIfBound();
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}
void UCSMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (IsClimbing())
	{
		PhysClimb(deltaTime, Iterations);
	}

	Super::PhysCustom(deltaTime, Iterations);
}
float UCSMovementComponent::GetMaxSpeed() const
{
	if (IsClimbing())
	{
		return MaxClimbSpeed;
	}
	return Super::GetMaxSpeed();
}
float UCSMovementComponent::GetMaxAcceleration() const
{
	if (IsClimbing())
	{
		return MaxClimbAcceleration;
	}
	return Super::GetMaxAcceleration();
}
FVector UCSMovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const
{
	const bool IsPlayingRMMontage = IsFalling() && CharacterAnimInstance && CharacterAnimInstance->IsAnyMontagePlaying();

	if (IsPlayingRMMontage)
	{
		return RootMotionVelocity;
	}
	return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
}
TArray<FHitResult> UCSMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistentShapes)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;
	if (bShowDebugShape)
	{
		if (bDrawPersistentShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
		else
		{
			DebugTraceType = EDrawDebugTrace::ForOneFrame;
		}
	}

	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this,
		Start,
		End,
		ClimbCapsuleTraceRadius,
		ClimbCapsuleTraceHalfHeight,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutCapsuleTraceHitResults,
		false);
	return OutCapsuleTraceHitResults;
}
FHitResult UCSMovementComponent::DoLineSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistentShapes)
{
	FHitResult OutCapsuleTraceHitResult;
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;

	if (bShowDebugShape)
	{
		if (bDrawPersistentShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
		else
		{
			DebugTraceType = EDrawDebugTrace::ForOneFrame;
		}
	}

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutCapsuleTraceHitResult,
		false);
	return OutCapsuleTraceHitResult;
}
bool UCSMovementComponent::TraceClimbableSurfaces()
{
	const float OffsetValue = 30.0f;

	const FVector StartOffset = UpdatedComponent->GetForwardVector() * OffsetValue;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	ClimbableSurfacesTraceResults = DoCapsuleTraceMultiByObject(Start, End);

	return !ClimbableSurfacesTraceResults.IsEmpty();
}
bool UCSMovementComponent::TraceHangSurfaces(float TraceDistance, float TraceStartOffset, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	const FVector CapsuleStart = UpdatedComponent->GetComponentLocation();
	const FVector ForwardDirection = UpdatedComponent->GetForwardVector();
	const FVector CapsuleEnd = CapsuleStart + ForwardDirection * TraceDistance;

	if (DoLineSingleByObject(CapsuleStart, CapsuleEnd).bBlockingHit)
	{
		return false;
	}

	// Full Capsule + Half + Space = 4X multiplier
	const float ToFloorDistanceLimit = CapsuleHalfHeight * 4;
	const FVector ToFloorEnd = CapsuleEnd + FVector::DownVector * ToFloorDistanceLimit;

	FHitResult FloorHit = DoLineSingleByObject(CapsuleEnd, ToFloorEnd);
	if (FloorHit.bBlockingHit)
	{
		return false;
	}

	const FVector ToWallStart = ToFloorEnd;
	const FVector ToWallEnd = ToWallStart + ForwardDirection * TraceDistance;

	ClimbableSurfacesTraceResults = DoCapsuleTraceMultiByObject(ToWallStart, ToWallEnd, true);

	return !ClimbableSurfacesTraceResults.IsEmpty();
}
FHitResult UCSMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance;

	return DoLineSingleByObject(Start, End);
}
bool UCSMovementComponent::CanStartClimbing()
{
	if (IsFalling() || !TraceClimbableSurfaces()
		|| !TraceFromEyeHeight(EyeTraceDistance).bBlockingHit)
	{
		return false;
	}
	return true;
}
bool UCSMovementComponent::CanStartHanging()
{
	if (IsFalling() || !TraceHangSurfaces(EyeTraceDistance))
	{
		return false;
	}
	return true;
}
void UCSMovementComponent::StartClimbing()
{
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climb);
}
void UCSMovementComponent::StopClimbing()
{
	SetMovementMode(MOVE_Falling);
}
void UCSMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	TraceClimbableSurfaces();
	ProcessClimbableSurfaceInfo();

	if (ShouldStopClimbing() || CheckHasReachedFloor())
	{
		StopClimbing();
	}

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(deltaTime, 0.f, true, MaxBreakClimbDeceleration);
	}

	ApplyRootMotionToVelocity(deltaTime);

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Adjusted, GetClimbRotation(deltaTime), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}

	SnapMovementToClimbableSurface(deltaTime);

	if (CheckHasReachedLedge())
	{
		PlayMontage(ClimbToTopMontage);
	}
}
void UCSMovementComponent::ProcessClimbableSurfaceInfo()
{
	CurrentClimbableSurfaceLocation = FVector::ZeroVector;
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if (ClimbableSurfacesTraceResults.IsEmpty())
	{
		return;
	}

	for (const FHitResult& TraceHitResult : ClimbableSurfacesTraceResults)
	{
		CurrentClimbableSurfaceLocation += TraceHitResult.ImpactPoint;
		CurrentClimbableSurfaceNormal += TraceHitResult.ImpactNormal;
	}

	CurrentClimbableSurfaceLocation /= ClimbableSurfacesTraceResults.Num();
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal();
}
bool UCSMovementComponent::ShouldStopClimbing()
{
	if (ClimbableSurfacesTraceResults.IsEmpty())
	{
		return true;
	}

	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	const float DegreeDiff = FMath::RadiansToDegrees(FMath::Acos(DotResult));

	return DegreeDiff <= MaxDegreeToSurface;
}
bool UCSMovementComponent::CheckHasReachedFloor()
{
	const FVector DownVector = -UpdatedComponent->GetUpVector();
	const FVector StartOffset = DownVector * StartOffset;

	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + DownVector;

	TArray<FHitResult> FloorHitResults = DoCapsuleTraceMultiByObject(Start, End);

	if (FloorHitResults.IsEmpty())
	{
		return false;
	}

	const float ToFloorDistanceLimit = -10.f;

	for (const FHitResult& FloorHit : FloorHitResults)
	{
		if (FVector::Parallel(FloorHit.ImpactNormal, -FVector::UpVector) && GetUntrotatedClimbVelocity().Z < ToFloorDistanceLimit)
		{
			return true;
		}
	}
	return false;
}
bool UCSMovementComponent::CheckHasReachedLedge()
{
	const FHitResult EyeTraceLine = TraceFromEyeHeight(EyeTraceDistance);
	if (EyeTraceLine.bBlockingHit)
	{
		return false;
	}

	const float ToFloorDistanceLimit = 40.f;

	const FVector Start = EyeTraceLine.TraceEnd;
	const FVector End = Start + -FVector::UpVector * ToFloorDistanceLimit;

	return DoLineSingleByObject(Start, End).bBlockingHit;
}
void UCSMovementComponent::TryStartVaulting()
{
	FVector VaultStartPosition;
	FVector VaultLandPosition;

	if (!CanStartVaulting(VaultStartPosition, VaultLandPosition))
	{
		return;
	}

	SetMotionWarpTarget(FName("VaultStartPoint"),VaultStartPosition);
	SetMotionWarpTarget(FName("VaultLandPoint"),VaultLandPosition);

	StartClimbing();
	PlayMontage(VaultMontage);
}
bool UCSMovementComponent::CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition)
{
	if (IsFalling())
	{
		return false;
	}

	OutVaultStartPosition = FVector::ZeroVector;
	OutVaultLandPosition = FVector::ZeroVector;

	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentUpVector = UpdatedComponent->GetUpVector();
	const FVector ComponentDawnVector = -UpdatedComponent->GetUpVector();

	const int32 MaxVaultTrace = 4;
	const float DistanceTo = 75.f;
	
	for (int32 i = 0; i < MaxVaultTrace; i++)
	{
		const float LengthMultiplier = DistanceTo * (i + 1);
		const FVector Start = ComponentLocation + ComponentUpVector * 100.f + ComponentForward * LengthMultiplier;
		const FVector End = Start + ComponentDawnVector * LengthMultiplier;

		FHitResult VaultHitTrace = DoLineSingleByObject(Start, End);

		if (VaultHitTrace.bBlockingHit)
		{
			if (i == 0)
			{
				OutVaultStartPosition = VaultHitTrace.ImpactPoint;
			}
			else if (i == MaxVaultTrace - 1)
			{
				OutVaultLandPosition = VaultHitTrace.ImpactPoint;
			}
		}
	}

	if (OutVaultStartPosition != FVector::ZeroVector && OutVaultLandPosition != FVector::ZeroVector)
	{
		return true;
	}
	return false;
}
FQuat UCSMovementComponent::GetClimbRotation(float DeltaTime)
{
	const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();
	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return CurrentQuat;
	}

	const FQuat TargetQuat = FRotationMatrix::MakeFromX(-CurrentClimbableSurfaceNormal).ToQuat();

	return FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 5.0f);
}
void UCSMovementComponent::SnapMovementToClimbableSurface(float DeltaTime)
{
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();

	const FVector ProjectedCharacterToSurface = (CurrentClimbableSurfaceLocation - ComponentLocation).ProjectOnTo(ComponentForward);

	const FVector SnapVector = -CurrentClimbableSurfaceNormal * ProjectedCharacterToSurface.Length();

	UpdatedComponent->MoveComponent(
		SnapVector * DeltaTime * MaxClimbSpeed, //
		UpdatedComponent->GetComponentQuat(),	//
		true);
}
void UCSMovementComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay || !CharacterAnimInstance || CharacterAnimInstance->IsAnyMontagePlaying())
	{
		return;
	}

	CharacterAnimInstance->Montage_Play(MontageToPlay);
}
void UCSMovementComponent::OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == IdleToClimbMontage || Montage == IdleToHangMontage)
	{
		StartClimbing();
		StopMovementImmediately();
	}
	if (Montage == ClimbToTopMontage || Montage == VaultMontage)
	{
		SetMovementMode(MOVE_Walking);
	}
}
void UCSMovementComponent::SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition)
{
	if (!PlayerCharacter)
	{
		return;
	}
	PlayerCharacter->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromLocation(InWarpTargetName, InTargetPosition);
}
bool UCSMovementComponent::CanHop(FVector& OutHopUpStartPosition, float VerticalOffset, bool HorizontalMovement)
{
	// ChestOffset
	const float TraceDistance = 100.f;
	const float TraceStartOffset = -20.f;
	FHitResult HopUpHit = TraceFromEyeHeight(TraceDistance, TraceStartOffset, true, true);

	FHitResult SafetyLedgeHit = TraceFromEyeHeight(TraceDistance, VerticalOffset, true , true );
	Debug::Print(TEXT("HELLO"));

	if (HopUpHit.bBlockingHit && SafetyLedgeHit.bBlockingHit)
	{
		OutHopUpStartPosition = SafetyLedgeHit.ImpactPoint;
		return true;
	}
	return false;
}
void UCSMovementComponent::HandleHopUp()
{
	FVector HopUpTargetPoint;
	if(CanHop(HopUpTargetPoint,HopOffset))
	{
		SetMotionWarpTarget(FName("HopUpTargetPoint"),HopUpTargetPoint);
		PlayMontage(HopUpMontage);
	}
}

void UCSMovementComponent::HandleHopDawn()
{
	FVector HopDawnTargetPoint;
	const float HopDawnOffset = -HopOffset;
	if(CanHop(HopDawnTargetPoint,HopDawnOffset))
	{
		SetMotionWarpTarget(FName("HopDawnTargetPoint"),HopDawnTargetPoint);
		PlayMontage(HopDawnMontage);
	}
}

