#include "EOS_GameInstance.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "../BackendServices/HealthConnectNetworkManager.h"
#include "VoiceChat.h"
#include "Engine/World.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemTypes.h"

class IVoiceChat;

void UEOS_GameInstance::Init()
{
    Super::Init();

    // Create and store a reference to your custom network manager
    HealthConnectNetworkManager = NewObject<UHealthConnectNetworkManager>(this);
    if (HealthConnectNetworkManager)
    {
        UE_LOG(LogTemp, Display, TEXT("HealthConnectNetworkManager initialized successfully."));
        
        // Properly bind the delegate to handle JWT token set
        HealthConnectNetworkManager->OnJWTTokenSet.AddDynamic(this, &UEOS_GameInstance::HandleJWTTokenSet);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize HealthConnectNetworkManager."));
    }
}

void UEOS_GameInstance::HandleJWTTokenSet()
{
    UE_LOG(LogTemp, Display, TEXT("JWT Token set, preparing to call FetchAllActivities."));

    // Wait 3 seconds and then call DelayedFetchActivityData
    GetWorld()->GetTimerManager().SetTimer(
        FetchActivityTimerHandle,
        this,
        &UEOS_GameInstance::DelayedFetchActivityData,
        3.0f,
        false
    );
}

/* ------------------------------------------------
 * Login / Auth
 * -----------------------------------------------*/
void UEOS_GameInstance::LoginWithEOS(const FString& AuthToken, const FString& InAccountId, const FString& InProductUserId)
{
    UE_LOG(LogTemp, Display, TEXT("Starting LoginWithEOS. AccountId: %s, ProductUserId: %s"), *InAccountId, *InProductUserId);

    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (!SubsystemRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Online Subsystem"));
        return;
    }

    IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface();
    if (!IdentityPointerRef.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Identity Interface"));
        return;
    }

    // Store these if needed for later
    AccountId = InAccountId;
    ProductUserId = InProductUserId;

    // Prepare the account credentials for login
    FOnlineAccountCredentials AccountDetails;
    AccountDetails.Id = InAccountId;
    AccountDetails.Token = AuthToken;
    AccountDetails.Type = TEXT("accountportal"); // e.g. "accountportal", "password", etc.

    // Bind the delegate for login completion
    IdentityPointerRef->AddOnLoginCompleteDelegate_Handle(
        0,
        FOnLoginCompleteDelegate::CreateUObject(this, &UEOS_GameInstance::LoginWithEOS_Return)
    );

    // Start the login process
    IdentityPointerRef->Login(0, AccountDetails);
}

void UEOS_GameInstance::LoginWithEOS_Return(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("Login Success"));

        // Validate the returned user ID
        if (!UserId.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Login successful but UserId is invalid."));
            return;
        }

        // Extract Auth Token and PUID
        TSharedPtr<const FUniqueNetId> SharedUserId = StaticCastSharedRef<const FUniqueNetId>(UserId.AsShared());
        if (!SharedUserId.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("SharedUserId is invalid after successful login."));
            return;
        }
        GetEOSAuthTokenAndPUID(SharedUserId);

        // Optionally connect to voice chat
        ConnectVoiceChat();

        // If we already have a JWT from our HealthConnectNetworkManager, handle that
        if (HealthConnectNetworkManager && !HealthConnectNetworkManager->GetJWTToken().IsEmpty())
        {
            OnLoginSuccess(HealthConnectNetworkManager->GetJWTToken());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("JWT Token is not set in HealthConnectNetworkManager after login."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Login failed, reason: %s"), *Error);
    }
}

