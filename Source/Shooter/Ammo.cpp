#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"

AAmmo::AAmmo()
{
    AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
    RootComponent = AmmoMesh;

    GetCollisionBox()->SetupAttachment(GetRootComponent());
    GetPickupWidget()->SetupAttachment(GetRootComponent());
    GetAreaSphere()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(RootComponent);
	AmmoCollisionSphere->SetSphereRadius(50.f);
	
	//AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAmmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAmmo::BeginPlay()
{
    Super::BeginPlay();
	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::AmmoSphereOverlap);
}

void AAmmo::SetItemProperties(EItemState State)
{
    Super::SetItemProperties(State);

    switch(State)
	{
		case EItemState::EIS_CanPickup:
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		case EItemState::EIS_Equipped:
			GetPickupWidget()->SetVisibility(false);
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		case EItemState::EIS_Falling:
			AmmoMesh->SetSimulatePhysics(true);
			AmmoMesh->SetEnableGravity(true);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

			break;
		case EItemState::EIS_EquipInterping:
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			break;
		case EItemState::EIS_Pickedup:
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(false);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			break;
	}
}

void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		AShooterCharacter* OverlappedCharacter = Cast<AShooterCharacter>(OtherActor);
		if(OverlappedCharacter)
		{
			StartItemCurve(OverlappedCharacter);
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}
