#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "ItemMeta.generated.h"

USTRUCT(BlueprintType)
struct FItemMeta : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Meno, zobrazovaný názov itemu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	FText DisplayName;

	// Ikona (UTexture2D) ktorá sa zobrazí v UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	UTexture2D* Icon;

	// Rarita (napr. "Common", "Rare", "Epic")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	FString Rarity;

	// Popis (tooltip alebo detailný text)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	FText Description;
};
