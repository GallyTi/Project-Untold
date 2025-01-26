#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "../Items/ItemStructs.h"
#include "PlayerInventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DefaultToInstanced, Blueprintable)
class PROJECTUNTOLD_API UPlayerInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerInventoryComponent(const FObjectInitializer& ObjectInitializer);

	// Replikovaný array itemov
	UPROPERTY(ReplicatedUsing=OnRep_Inventory)
	TArray<FItemData> InventoryItems;

protected:
	virtual void BeginPlay() override;

	// Tento callback sa zavolá na klientoch, keď sa InventoryItems zmení
	UFUNCTION()
	void OnRep_Inventory();

public:
	// Server RPC na fetch z Node.js
	UFUNCTION(Server, Reliable)
	void ServerFetchInventoryFromBackend();

	// Server RPC na použitie itemu
	UFUNCTION(Server, Reliable)
	void ServerUseItem(int32 ItemId);

	UFUNCTION(Server, Reliable)
	void ServerAddItem(int32 ItemId, int32 Quantity);

	// Pomocná funkcia na nastavenie itemov na serveri
	void SetInventoryItems(const TArray<FItemData>& NewItems);

	// reference na InventoryService
	UPROPERTY()
	class UInventoryService* InventoryServiceRef;

	// Override replikácie
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};