void UEOS_GameInstance::GetEOSAuthTokenAndPUID(TSharedPtr<const FUniqueNetId> UserId)
{
    if (!UserId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid UserId passed to GetEOSAuthTokenAndPUID"));
        return;
    }

    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (!SubsystemRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Online Subsystem"));
        return;
    }

    IOnlineIdentityPtr IdentityInterface = SubsystemRef->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Identity Interface"));
        return;
    }

    FString AuthToken = IdentityInterface->GetAuthToken(0);
    if (AuthToken.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("AuthToken is empty."));
        return;
    }

    // Example of splitting the CompositeId into AccountId and ProductUserId
    FString CompositeId = UserId->ToString();
    FString LocalAccountId;
    FString LocalProductUserId;
    if (CompositeId.Split(TEXT("|"), &LocalAccountId, &LocalProductUserId))
    {
        UE_LOG(LogTemp, Display, TEXT("Extracted AccountId: %s, ProductUserId: %s"), *LocalAccountId, *LocalProductUserId);
    }
    else
    {
        LocalAccountId = CompositeId;
        LocalProductUserId = TEXT("");
        UE_LOG(LogTemp, Warning, TEXT("CompositeId does not contain '|'. Using entire ID as AccountId: %s"), *LocalAccountId);
    }

    // Pass data to your backend network manager (if that is your workflow)
    if (HealthConnectNetworkManager)
    {
        HealthConnectNetworkManager->LoginWithEOS(AuthToken, LocalAccountId, LocalProductUserId);
    }
}

FString UEOS_GameInstance::GetPlayerUsername()
{
    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (SubsystemRef)
    {
        IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface();
        if (IdentityPointerRef.IsValid() &&
            IdentityPointerRef->GetLoginStatus(0) == ELoginStatus::LoggedIn)
        {
            return IdentityPointerRef->GetPlayerNickname(0);
        }
    }
    return FString();
}

bool UEOS_GameInstance::BLoggedInStatus()
{
    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (SubsystemRef)
    {
        IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface();
        if (IdentityPointerRef.IsValid())
        {
            return (IdentityPointerRef->GetLoginStatus(0) == ELoginStatus::LoggedIn);
        }
    }
    return false;
}

void UEOS_GameInstance::OnLoginSuccess(const FString& JWTToken)
{
    if (HealthConnectNetworkManager)
    {
        HealthConnectNetworkManager->SetJWTToken(JWTToken);
        UE_LOG(LogTemp, Display, TEXT("JWT Token set successfully: %s"), *JWTToken);

        // Example: after 5 seconds, call DelayedFetchActivityData
        GetWorld()->GetTimerManager().SetTimer(
            FetchActivityTimerHandle,
            this,
            &UEOS_GameInstance::DelayedFetchActivityData,
            5.0f,
            false
        );
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HealthConnectNetworkManager is null. Cannot set JWT Token or call FetchActivityData."));
    }
}

void UEOS_GameInstance::DelayedFetchActivityData()
{
    if (HealthConnectNetworkManager)
    {
        UE_LOG(LogTemp, Display, TEXT("Calling FetchActivityData after delay."));
        HealthConnectNetworkManager->FetchActivityData();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HealthConnectNetworkManager is null in DelayedFetchActivityData."));
    }
}

/* ------------------------------------------------
 * Session Creation / Destruction
 * -----------------------------------------------*/
void UEOS_GameInstance::CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPublicConnections)
{
    UE_LOG(LogTemp, Warning, TEXT("CreateEOSSession: Dedicated=%s, LAN=%s, Conns=%d"),
           bIsDedicatedServer ? TEXT("true") : TEXT("false"), 
           bIsLanServer       ? TEXT("true") : TEXT("false"), 
           NumberOfPublicConnections);

    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (!SubsystemRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Online Subsystem"));
        return;
    }

    IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();
    if (!SessionPtrRef.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Session Interface"));
        return;
    }

    // Destroy existing session if any
    FNamedOnlineSession* ExistingSession = SessionPtrRef->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Destroying existing session"));
        SessionPtrRef->DestroySession(NAME_GameSession);
    }

    // Build session settings
    FOnlineSessionSettings SessionCreationInfo;
    SessionCreationInfo.bIsDedicated = bIsDedicatedServer;
    SessionCreationInfo.bIsLANMatch = bIsLanServer;
    SessionCreationInfo.NumPublicConnections = NumberOfPublicConnections;
    SessionCreationInfo.bUsesPresence = true;
    SessionCreationInfo.bUseLobbiesIfAvailable = true;
    SessionCreationInfo.bShouldAdvertise = true;
    SessionCreationInfo.bAllowJoinInProgress = true;
    SessionCreationInfo.bAllowJoinViaPresence = true;
    SessionCreationInfo.bAllowInvites = true;
    SessionCreationInfo.Set(SEARCH_KEYWORDS, FString("MyEOSGameSession"), EOnlineDataAdvertisementType::ViaOnlineService);

    // Bind create-session completion
    SessionPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnCreateSessionCompleted);

    // Attempt to create the session
    bool bCreateSessionResult = SessionPtrRef->CreateSession(0, NAME_GameSession, SessionCreationInfo);
    UE_LOG(LogTemp, Warning, TEXT("CreateSession request: %s"),
           bCreateSessionResult ? TEXT("Success") : TEXT("Failed to trigger"));
}

