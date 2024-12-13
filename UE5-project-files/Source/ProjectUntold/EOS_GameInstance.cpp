#include "EOS_GameInstance.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "BackendServices/HealthConnectNetworkManager.h"
#include "VoiceChat.h"
#include "Engine/World.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemTypes.h"


class IVoiceChat;


void UEOS_GameInstance::Init()
{
    Super::Init();

    HealthConnectNetworkManager = NewObject<UHealthConnectNetworkManager>(this);
    if (HealthConnectNetworkManager)
    {
        UE_LOG(LogTemp, Display, TEXT("HealthConnectNetworkManager initialized successfully."));
        
        // Properly bind the delegate
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

    // Wait 3 seconds and then call FetchAllActivities
    GetWorld()->GetTimerManager().SetTimer(
        FetchActivityTimerHandle,
        this,
        &UEOS_GameInstance::DelayedFetchActivityData,
        3.0f,
        false
    );
}

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

    // Store InAccountId and InProductUserId as class members if needed
    this->AccountId = InAccountId;
    this->ProductUserId = InProductUserId;

    // Prepare account credentials for login
    FOnlineAccountCredentials AccountDetails;
    AccountDetails.Id = InAccountId;
    AccountDetails.Token = AuthToken;
    AccountDetails.Type = TEXT("accountportal"); // Adjust as needed based on your login type

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
    // Log výsledok prihlásenia
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("Login Success"));
        
        // Kontrola platnosti UserId
        if (!UserId.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Login successful but UserId is invalid."));
            return;
        }

        // Získanie AuthToken a PUID
        TSharedPtr<const FUniqueNetId> SharedUserId = StaticCastSharedRef<const FUniqueNetId>(UserId.AsShared());
        if (!SharedUserId.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("SharedUserId is invalid after successful login."));
            return;
        }
        GetEOSAuthTokenAndPUID(SharedUserId);

        // Pripojenie k Voice Chatu (ak je potrebné)
        ConnectVoiceChat();

        // Zavolanie OnLoginSuccess s JWT Tokenom (ak je už získaný v HealthConnectNetworkManager)
        if (!HealthConnectNetworkManager->GetJWTToken().IsEmpty())
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

void UEOS_GameInstance::OnLoginSuccess(const FString& JWTToken)
{
    // Nastavenie JWT tokenu
    if (HealthConnectNetworkManager)
    {
        HealthConnectNetworkManager->SetJWTToken(JWTToken);
        UE_LOG(LogTemp, Display, TEXT("JWT Token set successfully: %s"), *JWTToken);

        // Nastav oneskorenie 5 sekúnd na volanie FetchActivityData
        UE_LOG(LogTemp, Display, TEXT("Setting up delayed call for FetchActivityData."));
        GetWorld()->GetTimerManager().SetTimer(
            FetchActivityTimerHandle, // Handle časovača
            this,                     // Objekt
            &UEOS_GameInstance::DelayedFetchActivityData, // Funkcia na zavolanie
            5.0f,                     // Oneskorenie (v sekundách)
            false                     // Opakovanie (false = len raz)
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

    // Declare local variables to avoid shadowing class members
    FString CompositeId = UserId->ToString();
    FString LocalAccountId;
    FString LocalProductUserId;

    // Split the CompositeId into AccountId and ProductUserId
    if (CompositeId.Split(TEXT("|"), &LocalAccountId, &LocalProductUserId))
    {
        UE_LOG(LogTemp, Display, TEXT("Extracted AccountId: %s, ProductUserId: %s"), *LocalAccountId, *LocalProductUserId);
    }
    else
    {
        LocalAccountId = CompositeId;
        LocalProductUserId = TEXT(""); // Set to empty if not available
        UE_LOG(LogTemp, Warning, TEXT("CompositeId does not contain '|'. Using entire ID as AccountId: %s"), *LocalAccountId);
    }

    // Pass the obtained data to LoginWithEOS
    HealthConnectNetworkManager->LoginWithEOS(AuthToken, LocalAccountId, LocalProductUserId);
}


FString UEOS_GameInstance::GetPlayerUsername()
{
    IOnlineSubsystem* SubsystemRef = IOnlineSubsystem::Get();
    if (SubsystemRef)
    {
        IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface();
        if (IdentityPointerRef.IsValid())
        {
            if (IdentityPointerRef->GetLoginStatus(0) == ELoginStatus::LoggedIn)
            {
                return IdentityPointerRef->GetPlayerNickname(0);
            }
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
            return IdentityPointerRef->GetLoginStatus(0) == ELoginStatus::LoggedIn;
        }
    }
    return false;
}

void UEOS_GameInstance::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionCompleted for session: %s, result: %s"), *SessionName.ToString(), 
           bWasSuccessful ? TEXT("Success") : TEXT("Failure"));

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("Session created successfully: %s"), *SessionName.ToString());
        GetWorld()->ServerTravel(OpenLevelText);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session: %s"), *SessionName.ToString());
    }
}

