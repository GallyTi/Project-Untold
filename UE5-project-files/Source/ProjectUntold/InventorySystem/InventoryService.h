#pragma once

#include "CoreMinimal.h"
#include "../BackendServices/BackendServiceBase.h"
#include "InventoryService.generated.h"

UCLASS()
class PROJECTUNTOLD_API UInventoryService : public UBackendServiceBase
{
	GENERATED_BODY()

public:
	UInventoryService();

	// Napr. budeme v Blueprinte alebo C++ nastaviť "OwnerInventoryComp"
	// aby po úspešnom GET mohol zmeniť InventoryComp lokálne.
	UPROPERTY()
	class UPlayerInventoryComponent* OwnerInventoryComp;

	void GetInventory();
	void UseItem(int32 ItemId);


	UFUNCTION()
	void AddItem(int32 ItemId, int32 Quantity);
	

private:
	void OnGetInventoryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnUseItemResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnAddItemResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString BackendBaseUrl = TEXT("http://localhost:3000");
};