void UEOS_GameInstance::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionCompleted for %s, result: %s"),
           *SessionName.ToString(),
           bWasSuccessful ? TEXT("Success") : TEXT("Failure"));

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("Session created successfully: %s"), *SessionName.ToString());
        // Travel the server to the designated map
        GetWorld()->ServerTravel(OpenLevelText);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session: %s"), *SessionName.ToString());
    }
}

void UEOS_GameInstance::DestroySession()
{
    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (!SubsystemRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Online Subsystem"));
        return;
    }

    IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();
    if (!SessionPtrRef.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Session Interface"));
        return;
    }

    // Bind the destroy callback
    SessionPtrRef->OnDestroySessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnDestroySessionCompleted);

    // Destroy the default (Game) session
    SessionPtrRef->DestroySession(NAME_GameSession);
}

void UEOS_GameInstance::OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionCompleted for %s, result: %s"),
           *SessionName.ToString(),
           bWasSuccessful ? TEXT("Success") : TEXT("Failure"));
}

/* ------------------------------------------------
 * Finding & Joining Sessions
 * -----------------------------------------------*/
void UEOS_GameInstance::FindSessionAndJoin()
{
    UE_LOG(LogTemp, Warning, TEXT("FindSessionAndJoin called"));

    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (!SubsystemRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Online Subsystem"));
        return;
    }

    IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();
    if (!SessionPtrRef.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Session Interface"));
        return;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = false;
    SessionSearch->MaxSearchResults = 10;
    // We want to search sessions that use presence
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    // Bind search completion
    SessionPtrRef->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnFindSessionCompleted);

    bool bFindSessionResult = SessionPtrRef->FindSessions(0, SessionSearch.ToSharedRef());
    UE_LOG(LogTemp, Warning, TEXT("FindSessions request: %s"),
           bFindSessionResult ? TEXT("Issued") : TEXT("Failed to start"));
}

void UEOS_GameInstance::OnFindSessionCompleted(bool bWasSuccess)
{
    UE_LOG(LogTemp, Warning, TEXT("OnFindSessionCompleted: %s"),
           bWasSuccess ? TEXT("Success") : TEXT("Failure"));

    if (bWasSuccess && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d sessions. Joining the first."),
               SessionSearch->SearchResults.Num());

        // Instead of overshadowing UGameInstance::JoinSession, call our custom function:
        JoinSessionBySearchResult(SessionSearch->SearchResults[0]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No sessions found or search failed"));
    }
}

/**
 * Our custom function to join a session result. 
 * NOTE: Not a UFUNCTION because FOnlineSessionSearchResult is not a USTRUCT.
 */
void UEOS_GameInstance::JoinSessionBySearchResult(const FOnlineSessionSearchResult& SearchResult)
{
    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (!SubsystemRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Online Subsystem"));
        return;
    }

    IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();
    if (!SessionPtrRef.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Session Interface"));
        return;
    }

    // Bind the callback for completion
    SessionPtrRef->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnJoinSessionCompleted);

    // Now we call the actual OnlineSubsystem 'JoinSession'
    SessionPtrRef->JoinSession(0, NAME_GameSession, SearchResult);
}

void UEOS_GameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted: %s => %s"),
           *SessionName.ToString(),
           (Result == EOnJoinSessionCompleteResult::Success) ? TEXT("Success") : TEXT("Failure"));

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join session: %s"), *SessionName.ToString());
        return;
    }

    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (!SubsystemRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Online Subsystem"));
        return;
    }

    IOnlineSessionPtr SessionPtrRef = SubsystemRef->GetSessionInterface();
    if (!SessionPtrRef.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Session Interface"));
        return;
    }

    FString ConnectString;
    if (SessionPtrRef->GetResolvedConnectString(SessionName, ConnectString))
    {
        APlayerController* PlayerController = GetFirstLocalPlayerController();
        if (PlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("ClientTravel to: %s"), *ConnectString);
            PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get connect string for session: %s"), *SessionName.ToString());
    }
}

