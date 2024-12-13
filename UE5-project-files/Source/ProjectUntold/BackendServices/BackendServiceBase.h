#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "BackendServiceBase.generated.h"

class UHealthConnectNetworkManager;

UCLASS()
class PROJECTUNTOLD_API UBackendServiceBase : public UObject
{
	GENERATED_BODY()

public:
	UBackendServiceBase();

	void SetJWTToken(const FString& Token);

protected:
	FString JWTToken;

	// Pomocná funkcia na vytvorenie HTTP požiadavky s nastaveným JWT tokenom
	TSharedRef<IHttpRequest> CreateRequest(const FString& URL, const FString& Verb, float Timeout = 30.0f);

	// Callback na spracovanie základných HTTP odpovedí
	virtual void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UPROPERTY()
	UHealthConnectNetworkManager* AuthenticationService;

	UPROPERTY()
	UHealthConnectNetworkManager* HealthConnectNetworkManager;

	virtual void AttemptTokenRefresh();
};