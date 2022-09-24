#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Curves/CurveVector.h"
AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	ItemName = FString("Default");
	ItemCount = 0;
	ItemRarity = EItemRarity::EIR_Common;
	ItemState = EItemState::EIS_CanPickup;
	ZCurveTime = 0.7f;
	ItemInterpStartLocation = FVector(0.f);
	CameraTargetLoation = FVector(0.f);
	bInterping = false;
	ItemInterpX = 0.f;
	ItemInterpY = 0.f;
	InterpInitYawOffset = 0.f;
	MaterialIndex = 0;
	bCanChangeCustomDepth = true;
	PulseCurveTime = 5.f;
	GlowAmount = 150.f;
	FresnelExponent = 3.f;
	FresnelFraction = 4.f;
	SlotIndex = 0;
	bInventoryFull = false;
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	SetActiveStars();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);

	SetItemProperties(ItemState);

	InitCustomDepth();

	StartPulseTimer();
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bInterping) ItemInterp(DeltaTime);

	UpdatePulse();
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if(ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(1);
			ShooterCharacter->OverlapCountPlus();
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if(ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(-1);
			GetPickupWidget()->SetVisibility(false);
			DisableCustomDepth();
			ShooterCharacter->OverlapCountMinus();
			ShooterCharacter->UnHighlightInventorySlot();
		}
	}
}

void AItem::SetActiveStars()
{
	for(int32 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}

	switch(ItemRarity)
	{
		case EItemRarity::EIR_Damaged:
			ActiveStars[1] = true;
			break;
		case EItemRarity::EIR_Common:
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			break;
		case EItemRarity::EIR_Uncommon:
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			ActiveStars[3] = true;
			break;
		case EItemRarity::EIR_Rare:
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			ActiveStars[3] = true;
			ActiveStars[4] = true;
			break;
		case EItemRarity::EIR_Legendary:
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			ActiveStars[3] = true;
			ActiveStars[4] = true;
			ActiveStars[5] = true;
			break;
		
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch(State)
	{
		case EItemState::EIS_CanPickup:
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			break;
		case EItemState::EIS_Equipped:
			PickupWidget->SetVisibility(false);
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		case EItemState::EIS_Falling:
			ItemMesh->SetSimulatePhysics(true);
			ItemMesh->SetEnableGravity(true);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		case EItemState::EIS_EquipInterping:
			PickupWidget->SetVisibility(false);
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		case EItemState::EIS_Pickedup:
			PickupWidget->SetVisibility(false);
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(false);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(ItemState);
}

void AItem::StartItemCurve(AShooterCharacter* Char)
{
	Character = Char;
	ItemInterpStartLocation = GetActorLocation(); //아이템 시작 위치 초기화
	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);
	GetWorldTimerManager().ClearTimer(PulseTimerHandle);
	GetWorldTimerManager().SetTimer(ItemInterpTimerHandle, this, &ThisClass::FinishInterping, ZCurveTime);
	
	const float CameraRotationYaw = Character->GetCamera()->GetComponentRotation().Yaw;
	const float ItemRotationYaw = GetActorRotation().Yaw;
	InterpInitYawOffset = ItemRotationYaw - CameraRotationYaw;

	bCanChangeCustomDepth = false;
}

void AItem::FinishInterping()
{
	bInterping = false;
	if(Character)
	{
		Character->GetPickupItem(this);

		Character->UnHighlightInventorySlot();
	}
	SetActorScale3D(FVector(1.f));
	DisableGlowMaterial();
	bCanChangeCustomDepth = true;
	DisableCustomDepth();
}

void AItem::ItemInterp(float DeltaTime)
{
	if(!bInterping) return;
	if(Character && ItemZCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimerHandle); //타이머 경과시간 반환. 타이머는 0.7초로, 커브 시간과 동일함.
		const float ZCurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

		FVector ItemLocation = ItemInterpStartLocation;
		const FVector CameraInterpLocation = Character->GetCameraInterpLocation(); //카메라에서 앞으로, Z방향으로 올라간 위치.
		const FVector ItemToCamera = FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z); //델타.
		const float DeltaZ = ItemToCamera.Size();
		ItemLocation.Z += ZCurveValue * DeltaZ;

		const FVector CurrentLocation = GetActorLocation();
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.f);
		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;

		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		const FRotator CameraRotation = Character->GetCamera()->GetComponentRotation();
		FRotator ItemRotation = FRotator(0.f, CameraRotation.Yaw + InterpInitYawOffset, 0.f);
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if(ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
		
	}
}

void AItem::EnableCustomDepth()
{
	if(bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(true);
	}
}

void AItem::DisableCustomDepth()
{
	if(bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(false);
	}
}

void AItem::InitCustomDepth()
{
	DisableCustomDepth();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	if(MI)
	{
		DynamicMI = UMaterialInstanceDynamic::Create(MI, this);
		ItemMesh->SetMaterial(MaterialIndex, DynamicMI);
	}
	EnableGlowMaterial();
}

void AItem::EnableGlowMaterial()
{
	if(DynamicMI)
	{
		DynamicMI->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0);
	}
}

void AItem::DisableGlowMaterial()
{
	if(DynamicMI)
	{
		DynamicMI->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1);
	}
}

void AItem::ResetPulseTimer()
{
	StartPulseTimer();
}

void AItem::StartPulseTimer()
{
	if(ItemState == EItemState::EIS_CanPickup)
	{
		GetWorldTimerManager().SetTimer(PulseTimerHandle, this, &AItem::ResetPulseTimer, PulseCurveTime);
	}
}

void AItem::UpdatePulse()
{
	float ElapsedTime = 0.f;
	FVector CurveValue = FVector::ZeroVector;
	switch (ItemState)
	{
	case EItemState::EIS_CanPickup:
		if(PulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimerHandle);
			CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	case EItemState::EIS_EquipInterping:
		if(InterpPulseCurve)
		{
			ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimerHandle);
			CurveValue = InterpPulseCurve->GetVectorValue(ElapsedTime);
		}
		break;
	}

	if(DynamicMI)
	{
		DynamicMI->SetScalarParameterValue(TEXT("GlowAmount"), CurveValue.X * GlowAmount);
		DynamicMI->SetScalarParameterValue(TEXT("FresnelExponent"), CurveValue.Y * FresnelExponent);
		DynamicMI->SetScalarParameterValue(TEXT("FresnelFraction"), CurveValue.Z * FresnelFraction);
	}
	
}