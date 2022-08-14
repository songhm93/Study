#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"


class UUserWidget;

UCLASS()
class SHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterPlayerController();
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HUDOverlayClass;
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "Widgets", meta = (AllowPrivateAccess = "true"))
	UUserWidget* HUDOverlay;
};
