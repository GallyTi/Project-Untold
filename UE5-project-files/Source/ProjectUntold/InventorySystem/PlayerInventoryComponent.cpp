#include "PlayerInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "InventoryService.h"
#include "../ProjectUntoldPlayerController.h" // dôležité pre cast
#include "InventoryWidget.h"

UPlayerInventoryComponent::UPlayerInventoryComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsReplicatedByDefault(true);
}

void UPlayerInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwnerRole() == ROLE_Authority)
    {
        // Vytvorenie inštancie InventoryService
        InventoryServiceRef = NewObject<UInventoryService>(this, UInventoryService::StaticClass());
        if (InventoryServiceRef)
        {
            InventoryServiceRef->OwnerInventoryComp = this;
        }
    }
}

void UPlayerInventoryComponent::OnRep_Inventory()
{    
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;

    APlayerController* PC = Cast<APlayerController>(OwnerActor->GetInstigatorController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_Inventory: No valid PlayerController found."));
        return;
    }

    // Skúste castnúť na AProjectUntoldPlayerController
    AProjectUntoldPlayerController* MyPC = Cast<AProjectUntoldPlayerController>(PC);
    if (MyPC && MyPC->InventoryWidget)
    {
        // Zavoláme update UI
        MyPC->InventoryWidget->RefreshInventoryUI(InventoryItems);
    }
}

void UPlayerInventoryComponent::ServerFetchInventoryFromBackend_Implementation()
{
    if (!InventoryServiceRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryServiceRef is null, cannot fetch inventory."));
        return;
    }
    InventoryServiceRef->GetInventory();
}

void UPlayerInventoryComponent::ServerUseItem_Implementation(int32 ItemId)
{
    // Nájsť item
    FItemData* FoundItem = InventoryItems.FindByPredicate([ItemId](const FItemData& Data){
        return Data.ItemId == ItemId;
    });
    if (!FoundItem || FoundItem->Quantity <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ServerUseItem: Item not found or quantity=0, ID=%d"), ItemId);
        return;
    }

    // Zavoláme Node.js => /inventory/use-item
    if (InventoryServiceRef)
    {
        InventoryServiceRef->UseItem(ItemId);
    }

    // Optimisticky zmenšíme quantity
    FoundItem->Quantity -= 1;
    if (FoundItem->Quantity <= 0)
    {
        InventoryItems.RemoveSingle(*FoundItem);
    }

    // Zavolá OnRep_Inventory() na klientoch (owner) = update UI
    OnRep_Inventory();
}

void UPlayerInventoryComponent::SetInventoryItems(const TArray<FItemData>& NewItems)
{
    InventoryItems = NewItems;
    // OnRep_Inventory() sa zavolá automaticky
}

void UPlayerInventoryComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Zabezpečíme, že InventoryItems pôjde len ownerovi
    DOREPLIFETIME_CONDITION(UPlayerInventoryComponent, InventoryItems, COND_OwnerOnly);
}

void UPlayerInventoryComponent::ServerAddItem_Implementation(int32 ItemId, int32 Quantity)
{
    if (!InventoryServiceRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("No InventoryServiceRef, cannot add item."));
        return;
    }
    InventoryServiceRef->AddItem(ItemId, Quantity);
}