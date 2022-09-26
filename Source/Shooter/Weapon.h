#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"



class USoundCue;
class UWidgetComponent;
class UParticleSystem;

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* PickupSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MI;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIdx;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsMiddle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsBottom;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FireSound;
};

UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
protected:
	void StopFalling();

	virtual void OnConstruction(const FTransform& Transform) override;
private:
	FTimerHandle ThrowWeaponTimerHandle;
	float ThrowWeaponTime;
	bool bFalling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;
	int32 PrevMaterialIdx;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsMiddle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsLeft;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsRight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsBottom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsTop;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	float AutoFireRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	USoundCue* FireSound;

public: //get
	FORCEINLINE	 int32 GetAmmo() const { return Ammo;  }
	FORCEINLINE	 int32 GetMagazineCapacity() const { return MagazineCapacity;  }
	FORCEINLINE	 EWeaponType GetWeaponType() const { return WeaponType;  }
	FORCEINLINE	 EAmmoType GetAmmoType() const { return AmmoType;  }
	FORCEINLINE	 FName GetReloadMontageSection() const { return ReloadMontageSection;  }
	FORCEINLINE	 FName GetClipBoneName() const { return ClipBoneName;  }
	FORCEINLINE	 USoundCue* GetFireSound() const { return FireSound;  }
	FORCEINLINE	 UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash;  }
	FORCEINLINE	float GetAutoFireRate() const { return AutoFireRate;  }
public: //set
	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }
	FORCEINLINE void SetClipBoneName(FName Name) { ClipBoneName = Name;}
	FORCEINLINE void SetReloadMontageSection(FName Name) { ReloadMontageSection = Name; }
public:
	void ThrowWeaponTimerStart();
	void DecrementAmmo();
	void ReloadAmmo(int32 Amount);
	bool ClipIsFull();

};
