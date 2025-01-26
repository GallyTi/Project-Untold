#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UVerticalBox;
class UScrollBox;
class UInventoryItemWidget;  

UCLASS()
class PROJECTUNTOLD_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Funkcia, ktorou sa z UI refreshne zoznam itemov */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void RefreshInventoryUI(const TArray<FItemData>& Items);

protected:
	/** ScrollBox, do ktorého pridávame jednotlivé item widgety */
	UPROPERTY(meta=(BindWidget))
	UScrollBox* ItemsContainer;

	/** Typ widgetu, ktorý reprezentuje jeden item (napr. WBP_InventoryItem) */
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TSubclassOf<UUserWidget> InventoryItemWidgetClass;

	/** Override NativeConstruct - volá sa pri inicializácii */
	virtual void NativeConstruct() override;
};