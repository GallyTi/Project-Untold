#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "VoiceChatResult.h"
#include "../InventorySystem/InventoryService.h"
#include "EOS_GameInstance.generated.h"

/**
 * Delegate to notify when JWT token is set (optional).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJWTTokenSet);

class UHealthConnectNetworkManager;

/**
 * Custom GameInstance class for EOS features
 */
UCLASS()
class PROJECTUNTOLD_API UEOS_GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    /** Called on engine startup */
    virtual void Init() override;

    /** Delegate broadcast when the JWT token is set */
    FOnJWTTokenSet OnJWTTokenSet;

    /** Handler called internally when JWT token is set */
    UFUNCTION()
    void HandleJWTTokenSet();
    
    /** Pointer to your custom network manager */
    UPROPERTY()
    UHealthConnectNetworkManager* HealthConnectNetworkManager;
    
    /** Example service for handling inventory */
    UPROPERTY()
    UInventoryService* InventoryService;

    UHealthConnectNetworkManager* GetHealthConnectNetworkManager() const { return HealthConnectNetworkManager; }

    /* --------------------
     * EOS / Login Methods
     * -------------------- */
    
    UFUNCTION(BlueprintCallable, Category="EOS Function")
    void LoginWithEOS(const FString& AuthToken, const FString& AccountId, const FString& ProductUserId);

    /** Internal function to parse AuthToken/PUID after login success */
    void GetEOSAuthTokenAndPUID(TSharedPtr<const FUniqueNetId> UserId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="EOS Function")
    FString GetPlayerUsername();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="EOS Function")
    bool BLoggedInStatus();

    /* --------------------
     * Session Creation / Destruction
     * -------------------- */
    
    UFUNCTION(BlueprintCallable, Category="EOS Function")
    void CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPublicConnections);

    UFUNCTION(BlueprintCallable, Category="EOS Function")
    void FindSessionAndJoin();
	
    void JoinSessionBySearchResult(const FOnlineSessionSearchResult& SearchResult);

    UFUNCTION(BlueprintCallable, Category="EOS Function")
    void DestroySession();

    /** Level name (map) to open after session creation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EOS Variables")
    FString OpenLevelText;

    /* --------------------
     * Session Callbacks
     * -------------------- */
    
    void LoginWithEOS_Return(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error);
    void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);
    void OnFindSessionCompleted(bool bWasSuccess);
    void OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful);
    void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    /* --------------------
     * Voice Chat Methods
     * -------------------- */
    
    void InitializeVoiceChat();
    void ConnectVoiceChat();
    void JoinVoiceChannel(const FString& ChannelName);
    void OnVoiceChatConnected(const FString& PlayerName, const FVoiceChatResult& Result);
    void OnVoiceChatChannelJoined(const FString& ChannelName, const FVoiceChatResult& Result);

    /* --------------------
     * Stats Methods
     * -------------------- */
    
    void UpdatePlayerStat(const FString& StatName, int32 Value);
    void OnStatsUpdated(const FOnlineError& Result);
    void QueryPlayerStats();
    void OnStatsQueried(
        const FOnlineError& Result,
        const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStats
    );

    /* --------------------
     * User Data (Cloud Saves)
     * -------------------- */
    
    void SavePlayerData(const FString& FileName, const TArray<uint8>& Data);
    void OnWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);
    void LoadPlayerData(const FString& FileName);
    void OnReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);

    /* --------------------
     * JWT Handling
     * -------------------- */
    
    void OnLoginSuccess(const FString& JWTToken);
    void DelayedFetchActivityData();

    /* --------------------
     * Example Gameplay Var
     * -------------------- */
    
    void SetPlayerStamina(int32 NewStamina);

private:
    /** Session search object */
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    /** Cached authentication data */
    FString CachedAuthToken;
    FString CachedPUID;
    FString AccountId;
    FString ProductUserId;

    /** Timer handle for delayed calls */
    FTimerHandle FetchActivityTimerHandle;

    /** Example gameplay variable */
    int32 PlayerStamina;
};
