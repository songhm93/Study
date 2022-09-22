#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (nullptr == ShooterCharacter)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter)
	{
		bCrouching = ShooterCharacter->IsCrouching();
		
		bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
		FVector Velocity = ShooterCharacter->GetVelocity();
		Velocity.Z = 0;
		Speed = Velocity.Size();

		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
		bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		/* FString RotationMSG = FString::Printf(TEXT("BaseAimRotation:%f"), AimRotation.Yaw);
		FString MovementRotationMSG = FString::Printf(TEXT("MovementRotation:%f"), MovementRotation.Yaw);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, MovementRotationMSG);
		} */
		MovementYawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		//FString MovementOffsetYawMSG = FString::Printf(TEXT("MovementOffsetYaw:%f"), MovementYawOffset);
		/* if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, MovementOffsetYawMSG);
		} */
		if(ShooterCharacter->GetVelocity().Size() > 0.f)
		{
			LastMovementYawOffset = MovementYawOffset;
		}
		bAiming = ShooterCharacter->IsAiming();

		if(bReloading)
			OffsetState = EOffsetState::EOS_Reloading;
		else if(bIsInAir)
			OffsetState = EOffsetState::EOS_InAir;
		else if(ShooterCharacter->IsAiming())
			OffsetState = EOffsetState::EOS_Aiming;
		else
			OffsetState = EOffsetState::EOS_Hip;
	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::TurnInPlace()
{
	if(nullptr == ShooterCharacter) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
	

	if(Speed > 0 || bIsInAir)
	{
		//캐릭터가 움직이고 있음. TurnInPlace X
		RootYawOffset = 0.f;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float TIPYawDelta = TIPCharacterYaw - TIPCharacterYawLastFrame;

		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta); //NormalizeAxis - 각도 범위가 -180~180을 벗어나지 않음.
	
		const float Turning = GetCurveValue(TEXT("Turning")); //Turning은 메타데이터. 애니메이션에 커브에 만들었음. Turning이라는 메타데이터를 가지고 있는
		//애니메이션은 재생하지 않을 때 Turning 값이 0이다. 재생할 땐 1.
		if(Turning > 0.f)
		{
			bTIP = true;
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation = RotationCurve - RotationCurveLastFrame;

			// RootYawOffset > 0 -> 왼쪽으로 회전
			// RootYawOffset < 0 -> 오른쪽으로 회전
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset = FMath::Abs(RootYawOffset);
			if(ABSRootYawOffset > 90.f)
			{
				const float YawExcess = ABSRootYawOffset - 90.f;
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
			
		}
		else
		{
			bTIP = false;
		}
	}

	if(bTIP)
		{
			if(bReloading)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.f;
			}
		}
		else
		{
			if(bCrouching)
			{
				if(bReloading)
				{
					RecoilWeight = 1.f;
				}
				else
				{
					RecoilWeight = 0.1f;
				}
			}
			else
			{
				if(bAiming || bReloading)
				{
					RecoilWeight = 1.f;
				}
				else
				{
					RecoilWeight = 0.5f;
				}
			}
		}	
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if(nullptr == ShooterCharacter) return;
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f);

	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	// if(GEngine) GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, FString::Printf(TEXT("YawDelta : %f"), YawDelta));
	// if(GEngine) GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, FString::Printf(TEXT("Delta.Yaw : %f"), Delta.Yaw));
}