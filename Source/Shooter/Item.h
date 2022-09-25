#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"


UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_CanPickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_Pickedup UMETA(DisplayName = "Pickedup"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Ammo UMETA(DisplayName = "Ammo"),
	Weapon UMETA(DisplayName = "Weapon"),

	MAX UMETA(DisplayName ="DefaultMAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDS;
};

class UBoxComponent;
class UWidgetComponent;
class USphereComponent;
class UCurveFloat;
class UCurveVector;
class AShooterCharacter;
class USoundCue;
class UDataTable;

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:
	AItem();
	virtual void Tick(float DeltaTime) override;
	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	void DisableGlowMaterial();
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void SetActiveStars();
	virtual void SetItemProperties(EItemState State);
	void FinishInterping(); //보간 끝. 콜백함수
	void ItemInterp(float DeltaTime);
	
	virtual void InitCustomDepth();
	virtual void OnConstruction(const FTransform& Transform) override;
	void EnableGlowMaterial();
	void UpdatePulse();
	void StartPulseTimer();
	void ResetPulseTimer();
	FVector GetInterpLocation();
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* CollisionBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PickupWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AreaSphere;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount; //총알 등등의 양
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity; //희귀도. 위젯에 별 갯수.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemZCurve;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLoation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bInterping; //보간중일 땐 true
	FTimerHandle ItemInterpTimerHandle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime; //커브 지속시간, 타이머 시간.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	AShooterCharacter* Character;
	float ItemInterpX;
	float ItemInterpY;
	float InterpInitYawOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USoundCue* PickupSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USoundCue* EquipSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMI;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MI;
	bool bCanChangeCustomDepth;
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* PulseCurve;
	FTimerHandle PulseTimerHandle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;
	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount; //머티리얼 파라미터
	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent; //머티리얼 파라미터
	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelFraction; //머티리얼 파라미터

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* InterpPulseCurve; //MaterialInterpCurve
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIcon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	bool bInventoryFull;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocIdx;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
	UDataTable* ItemRarityDataTable;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;
public: //get
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget;}
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE UMaterialInstance* GetMI() const { return MI; }
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMI() const { return DynamicMI; }
	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }
	FORCEINLINE int32 GetMaterialIdx() const { return MaterialIndex; }
public: //set
	void SetItemState(EItemState State);
	void StartItemCurve(AShooterCharacter* Char);
	FORCEINLINE void SetPickupSound(USoundCue* Sound) { PickupSound = Sound; }
	FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }
	FORCEINLINE void SetEquipSound(USoundCue* Sound) { EquipSound = Sound; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	FORCEINLINE void SetInventoryFull(bool bFull) { bInventoryFull = bFull; }
	FORCEINLINE void SetIconItem(UTexture2D* Icon) { IconItem = Icon; }
	FORCEINLINE void SetAmmoItem(UTexture2D* Icon) { AmmoIcon = Icon; }
	FORCEINLINE void SetMI(UMaterialInstance* Instance) { MI = Instance; }
	FORCEINLINE void SetDynamicMI(UMaterialInstanceDynamic* Instance) { DynamicMI = Instance; }
	FORCEINLINE void SetMaterialIdx(int32 Index) { MaterialIndex = Index; }
};
