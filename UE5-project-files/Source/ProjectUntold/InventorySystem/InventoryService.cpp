#include "InventoryService.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "../InventorySystem/PlayerInventoryComponent.h"
#include "ProjectUntold/Items/ItemStructs.h"

UInventoryService::UInventoryService()
{
}

void UInventoryService::GetInventory()
{
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("http://localhost:3000/inventory"), TEXT("GET"));
	Request->OnProcessRequestComplete().BindUObject(this, &UInventoryService::OnGetInventoryResponse);
	Request->ProcessRequest();
}

void UInventoryService::UseItem(int32 ItemId)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("itemId"), ItemId);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("http://localhost:3000/inventory/use-item"), TEXT("POST"));
	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &UInventoryService::OnUseItemResponse);
	Request->ProcessRequest();
}

void UInventoryService::OnGetInventoryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseContent = Response->GetContentAsString();

        // Parse JSON array of items -> TArray<FItemData>
        TArray<TSharedPtr<FJsonValue>> JsonItems;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
        if (FJsonSerializer::Deserialize(Reader, JsonItems))
        {
            TArray<FItemData> NewInventory;
            for (TSharedPtr<FJsonValue> ItemVal : JsonItems)
            {
                TSharedPtr<FJsonObject> ItemObj = ItemVal->AsObject();
                if (!ItemObj.IsValid()) continue;

                FItemData NewItem;
                NewItem.ItemId   = ItemObj->GetIntegerField(TEXT("itemId"));
                NewItem.Quantity = ItemObj->GetIntegerField(TEXT("quantity"));
                // prípadne aj AttackBonus = ...
                NewInventory.Add(NewItem);
            }

            // Zavoláme do nášho InventoryComponentu
            if (OwnerInventoryComp && OwnerInventoryComp->GetOwnerRole() == ROLE_Authority)
            {
                OwnerInventoryComp->SetInventoryItems(NewInventory);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("OnGetInventoryResponse: JSON parse error."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get inventory from backend."));
    }
}

void UInventoryService::OnUseItemResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        // Tu môžeme spraviť nejaký log, alebo aktualizovať Inventory z Node.js
        // Napr. znovu fetchneme, aby sme mali najčerstvejší stav:
        if (OwnerInventoryComp) { OwnerInventoryComp->ServerFetchInventoryFromBackend(); }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to use item in backend."));
    }
}

void UInventoryService::AddItem(int32 ItemId, int32 Quantity)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetNumberField(TEXT("itemId"), ItemId);
    JsonObject->SetNumberField(TEXT("quantity"), Quantity);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // Adresa: "http://localhost:3000/inventory/add-item"
    TSharedRef<IHttpRequest> Request = CreateRequest(BackendBaseUrl + TEXT("/inventory/add-item"), TEXT("POST"));
    Request->SetContentAsString(RequestBody);

    // Callback
    Request->OnProcessRequestComplete().BindUObject(this, &UInventoryService::OnAddItemResponse);
    Request->ProcessRequest();
}

void UInventoryService::OnAddItemResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        if (Response->GetResponseCode() == 200)
        {
            UE_LOG(LogTemp, Display, TEXT("AddItem success: %s"), *Response->GetContentAsString());

            // Napr. si hneď fetchneme nový zoznam inventára
            if (OwnerInventoryComp)
            {
                OwnerInventoryComp->ServerFetchInventoryFromBackend();
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AddItem failed: code %d, %s"),
                Response->GetResponseCode(), *Response->GetContentAsString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AddItem request failed, no valid response."));
    }
}