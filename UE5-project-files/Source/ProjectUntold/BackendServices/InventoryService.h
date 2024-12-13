#pragma once

#include "CoreMinimal.h"
#include "BackendServiceBase.h"
#include "InventoryService.generated.h"

UCLASS()
class PROJECTUNTOLD_API UInventoryService : public UBackendServiceBase
{
	GENERATED_BODY()

public:
	UInventoryService();

	void GetInventory();
	void UseItem(int32 ItemId);

private:
	void OnGetInventoryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnUseItemResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