void UEOS_GameInstance::CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPublicConnections)
{
    UE_LOG(LogTemp, Warning, TEXT("CreateEOSSession called with DedicatedServer: %s, LANServer: %s, Connections: %d"), 
           bIsDedicatedServer ? TEXT("true") : TEXT("false"), 
           bIsLanServer ? TEXT("true") : TEXT("false"), 
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

    FNamedOnlineSession* ExistingSession = SessionPtrRef->GetNamedSession(NAME_GameSession);
    if (ExistingSession != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Destroying existing session"));
        SessionPtrRef->DestroySession(NAME_GameSession);
    }

    FOnlineSessionSettings SessionCreationInfo;
    SessionCreationInfo.bIsDedicated = bIsDedicatedServer;
    SessionCreationInfo.bAllowInvites = true;
    SessionCreationInfo.bIsLANMatch = bIsLanServer;
    SessionCreationInfo.NumPublicConnections = NumberOfPublicConnections;
    SessionCreationInfo.bUsesPresence = true;
    SessionCreationInfo.bUseLobbiesIfAvailable = true;
    SessionCreationInfo.bShouldAdvertise = true;
    SessionCreationInfo.bAllowJoinInProgress = true;
    SessionCreationInfo.bAllowJoinViaPresence = true;
    SessionCreationInfo.Set(SEARCH_KEYWORDS, FString("MyEOSGameSession"), EOnlineDataAdvertisementType::ViaOnlineService);

    SessionPtrRef->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnCreateSessionCompleted);

    bool bCreateSessionResult = SessionPtrRef->CreateSession(0, NAME_GameSession, SessionCreationInfo);
    UE_LOG(LogTemp, Warning, TEXT("Create session request sent with result: %s"), bCreateSessionResult ? TEXT("Success") : TEXT("Failure"));
}


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
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    SessionPtrRef->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnFindSessionCompleted);

    bool bFindSessionResult = SessionPtrRef->FindSessions(0, SessionSearch.ToSharedRef());
    UE_LOG(LogTemp, Warning, TEXT("Find session request sent with result: %s"), bFindSessionResult ? TEXT("Success") : TEXT("Failure"));
}

void UEOS_GameInstance::OnFindSessionCompleted(bool bWasSuccess)
{
    UE_LOG(LogTemp, Warning, TEXT("OnFindSessionCompleted called with result: %s"), bWasSuccess ? TEXT("Success") : TEXT("Failure"));

    if (bWasSuccess && SessionSearch->SearchResults.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d sessions, attempting to join first one"), SessionSearch->SearchResults.Num());
        JoinSession(SessionSearch->SearchResults[0]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No sessions found"));
    }
}

void UEOS_GameInstance::JoinSession(const FOnlineSessionSearchResult& SearchResult)
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

    SessionPtrRef->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnJoinSessionCompleted);
    SessionPtrRef->JoinSession(0, NAME_GameSession, SearchResult);
}

void UEOS_GameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionCompleted called for session: %s with result: %s"), 
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
            UE_LOG(LogTemp, Warning, TEXT("Joining session with connect string: %s"), *ConnectString);
            PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get connect string for session: %s"), *SessionName.ToString());
    }
}

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
        FPlatformUserId PlatformId = FPlatformUserId(); // Predvolený neplatný ID

        if (LocalPlayer)
        {
            PlatformId = LocalPlayer->GetPlatformUserId();
        }

        VoiceChat->Login(
            PlatformId,
            PlayerName,
            FString(), // Credentials
            FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &UEOS_GameInstance::OnVoiceChatConnected)
        );
    }
}

void UEOS_GameInstance::OnVoiceChatConnected(const FString& PlayerName, const FVoiceChatResult& Result)
{
    if (Result.IsSuccess())
    {
        UE_LOG(LogTemp, Log, TEXT("Voice Chat Connected as %s"), *PlayerName);
        this->JoinVoiceChannel("GlobalChannel");
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to connect Voice Chat: ErrorCode=%s, ErrorDesc=%s"), *Result.ErrorCode, *Result.ErrorDesc);
    }
}


void UEOS_GameInstance::JoinVoiceChannel(const FString& ChannelName)
{
    IVoiceChat* VoiceChat = IVoiceChat::Get();
    if (VoiceChat)
    {
        VoiceChat->JoinChannel(
            ChannelName,
            FString(), // ChannelCredentials
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
        UE_LOG(LogTemp, Error, TEXT("Failed to join Voice Channel: ErrorCode=%s, ErrorDesc=%s"), *Result.ErrorCode, *Result.ErrorDesc);
    }
}


void UEOS_GameInstance::UpdatePlayerStat(const FString& StatName, int32 Value)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineStatsPtr StatsInterface = Subsystem->GetStatsInterface();
        if (StatsInterface.IsValid())
        {
            TSharedPtr<const FUniqueNetId> UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
            if (UserId.IsValid())
            {
                FOnlineStatsUserUpdatedStats UpdatedStats(UserId.ToSharedRef());
                UpdatedStats.Stats.Add(StatName, FOnlineStatUpdate(Value, FOnlineStatUpdate::EOnlineStatModificationType::Sum));

                TArray<FOnlineStatsUserUpdatedStats> UsersStats;
                UsersStats.Add(UpdatedStats);

                StatsInterface->UpdateStats(
                    UserId.ToSharedRef(),
                    UsersStats,
                    FOnlineStatsUpdateStatsComplete::CreateUObject(this, &UEOS_GameInstance::OnStatsUpdated)
                );
            }
        }
    }
}

