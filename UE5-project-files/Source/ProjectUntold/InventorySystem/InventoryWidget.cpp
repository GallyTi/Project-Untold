#include "InventoryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "InventoryItemWidget.h"
#include "../Items/ItemMeta.h"
#include "../Items/ItemStructs.h"
#include "Components/ScrollBox.h"
#include "InventoryItemWidget.h"  // trieda pre widget jedného itemu (budeme ju potrebovať)
#include "Blueprint/UserWidget.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Tu prípadne môžeme spraviť nejakú inicializáciu.
	// Napr. vyčistiť ItemsContainer -> ItemsContainer->ClearChildren();
}

void UInventoryWidget::RefreshInventoryUI(const TArray<FItemData>& Items)
{
	if (!ItemsContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryWidget::RefreshInventoryUI - ItemsContainer is null!"));
		return;
	}

	// 1) Najprv zmažeme staré itemy
	ItemsContainer->ClearChildren();

	// 2) Pre každý FItemData vytvoríme widget
	for (const FItemData& Item : Items)
	{
		if (!InventoryItemWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("UInventoryWidget::RefreshInventoryUI - InventoryItemWidgetClass is null!"));
			return;
		}

		UUserWidget* NewItemWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryItemWidgetClass);
		if (!NewItemWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("UInventoryWidget::RefreshInventoryUI - failed to create item widget"));
			continue;
		}

		// 3) Nastavíme item data
		UInventoryItemWidget* CastedWidget = Cast<UInventoryItemWidget>(NewItemWidget);
		if (CastedWidget)
		{
			CastedWidget->SetItemData(Item);
		}

		// 4) Pridáme do ScrollBoxu
		ItemsContainer->AddChild(NewItemWidget);
	}
}