/* ------------------------------------------------
 * Voice Chat
 * -----------------------------------------------*/
void UEOS_GameInstance::InitializeVoiceChat()
{
    IVoiceChat* VoiceChat = IVoiceChat::Get();
    if (VoiceChat)
    {
        VoiceChat->Initialize();
        UE_LOG(LogTemp, Log, TEXT("Voice Chat Initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get VoiceChat interface"));
    }
}

void UEOS_GameInstance::ConnectVoiceChat()
{
    IVoiceChat* VoiceChat = IVoiceChat::Get();
    if (VoiceChat && VoiceChat->IsInitialized())
    {
        FString PlayerName = GetPlayerUsername();

        ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
        FPlatformUserId PlatformId;
        if (LocalPlayer)
        {
            PlatformId = LocalPlayer->GetPlatformUserId();
        }

        VoiceChat->Login(
            PlatformId,
            PlayerName,
            /* Credentials */ FString(),
            FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &UEOS_GameInstance::OnVoiceChatConnected)
        );
    }
}

void UEOS_GameInstance::OnVoiceChatConnected(const FString& PlayerName, const FVoiceChatResult& Result)
{
    if (Result.IsSuccess())
    {
        UE_LOG(LogTemp, Log, TEXT("Voice Chat Connected as %s"), *PlayerName);
        JoinVoiceChannel(TEXT("GlobalChannel"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to connect Voice Chat: %s - %s"),
               *Result.ErrorCode,
               *Result.ErrorDesc);
    }
}

void UEOS_GameInstance::JoinVoiceChannel(const FString& ChannelName)
{
    IVoiceChat* VoiceChat = IVoiceChat::Get();
    if (VoiceChat)
    {
        VoiceChat->JoinChannel(
            ChannelName,
            /* ChannelCredentials */ FString(),
            EVoiceChatChannelType::NonPositional,
            FOnVoiceChatChannelJoinCompleteDelegate::CreateUObject(this, &UEOS_GameInstance::OnVoiceChatChannelJoined)
        );
    }
}

void UEOS_GameInstance::OnVoiceChatChannelJoined(const FString& ChannelName, const FVoiceChatResult& Result)
{
    if (Result.IsSuccess())
    {
        UE_LOG(LogTemp, Log, TEXT("Joined Voice Channel: %s"), *ChannelName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join Voice Channel: %s - %s"),
               *Result.ErrorCode,
               *Result.ErrorDesc);
    }
}

/* ------------------------------------------------
 * Stats
 * -----------------------------------------------*/
void UEOS_GameInstance::UpdatePlayerStat(const FString& StatName, int32 Value)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;
    
    IOnlineStatsPtr StatsInterface = Subsystem->GetStatsInterface();
    if (!StatsInterface.IsValid()) return;

    TSharedPtr<const FUniqueNetId> UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
    if (!UserId.IsValid()) return;

    // Build stats update
    FOnlineStatsUserUpdatedStats UpdatedStats(UserId.ToSharedRef());
    UpdatedStats.Stats.Add(
        StatName,
        FOnlineStatUpdate(Value, FOnlineStatUpdate::EOnlineStatModificationType::Sum)
    );

    TArray<FOnlineStatsUserUpdatedStats> UsersStats;
    UsersStats.Add(UpdatedStats);

    // Submit stats update, bind callback
    StatsInterface->UpdateStats(
        UserId.ToSharedRef(),
        UsersStats,
        FOnlineStatsUpdateStatsComplete::CreateUObject(this, &UEOS_GameInstance::OnStatsUpdated)
    );
}

void UEOS_GameInstance::OnStatsUpdated(const FOnlineError& Result)
{
    if (Result.WasSuccessful())
    {
        UE_LOG(LogTemp, Log, TEXT("Stats updated successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update stats: %s"), *Result.GetErrorMessage().ToString());
    }
}

