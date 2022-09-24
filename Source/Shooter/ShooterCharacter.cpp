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
#include "Components/CapsuleComponent.h"


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
	bCrouching = false;
	CurrentCapsuleHalfHeight = 0.f;
	StandingCapsuleHalfHeight = 88.f;
	CrouchingCapsuleHalfHeight = 44.f;
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));
	BaseGroundFriction = 2.f;
	CrouchingGroundFriction = 100.f;
	HighlightedSlot = -1;
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
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();

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
	InterpCapsuleHalfHeight(DeltaTime);
	
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
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ThisClass::CrouchButton);

	PlayerInputComponent->BindAction(TEXT("FKey"), IE_Pressed, this, &ThisClass::FKeyPressed);
	PlayerInputComponent->BindAction(TEXT("1Key"), IE_Pressed, this, &ThisClass::OneKeyPressed);
	PlayerInputComponent->BindAction(TEXT("2Key"), IE_Pressed, this, &ThisClass::TwoKeyPressed);
	PlayerInputComponent->BindAction(TEXT("3Key"), IE_Pressed, this, &ThisClass::ThreeKeyPressed);
	PlayerInputComponent->BindAction(TEXT("4Key"), IE_Pressed, this, &ThisClass::FourKeyPressed);
	PlayerInputComponent->BindAction(TEXT("5Key"), IE_Pressed, this, &ThisClass::FiveKeyPressed);
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
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
	if(!bCrouching)
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
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
		const AWeapon* TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		if(TraceHitWeapon)
		{
			if(HighlightedSlot == -1)
			{
				HighlightInventorySlot();
			}
		}
		else
		{
			if(HighlightedSlot != -1)
			{
				UnHighlightInventorySlot();
			}
		}

		if(TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
		{
			TraceHitItem = nullptr;
		}

		if(TraceHitItem && TraceHitItem->GetPickupWidget())
		{
			TraceHitItem->GetPickupWidget()->SetVisibility(true);
			TraceHitItem->EnableCustomDepth();

			if(Inventory.Num() >= INVENTORY_CAPACITY) //인벤 꽉참
			{
				TraceHitItem->SetInventoryFull(true);
			}
			else
			{
				TraceHitItem->SetInventoryFull(false);
			}
		}

		if(TraceHitItemLastFrame)
		{
			if(TraceHitItem != TraceHitItemLastFrame)
			{
				TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				TraceHitItemLastFrame->DisableCustomDepth();
			}
		}

		TraceHitItemLastFrame = TraceHitItem;
	}
	else if(TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
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

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
	if(WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if(nullptr == EquippedWeapon)
		{
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if(!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
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
	if(CombatState == ECombatState::ECS_Reloading) return;
	if(TraceHitItem && OverlapCount > 0)
	{
		TraceHitItem->StartItemCurve(this);
		if(TraceHitItem->GetPickupSound())
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickupSound());
		}
		TraceHitItem = nullptr;
	}
}

void AShooterCharacter::SelectButtonReleased()
{

}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if(Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}

	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
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
		if(Weapon->GetEquipSound())
		{
			UGameplayStatics::PlaySound2D(this, Weapon->GetEquipSound());
		}
		if(Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_Pickedup);
		}
		else //인벤 꽉참.
		{
			SwapWeapon(Weapon);
		}
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

	if(EquippedWeapon && CarryingAmmo() && !EquippedWeapon->ClipIsFull())
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

void AShooterCharacter::GrapClip()
{
	if(nullptr == EquippedWeapon) return;
	if(nullptr == HandSceneComponent) return;

	int32 ClipBoneIndex = EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName());
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(EAttachmentRule::KeepRelative, true);
	
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	
	

	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
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

void AShooterCharacter::CrouchButton()
{
	if(!GetCharacterMovement()->IsFalling() && !bCrouching)
	{
		bCrouching = !bCrouching;
		Crouch();
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else if(bCrouching)
	{
		bCrouching = !bCrouching;
		UnCrouch();
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
	
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight = 0.f;
	if(bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f);

	const float DeltaCapsuleHalfHeight = InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector MeshOffset = FVector(0.f, 0.f, -DeltaCapsuleHalfHeight);
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);

}

void AShooterCharacter::FKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 2) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIdx, int32 NewItemIdx)
{
	if((CurrentItemIdx != NewItemIdx) && \
	(NewItemIdx < Inventory.Num()) && \
	(CombatState == ECombatState::ECS_Unoccupied) || \
	(CombatState == ECombatState::ECS_Equipping))
	{
		AWeapon* OldEquippedWeapon = EquippedWeapon;
		AWeapon* NewWeapon = Cast<AWeapon>(Inventory[NewItemIdx]);
		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_Pickedup);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if(AnimInst && EquipMontage)
		{
			AnimInst->Montage_Play(EquipMontage, 1.f);
			AnimInst->Montage_JumpToSection(FName("Equip"));
			if(SwapSound)
			{
				UGameplayStatics::PlaySound2D(this, SwapSound);
			}
		}
	}
}

void AShooterCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for(int32 i = 0; i < Inventory.Num(); ++i)//?? 여기 들어올일이 있나요?
	{
		if(Inventory[i] == nullptr) //5개면 0 1 2 3 4 인데 거기가 nullptr일리가 없잖아요?
		{
			return i; //??대체 무슨 뜻인지 이게 ..
		}
	}
	if(Inventory.Num() < INVENTORY_CAPACITY) 
	{
		return Inventory.Num();
	}
	return -1; //인벤토리 꽉참
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot = GetEmptyInventorySlot();
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}