#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Items/ItemStructs.h"   
#include "InventoryItemWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;   // ak chcete ikony

UCLASS()
class PROJECTUNTOLD_API UInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Nastavíme FItemData (id, quantity, atď.) */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetItemData(const FItemData& InData);
	
	// Naplníme z externého kódu
	UPROPERTY(BlueprintReadOnly, Category="Item Data")
	int32 ItemId;

	UPROPERTY(BlueprintReadOnly, Category="Item Data")
	int32 Quantity;

	UPROPERTY(BlueprintReadOnly, Category="Item Data")
	FString ItemName;

	UPROPERTY(BlueprintReadOnly, Category="Item Data")
	UTexture2D* ItemIcon;

	UPROPERTY(BlueprintReadOnly, Category="Item Data")
	FString ItemRarity;

	UPROPERTY(BlueprintReadOnly, Category="Item Data")
	FString ItemDescription;

protected:
	// Referencie na widgetové komponenty
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* QuantityText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* RarityText;

	UPROPERTY(meta=(BindWidget))
	UImage* IconImage;

	UPROPERTY(meta=(BindWidget))
	UButton* UseButton;

	/** Volané pri inicializácii widgetu */
	virtual void NativeConstruct() override;

	/** Premenná, kde si držíme info o iteme. */
	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	FItemData ItemData;

private:
	UFUNCTION()
	void OnUseButtonClicked();
};