void UEOS_GameInstance::OnStatsUpdated(const FOnlineError& Result)
{
    if (Result.WasSuccessful())
    {
        UE_LOG(LogTemp, Log, TEXT("Stats updated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to update stats: %s"), *Result.GetErrorMessage().ToString());
    }
}

void UEOS_GameInstance::QueryPlayerStats()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineStatsPtr StatsInterface = Subsystem->GetStatsInterface();
        if (StatsInterface.IsValid())
        {
            TSharedPtr<const FUniqueNetId> UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
            if (UserId.IsValid())
            {
                FUniqueNetIdRef UserIdRef = UserId.ToSharedRef();

                TArray<FUniqueNetIdRef> StatUsers;
                StatUsers.Add(UserIdRef);

                TArray<FString> StatNames;
                StatNames.Add("Kills");
                StatNames.Add("Deaths");

                StatsInterface->QueryStats(
                    UserIdRef, // LocalUserId
                    StatUsers, // StatUsers
                    StatNames, // StatNames
                    FOnlineStatsQueryUsersStatsComplete::CreateUObject(this, &UEOS_GameInstance::OnStatsQueried)
                );
            }
        }
    }
}

void UEOS_GameInstance::OnStatsQueried(const FOnlineError& Result, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStats)
{
    if (Result.WasSuccessful())
    {
        for (const TSharedRef<const FOnlineStatsUserStats>& UserStats : UsersStats)
        {
            for (const TPair<FString, FOnlineStatValue>& Stat : UserStats->Stats)
            {
                UE_LOG(LogTemp, Log, TEXT("Stat %s: %s"), *Stat.Key, *Stat.Value.ToString());
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to query stats: %s"), *Result.GetErrorMessage().ToString());
    }
}

void UEOS_GameInstance::SavePlayerData(const FString& FileName, const TArray<uint8>& Data)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineUserCloudPtr CloudInterface = Subsystem->GetUserCloudInterface();
        if (CloudInterface.IsValid())
        {
            TSharedPtr<const FUniqueNetId> UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
            if (UserId.IsValid())
            {
                CloudInterface->AddOnWriteUserFileCompleteDelegate_Handle(
                    FOnWriteUserFileCompleteDelegate::CreateUObject(this, &UEOS_GameInstance::OnWriteUserFileComplete)
                );

                // Vytvorenie nekonštantnej kópie
                TArray<uint8> NonConstData = Data;

                CloudInterface->WriteUserFile(*UserId, FileName, NonConstData);
            }
        }
    }
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
    if (Subsystem)
    {
        IOnlineUserCloudPtr CloudInterface = Subsystem->GetUserCloudInterface();
        if (CloudInterface.IsValid())
        {
            TSharedPtr<const FUniqueNetId> UserId = Subsystem->GetIdentityInterface()->GetUniquePlayerId(0);
            if (UserId.IsValid())
            {
                CloudInterface->AddOnReadUserFileCompleteDelegate_Handle(
                    FOnReadUserFileCompleteDelegate::CreateUObject(this, &UEOS_GameInstance::OnReadUserFileComplete)
                );

                CloudInterface->ReadUserFile(*UserId, FileName);
            }
        }
    }
}

void UEOS_GameInstance::OnReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    if (bWasSuccessful)
    {
        TArray<uint8> Data;
        IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
        IOnlineUserCloudPtr CloudInterface = Subsystem->GetUserCloudInterface();
        if (CloudInterface.IsValid())
        {
            CloudInterface->GetFileContents(UserId, FileName, Data);
            // Tu môžete de-serializovať dáta a načítať inventár hráča
        }
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded player data from file: %s"), *FileName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load player data from file: %s"), *FileName);
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

    SessionPtrRef->OnDestroySessionCompleteDelegates.AddUObject(this, &UEOS_GameInstance::OnDestroySessionCompleted);
    SessionPtrRef->DestroySession(NAME_GameSession);
}

void UEOS_GameInstance::OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionCompleted for session: %s, result: %s"),
        *SessionName.ToString(),
        bWasSuccessful ? TEXT("Success") : TEXT("Failure"));
}

void UEOS_GameInstance::SetPlayerStamina(int32 NewStamina)
{
    PlayerStamina = NewStamina;

    // Update any HUD or UI elements if necessary
    // For example:
    // UpdateHUDStamina(PlayerStamina);

    UE_LOG(LogTemp, Display, TEXT("PlayerStamina set to: %d"), PlayerStamina);
}