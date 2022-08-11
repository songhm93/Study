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

	}
}
