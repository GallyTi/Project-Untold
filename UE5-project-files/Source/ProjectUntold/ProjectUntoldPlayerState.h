#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ProjectUntoldPlayerState.generated.h"

UCLASS()
class PROJECTUNTOLD_API AProjectUntoldPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AProjectUntoldPlayerState();

protected:
	virtual void BeginPlay() override;

	/** Tu deklarujete prepis replikácie */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	class UPlayerInventoryComponent* InventoryComp;
};
