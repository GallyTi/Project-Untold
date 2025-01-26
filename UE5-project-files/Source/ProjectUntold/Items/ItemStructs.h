#pragma once
#include "CoreMinimal.h"
#include "ItemStructs.generated.h"

USTRUCT(BlueprintType)
struct PROJECTUNTOLD_API FItemData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	int32 ItemId = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	UTexture2D* ItemIcon = nullptr;

	// Define equality operator
	bool operator==(const FItemData& Other) const
	{
		return ItemId == Other.ItemId && Quantity == Other.Quantity;
	}
};
