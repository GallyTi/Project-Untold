#include "InventoryItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "../InventorySystem/PlayerInventoryComponent.h"
#include "GameFramework/PlayerState.h"
#include "../Items/ItemStructs.h"
#include "Components/Image.h"


void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemNameText)
	{
		ItemNameText->SetText(FText::FromString(FString::Printf(TEXT("ItemID = %d"), ItemData.ItemId)));
	}

	if (QuantityText)
	{
		QuantityText->SetText(FText::AsNumber(ItemData.Quantity));
	}

	if (RarityText)
	{
		RarityText->SetText(FText::AsNumber(ItemData.Quantity));
	}

	/*if (IconImage)
	{
		IconImage-> (FText::AsNumber(ItemData.Quantity));
	}*/

	if (UseButton)
	{
		UseButton->OnClicked.AddDynamic(this, &UInventoryItemWidget::OnUseButtonClicked);
	}
}

void UInventoryItemWidget::OnUseButtonClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	if (PC->PlayerState)
	{
		UPlayerInventoryComponent* InvComp = nullptr;

		AActor* OwnerAsActor = Cast<AActor>(PC->PlayerState);
		if (OwnerAsActor)
		{
			InvComp = OwnerAsActor->FindComponentByClass<UPlayerInventoryComponent>();
		}

		if (InvComp)
		{
			InvComp->ServerUseItem(ItemId);
		}
	}
}

void UInventoryItemWidget::SetItemData(const FItemData& InData)
{
	ItemData = InData;

	// Ak widget už je zkonštruovaný, rovno prepišeme texty
	if (ItemNameText)
	{
		// Napr. neskôr by ste mohli cez DataTable dohľadať reálny názov itemu
		ItemNameText->SetText(FText::FromString(FString::Printf(TEXT("ItemID = %d"), InData.ItemId)));
	}
	if (QuantityText)
	{
		QuantityText->SetText(FText::AsNumber(InData.Quantity));
	}
	if (ItemData.ItemIcon)
	{
		IconImage->SetBrushFromTexture(ItemData.ItemIcon);
	}
	else
	{
		// Vytvor Brush
		FSlateBrush WhiteBrush;
		WhiteBrush.TintColor = FSlateColor(FLinearColor::White);
		WhiteBrush.ImageSize = FVector2D(64.f, 64.f);
		IconImage->SetBrush(WhiteBrush);
	}
}