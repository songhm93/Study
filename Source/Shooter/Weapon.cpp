#include "Weapon.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = true;
    ThrowWeaponTime = 0.7f;
    bFalling = false;
    Ammo = 0;
    WeaponType = EWeaponType::EWT_SubmachineGun;
    AmmoType = EAmmoType::EAT_9mm;
    ReloadMontageSection = FName(TEXT("Reload SMG"));
    MagazineCapacity = 30;
    ClipBoneName = FName(TEXT("smg_clip"));
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if(GetItemState() == EItemState::EIS_Falling && bFalling)
    {
        const FRotator MeshRotation = FRotator(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
        GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void AWeapon::ThrowWeaponTimerStart()
{
    FRotator MeshRotation = FRotator(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

    const FVector MeshForward = GetItemMesh()->GetForwardVector();
    const FVector MeshRight = GetItemMesh()->GetRightVector();
    FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward); //MeshForward 축으로 -20만큼 회전

    float RandomRotation = FMath::FRandRange(0.f, 30.f);
    ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));

    ImpulseDirection *= 20'000.f;
    GetItemMesh()->AddImpulse(ImpulseDirection);

    bFalling = true;
    GetWorldTimerManager().SetTimer(ThrowWeaponTimerHandle, this, &ThisClass::StopFalling, ThrowWeaponTime);

    EnableGlowMaterial();

}

void AWeapon::StopFalling()
{
    bFalling = false;
    SetItemState(EItemState::EIS_CanPickup);
    StartPulseTimer();
}

void AWeapon::DecrementAmmo()
{
    if(Ammo - 1 <= 0) Ammo = 0;
    else --Ammo;
    
}

void AWeapon::ReloadAmmo(int32 Amount)
{
    checkf(Ammo + Amount <= MagazineCapacity, TEXT("용량 초과"));
    Ammo += Amount;
}

bool AWeapon::ClipIsFull()
{
    return Ammo >= MagazineCapacity;
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    const FString Path = FString(TEXT("/Game/_Game/DataTable/WeaponDataTable.WeaponDataTable"));
    UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path));

    if(WeaponTableObject)
    {
        FWeaponDataTable* WeaponDataRow = nullptr;
        switch (WeaponType)
        {
            case EWeaponType::EWT_SubmachineGun:
                WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));
                break;
            case EWeaponType::EWT_AssaultRifle:
                WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
                break;
        }

        if(WeaponDataRow)
        {
            AmmoType = WeaponDataRow->AmmoType;
            Ammo = WeaponDataRow->WeaponAmmo;
            MagazineCapacity = WeaponDataRow->MagazineCapacity;
            SetPickupSound(WeaponDataRow->PickupSound);
            SetEquipSound(WeaponDataRow->EquipSound);
            GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
            SetItemName(WeaponDataRow->ItemName);
            SetIconItem(WeaponDataRow->InventoryIcon); //데이터테이블에 저장한 텍스처를 이렇게 Set하는게 되는건 신기하다.
            SetAmmoItem(WeaponDataRow->AmmoIcon); 

            SetMI(WeaponDataRow->MI);
            PrevMaterialIdx = GetMaterialIdx();
            GetItemMesh()->SetMaterial(PrevMaterialIdx, nullptr);
            SetMaterialIdx(WeaponDataRow->MaterialIdx);
        }

        if(GetMI())
        {
            SetDynamicMI(UMaterialInstanceDynamic::Create(GetMI(), this));
            GetDynamicMI()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
            GetItemMesh()->SetMaterial(GetMaterialIdx(), GetDynamicMI());

            EnableGlowMaterial();
        }
    }

}