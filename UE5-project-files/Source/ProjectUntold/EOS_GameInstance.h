#pragma once

#include "CoreMinimal.h"
#include "VoiceChatResult.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "BackendServices/InventoryService.h"
#include "EOS_GameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJWTTokenSet);

UCLASS()
class PROJECTUNTOLD_API UEOS_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	FOnJWTTokenSet OnJWTTokenSet;

	UFUNCTION()
	void HandleJWTTokenSet();
	
	UPROPERTY()
	UHealthConnectNetworkManager* HealthConnectNetworkManager;
	
	UPROPERTY()
	UInventoryService* InventoryService;

	UHealthConnectNetworkManager* GetHealthConnectNetworkManager() const { return HealthConnectNetworkManager; }
	
	UFUNCTION(BlueprintCallable, Category="EOS Function")
	void LoginWithEOS(const FString& AuthToken, const FString& AccountId, const FString& ProductUserId);
	void GetEOSAuthTokenAndPUID(TSharedPtr<const FUniqueNetId> UserId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="EOS Function")
	FString GetPlayerUsername();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="EOS Function")
	bool BLoggedInStatus();

	UFUNCTION(BlueprintCallable, Category="EOS Function")
	void CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPublicConnections);

	UFUNCTION(BlueprintCallable, Category="EOS Function")
	void FindSessionAndJoin();

	void JoinSession(const FOnlineSessionSearchResult& SearchResult);

	UFUNCTION(BlueprintCallable, Category="EOS Function")
	void DestroySession();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EOS Variables")
	FString OpenLevelText;

	void LoginWithEOS_Return(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error);
	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);
	void OnFindSessionCompleted(bool bWasSuccess);
	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void InitializeVoiceChat();
	void ConnectVoiceChat();
	void JoinVoiceChannel(const FString& ChannelName);
	void OnVoiceChatConnected(const FString& PlayerName, const FVoiceChatResult& Result);
	void OnVoiceChatChannelJoined(const FString& ChannelName, const FVoiceChatResult& Result);

	void UpdatePlayerStat(const FString& StatName, int32 Value);
	void OnStatsUpdated(const FOnlineError& Result);
	void QueryPlayerStats();
	void OnStatsQueried(
		const FOnlineError& Result,
		const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStats
	);

	void SavePlayerData(const FString& FileName, const TArray<uint8>& Data);
	void OnWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);
	void LoadPlayerData(const FString& FileName);
	void OnReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);

	void OnLoginSuccess(const FString& JWTToken);

	void DelayedFetchActivityData();

	void SetPlayerStamina(int32 NewStamina);
	
private:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FString CachedAuthToken;
	FString CachedPUID;
	FString AccountId;
	FString ProductUserId;

	FTimerHandle FetchActivityTimerHandle;

	int32 PlayerStamina;

};
