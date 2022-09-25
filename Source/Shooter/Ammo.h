#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

class USphereComponent;

UCLASS()
class SHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()
public:
	AAmmo();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	virtual void SetItemProperties(EItemState State) override;
	UFUNCTION()
	void AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ammo", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIconTexture;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ammo", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AmmoCollisionSphere;

public: //get
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

public: //set

public:
};
