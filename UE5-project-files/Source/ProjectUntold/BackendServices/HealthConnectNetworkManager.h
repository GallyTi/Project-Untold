#pragma once

#include "CoreMinimal.h"
#include "BackendServiceBase.h"
#include "HealthConnectNetworkManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJWTTokenSet_HealthConnect);

UCLASS()
class PROJECTUNTOLD_API UHealthConnectNetworkManager : public UBackendServiceBase
{
	GENERATED_BODY()

public:
	UHealthConnectNetworkManager();

	// Delegate for JWT token events
	FOnJWTTokenSet_HealthConnect OnJWTTokenSet;

	// Getters and Setters
	FString GetJWTToken() const;
	void SetJWTToken(const FString& Token);

	// EOS Login
	void LoginWithEOS(const FString& AuthToken, const FString& AccountId, const FString& ProductUserId);

	// Fetch and submit data
	void FetchActivityData();
	void SubmitActivityData(const TArray<TSharedPtr<FJsonValue>>& Activities);

	void FetchActivityDataFromIPs(TArray<FString> IPs, int32 RetryCount, float RetryDelay);

	// Save player position
	void SavePlayerPosition(const FVector& Position);

	void FetchAllActivities();

	void RefreshToken();

	void FetchAggregatedActivityData();
	void OnFetchAggregatedActivitiesResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdatePlayerStamina(int32 TotalSteps);

private:
	FString BackendBaseUrl = TEXT("http://192.168.56.1:3000");

	// Member variables
	FString JWTToken;
	FString CachedAuthToken;
	FString CachedPUID;
	FString CachedAccountId;

	// Callback functions
	void OnEOSLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnFetchActivityDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnSubmitActivityResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnSavePositionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnFetchAllActivitiesResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// Token management
	virtual void AttemptTokenRefresh() override;
};
