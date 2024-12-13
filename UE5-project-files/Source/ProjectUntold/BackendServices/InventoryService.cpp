#include "InventoryService.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

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
		// Spracovanie inventára
		FString ResponseContent = Response->GetContentAsString();
		// Parse JSON and update inventory
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get inventory"));
	}
}

void UInventoryService::OnUseItemResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		// Spracovanie odpovede po použití predmetu
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to use item"));
	}
}