void UEOS_GameInstance::QueryPlayerStats()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;
    
    IOnlineStatsPtr StatsInterface = Subsystem->GetStatsInterface();
    if (!StatsInterface.IsValid()) return;

    TSharedPtr<const FUniqueNetId> UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
    if (!UserId.IsValid()) return;

    FUniqueNetIdRef UserIdRef = UserId.ToSharedRef();

    // We want to query stats for these users:
    TArray<FUniqueNetIdRef> StatUsers;
    StatUsers.Add(UserIdRef);

    // We want these named stats:
    TArray<FString> StatNames;
    StatNames.Add(TEXT("Kills"));
    StatNames.Add(TEXT("Deaths"));

    // Query stats, bind callback
    StatsInterface->QueryStats(
        UserIdRef,                // Local user performing the query
        StatUsers,                // Which users we want stats for
        StatNames,                // Which stats we want
        FOnlineStatsQueryUsersStatsComplete::CreateUObject(this, &UEOS_GameInstance::OnStatsQueried)
    );
}

void UEOS_GameInstance::OnStatsQueried(const FOnlineError& Result, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStats)
{
    if (Result.WasSuccessful())
    {
        for (const TSharedRef<const FOnlineStatsUserStats>& UserStats : UsersStats)
        {
            for (const TPair<FString, FOnlineStatValue>& StatPair : UserStats->Stats)
            {
                UE_LOG(LogTemp, Log, TEXT("Stat %s => %s"),
                       *StatPair.Key,
                       *StatPair.Value.ToString());
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to query stats: %s"), *Result.GetErrorMessage().ToString());
    }
}

/* ------------------------------------------------
 * Cloud Saves (User Files)
 * -----------------------------------------------*/
void UEOS_GameInstance::SavePlayerData(const FString& FileName, const TArray<uint8>& Data)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;

    IOnlineUserCloudPtr CloudInterface = Subsystem->GetUserCloudInterface();
    if (!CloudInterface.IsValid()) return;

    TSharedPtr<const FUniqueNetId> UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
    if (!UserId.IsValid()) return;

    // Bind the delegate
    CloudInterface->AddOnWriteUserFileCompleteDelegate_Handle(
        FOnWriteUserFileCompleteDelegate::CreateUObject(this, &UEOS_GameInstance::OnWriteUserFileComplete)
    );

    // Make a non-const copy to pass in
    TArray<uint8> NonConstData = Data;
    CloudInterface->WriteUserFile(*UserId, FileName, NonConstData);
}

void UEOS_GameInstance::OnWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully saved player data to file: %s"), *FileName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save player data to file: %s"), *FileName);
    }
}

void UEOS_GameInstance::LoadPlayerData(const FString& FileName)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;
    
    IOnlineUserCloudPtr CloudInterface = Subsystem->GetUserCloudInterface();
    if (!CloudInterface.IsValid()) return;

    TSharedPtr<const FUniqueNetId> UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
    if (!UserId.IsValid()) return;

    // Bind the read complete delegate
    CloudInterface->AddOnReadUserFileCompleteDelegate_Handle(
        FOnReadUserFileCompleteDelegate::CreateUObject(this, &UEOS_GameInstance::OnReadUserFileComplete)
    );

    CloudInterface->ReadUserFile(*UserId, FileName);
}

void UEOS_GameInstance::OnReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    if (bWasSuccessful)
    {
        TArray<uint8> Data;
        IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
        if (!Subsystem) return;

        IOnlineUserCloudPtr CloudInterface = Subsystem->GetUserCloudInterface();
        if (CloudInterface.IsValid())
        {
            CloudInterface->GetFileContents(UserId, FileName, Data);
            // Here you can deserialize the data and load player inventory, etc.
        }
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded player data from file: %s"), *FileName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load player data from file: %s"), *FileName);
    }
}

/* ------------------------------------------------
 * Example Gameplay Variable
 * -----------------------------------------------*/
void UEOS_GameInstance::SetPlayerStamina(int32 NewStamina)
{
    PlayerStamina = NewStamina;
    UE_LOG(LogTemp, Display, TEXT("PlayerStamina set to: %d"), PlayerStamina);

    // Update any HUD/UI elements if needed
}
