#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"


AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bUsePawnControlRotation = true; //캐릭터를 따라 회전
	SpringArm->SocketOffset = FVector(0.f, 50.f, 50.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm , USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	BaseTurnRate= 45.f;
	BaseLookUpRate = 45.f;
	bAiming = false; 
	CameraDefaultFOV = 0.f;
	CameraZoomedFOV = 60.f;
	CameraCurrentFOV= 0.f;
	ZoomInterpSpeed = 20.f;
	HipTurnRate = 90.f;
	HipLookUpRate = 90.f;
	AimingTurnRate = 90.f;
	AimingLookUpRate = 20.f;
	MouseHipTurnRate = 1.f;
	MouseHipLookUpRate = 1.f;
	MouseAimingTurnRate = 0.2f;
	MouseAimingLookUpRate = 0.2f;
	CrosshairSpreadMultiplier = 0.f; 
	CrosshairVelocityFactor = 0.f; 
	CrosshairInAirFactor = 0.f; 
	CrosshairAimFactor = 0.f; 
	CrosshairShootingFactor = 0.f;
	ShootTimeDuration = 0.05f;
	bFiringBullet = false;
	AutomaticFireRate = 0.1; //연사속도
	bCanFire = true;
	bFireButtonPressed = false;
	bShouldTraceForItems = false;
	CameraInterpDist = 250.f;
	CameraInterpHeight = 65.f;
	Starting9mmAmmo = 90;
	StartingARAmmo = 120;
	CombatState = ECombatState::ECS_Unoccupied;
}


void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(Camera)
	{
		CameraDefaultFOV = GetCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	EquipWeapon(SpawnDefaultWeapon());
	InitAmmoMap();
	
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);

	SetLookRates();

	CalculateCrosshairSpread(DeltaTime);
	if(bShouldTraceForItems)
	{
		TraceForItems();
	}
	
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("TurnRate"), this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction(TEXT("Aiming"), IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aiming"), IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction(TEXT("Select"), IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
}

void AShooterCharacter::MoveForward(float Value)
{
	if (nullptr != Controller && Value != 0.f)
	{
		const FRotator YawRotation(0 , Controller->GetControlRotation().Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (nullptr != Controller && Value != 0.f)
	{
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor = bAiming ? MouseAimingTurnRate : MouseHipTurnRate;
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor = bAiming ? MouseAimingLookUpRate : MouseHipLookUpRate;
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if(nullptr == EquippedWeapon) return;
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if(WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunfireMontage();
		CrosshairFireTimerStart();
		EquippedWeapon->DecrementAmmo();

		FireTimerStart();
	}
	
	
	
	
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshair(CrosshairHitResult, OutBeamLocation);

	if(bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.ImpactPoint;
	}
	else
	{

	}

	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector StartToEnd = OutBeamLocation - MuzzleSocketLocation;
	const FVector WeaponTraceEnd = MuzzleSocketLocation + StartToEnd * 1.25f;
	GetWorld()->LineTraceSingleByChannel(	\
		WeaponTraceHit,						\
		WeaponTraceStart,					\
		WeaponTraceEnd,						\
		ECollisionChannel::ECC_Visibility);
	if(WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.ImpactPoint;
		return true;
	}
	
	return false;

}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if(bAiming) CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	else CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	GetCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if(bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange = FVector2D(0.f, 600.f);
	FVector2D VelocityMultiRange = FVector2D(0.f, 1.f);
	FVector Velocity = GetVelocity();

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiRange, Velocity.Size2D());

	if(GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if(bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	if(bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::CrosshairFireTimerStart()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimerHandle, this, &ThisClass::CrosshairFireTimerFinished, ShootTimeDuration);

}

void AShooterCharacter::CrosshairFireTimerFinished()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::FireTimerStart()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	
	
	GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &ThisClass::AutoFireReset, AutomaticFireRate);
	
}

void AShooterCharacter::AutoFireReset() //콜백
{
	CombatState = ECombatState::ECS_Unoccupied;

	if(WeaponHasAmmo())
	{
		if(bFireButtonPressed)
			FireWeapon();
	}
	else
	{
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(	\
		UGameplayStatics::GetPlayerController(this, 0),				\
		CrosshairLocation, 											\
		CrosshairWorldPosition, 									\
		CrosshairWorldDirection);

	if(bScreenToWorld)
	{
		const FVector Start = CrosshairWorldPosition;
		const FVector End = Start + CrosshairWorldDirection * 50'000.f;
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(	\
			OutHitResult,						\
			Start,								\
			End,								\
			ECollisionChannel::ECC_Visibility);
		if(OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.ImpactPoint;
			return true;
		}
	}

	return false;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if(OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

void AShooterCharacter::TraceForItems()
{
	FHitResult ItemTraceResult;
	FVector TempLocation;
	if(TraceUnderCrosshair(ItemTraceResult, TempLocation))
	{
		TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
		if(TraceHitItem && TraceHitItem->GetPickupWidget())
		{
			TraceHitItem->GetPickupWidget()->SetVisibility(true);
		}

		if(TraceHitItemLastFrame)
		{
			if(TraceHitItem != TraceHitItemLastFrame)
			{
				TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
			}
		}

		TraceHitItemLastFrame = TraceHitItem;
	}
	else if(TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if(DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if(EquippedWeapon)
	{
		FDetachmentTransformRules Rules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(Rules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeaponTimerStart();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if(TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);
	}
	
}

void AShooterCharacter::SelectButtonReleased()
{

}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation = Camera->GetComponentLocation();
	const FVector CameraForward = Camera->GetForwardVector();
	return (CameraWorldLocation + CameraForward * CameraInterpDist) + (FVector(0.f, 0.f, CameraInterpHeight));
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	AWeapon* Weapon = Cast<AWeapon>(Item);
	if(Weapon)
	{
		SwapWeapon(Weapon);
	}
}

void AShooterCharacter::InitAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if(nullptr == EquippedWeapon) return false;
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound)
		{
			UGameplayStatics::PlaySound2D(this, FireSound);
		}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if(bBeamEnd)
		{
			if(ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamEnd);
			}

			if(BeamParticle)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticle, SocketTransform);
				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}

void AShooterCharacter::PlayGunfireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if(EquippedWeapon && CarryingAmmo())
	{
		CombatState = ECombatState::ECS_Reloading;
		FName MontageSection = EquippedWeapon->GetReloadMontageSection();
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(MontageSection);
		}
	}
	
}

bool AShooterCharacter::CarryingAmmo()
{
	if(nullptr == EquippedWeapon) return false;
	
	EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	if(AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::FinishReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if(nullptr == EquippedWeapon) return;

	const EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	if(AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];

		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
		//탄창에 넣을 수 있는 탄의 양이, 가진 휴대탄 양보다 많으면 휴대탄을 다 넣을 수 있다.
		if(MagEmptySpace > CarriedAmmo)
		{
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo); //맵에 0으로 초기화
		}
		else //현재 가지고 있는 휴대탄이, 탄창에 넣을 수 있는 양보다 많으면 탄창에 비는 양만큼 넣고, 휴대탄은 그만큼 Minus.
		{
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}