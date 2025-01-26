#include "HealthConnectNetworkManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "pma/TypeDefs.h"
#include "../EOS/EOS_GameInstance.h"

UHealthConnectNetworkManager::UHealthConnectNetworkManager()
{
}

FString UHealthConnectNetworkManager::GetJWTToken() const
{
    return JWTToken;
}

void UHealthConnectNetworkManager::SetJWTToken(const FString& Token)
{
    JWTToken = Token;
    UE_LOG(LogTemp, Display, TEXT("JWT Token set successfully in HealthConnectNetworkManager: %s"), *JWTToken);

    // Zavolaj delegate po nastavení tokenu
    OnJWTTokenSet.Broadcast();
}

void UHealthConnectNetworkManager::LoginWithEOS(const FString& AuthToken, const FString& AccountId, const FString& ProductUserId)
{
    UE_LOG(LogTemp, Display, TEXT("Attempting to login with EOS. AccountId: %s, ProductUserId: %s"), *AccountId, *ProductUserId);

    CachedAuthToken = AuthToken;
    CachedAccountId = AccountId;
    CachedPUID = ProductUserId;

    // Include ProductUserId in the request body
    FString RequestBody = FString::Printf(TEXT("{\"authToken\":\"%s\",\"accountId\":\"%s\",\"productUserId\":\"%s\"}"), *AuthToken, *AccountId, *ProductUserId);

    TSharedRef<IHttpRequest> Request = CreateRequest(BackendBaseUrl + TEXT("/auth/eos-login"), TEXT("POST"));
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UHealthConnectNetworkManager::OnEOSLoginResponseReceived);
    Request->ProcessRequest();
}

void UHealthConnectNetworkManager::RefreshToken()
{
    if (!CachedAuthToken.IsEmpty() && !CachedPUID.IsEmpty() && !CachedAccountId.IsEmpty())
    {
        LoginWithEOS(CachedAuthToken, CachedAccountId, CachedPUID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot refresh token: AuthToken, PUID, or AccountId is missing."));
    }
}

void UHealthConnectNetworkManager::AttemptTokenRefresh()
{
    if (!CachedAuthToken.IsEmpty() && !CachedPUID.IsEmpty() && !CachedAccountId.IsEmpty())
    {
        UE_LOG(LogTemp, Display, TEXT("Attempting to refresh token..."));
        LoginWithEOS(CachedAuthToken, CachedAccountId, CachedPUID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot refresh token: AuthToken, PUID, or AccountId is missing."));
    }
}

void UHealthConnectNetworkManager::OnEOSLoginResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Display, TEXT("EOS Login Response Received. Success: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseContent = Response->GetContentAsString();
        UE_LOG(LogTemp, Display, TEXT("Server Response Content: %s"), *ResponseContent);

        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            if (JsonObject->HasField(TEXT("token")))
            {
                JWTToken = JsonObject->GetStringField(TEXT("token"));
                UE_LOG(LogTemp, Display, TEXT("Received JWT Token: %s"), *JWTToken);

                // Nastav JWT token pre ostatné služby, ak existujú
                SetJWTToken(JWTToken);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Token field is missing in the response JSON."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EOS Login request failed."));
        if (Response.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("Server Response Code: %d"), Response->GetResponseCode());
            UE_LOG(LogTemp, Warning, TEXT("Server Response Content: %s"), *Response->GetContentAsString());
        }
    }
}

void UHealthConnectNetworkManager::FetchActivityData()
{
    if (JWTToken.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot fetch activity data: JWT Token is missing."));
        return;
    }

    TArray<FString> IPAddresses;
    IPAddresses.Add(TEXT("http://192.168.137.128:8082/allData")); // Primary IP
    IPAddresses.Add(TEXT("http://192.168.1.26:8082/allData"));    // Secondary IP

    // Start fetching data with retries
    FetchActivityDataFromIPs(IPAddresses, /*RetryCount=*/3, /*RetryDelay=*/5.0f);
}

void UHealthConnectNetworkManager::FetchActivityDataFromIPs(TArray<FString> IPs, int32 RetryCount, float RetryDelay)
{
    if (IPs.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No IP addresses provided to fetch activity data."));
        return;
    }

    FString CurrentIP = IPs[0];
    UE_LOG(LogTemp, Display, TEXT("Attempting to fetch activity data from: %s"), *CurrentIP);

    TSharedRef<IHttpRequest> Request = CreateRequest(CurrentIP, TEXT("GET"), /*Timeout=*/30.0f); // Set timeout to 30 seconds
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *JWTToken));

    // Remove the current IP from the list
    IPs.RemoveAt(0);

    Request->OnProcessRequestComplete().BindLambda([this, IPs, RetryCount, RetryDelay](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
    {
        if (bWasSuccessful && ResponsePtr.IsValid())
        {
            OnFetchActivityDataResponseReceived(RequestPtr, ResponsePtr, bWasSuccessful);
        }
        else
        {
            if (RetryCount > 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Fetch attempt failed. Retrying in %f seconds..."), RetryDelay);

                // Schedule a retry after RetryDelay seconds
                FTimerHandle RetryTimerHandle;
                GetWorld()->GetTimerManager().SetTimer(RetryTimerHandle, FTimerDelegate::CreateLambda([this, IPs, RetryCount, RetryDelay]()
                {
                    // Retry the same IP with decremented RetryCount
                    FetchActivityDataFromIPs(IPs, RetryCount - 1, RetryDelay);
                }), RetryDelay, false);
            }
            else if (IPs.Num() > 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to fetch from current IP. Trying next IP."));
                // Reset RetryCount for the next IP
                FetchActivityDataFromIPs(IPs, /*RetryCount=*/3, RetryDelay);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to fetch activity data from all provided IPs."));
            }
        }
    });

    Request->ProcessRequest();
}

void UHealthConnectNetworkManager::OnFetchActivityDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        TArray<TSharedPtr<FJsonValue>> ActivityArray;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(Reader, ActivityArray))
        {
            UE_LOG(LogTemp, Display, TEXT("Successfully fetched activity data: %s"), *Response->GetContentAsString());

            // Odosielanie získaných dát na backend
            SubmitActivityData(ActivityArray);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON data from response."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FetchActivityData request failed. Possibly a network or server issue."));
    }
}

void UHealthConnectNetworkManager::SubmitActivityData(const TArray<TSharedPtr<FJsonValue>>& Activities)
{
    if (JWTToken.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot submit activity data: JWT token is missing."));
        return;
    }

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(Activities, Writer);

    UE_LOG(LogTemp, Display, TEXT("Submitting activity data: %s"), *RequestBody);

    TSharedRef<IHttpRequest> Request = CreateRequest(BackendBaseUrl + TEXT("/activities"), TEXT("POST"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *JWTToken)); // Nastavenie tokenu
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UHealthConnectNetworkManager::OnSubmitActivityResponseReceived);
    Request->ProcessRequest();
}

void UHealthConnectNetworkManager::OnSubmitActivityResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Display, TEXT("SubmitActivityData Response Received. Success: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

    if (bWasSuccessful && Response.IsValid())
    {
        UE_LOG(LogTemp, Display, TEXT("Submit Response Code: %d"), Response->GetResponseCode());
        UE_LOG(LogTemp, Display, TEXT("Submit Response Content: %s"), *Response->GetContentAsString());

        if (Response->GetResponseCode() == 200)
        {
            UE_LOG(LogTemp, Display, TEXT("Activity data submitted successfully."));

            // Fetch all activities to verify submission
            FetchAllActivities();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SubmitActivityData failed with response code %d: %s"),
                   Response->GetResponseCode(), *Response->GetContentAsString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SubmitActivityData request failed. Possibly a network or server issue."));
    }
}

void UHealthConnectNetworkManager::SavePlayerPosition(const FVector& Position)
{
    if (JWTToken.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No JWT token available."));
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetNumberField(TEXT("positionX"), Position.X);
    JsonObject->SetNumberField(TEXT("positionY"), Position.Y);
    JsonObject->SetNumberField(TEXT("positionZ"), Position.Z);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("http://localhost:3000/positions/save"), TEXT("POST"));
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UHealthConnectNetworkManager::OnSavePositionResponseReceived);
    Request->ProcessRequest();
}

void UHealthConnectNetworkManager::OnSavePositionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        UE_LOG(LogTemp, Display, TEXT("Position saved successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Save position request failed."));
    }
}

void UHealthConnectNetworkManager::FetchAllActivities()
{
    if (JWTToken.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot fetch activities: JWT Token is missing."));
        return;
    }

    FString Endpoint = BackendBaseUrl + TEXT("/activities/all");
    UE_LOG(LogTemp, Display, TEXT("Attempting to fetch all activities from: %s with JWT Token: %s"), *Endpoint, *JWTToken);

    TSharedRef<IHttpRequest> Request = CreateRequest(Endpoint, TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *JWTToken));
    Request->OnProcessRequestComplete().BindUObject(this, &UHealthConnectNetworkManager::OnFetchAllActivitiesResponseReceived);
    Request->ProcessRequest();
}

void UHealthConnectNetworkManager::OnFetchAllActivitiesResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Display, TEXT("FetchAllActivities response received. Success: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

    if (bWasSuccessful && Response.IsValid())
    {
        UE_LOG(LogTemp, Display, TEXT("Response Code: %d"), Response->GetResponseCode());
        UE_LOG(LogTemp, Display, TEXT("Response Content: %s"), *Response->GetContentAsString());

        if (Response->GetResponseCode() == 200)
        {
            TArray<TSharedPtr<FJsonValue>> ActivityArray;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

            if (FJsonSerializer::Deserialize(Reader, ActivityArray))
            {
                for (const TSharedPtr<FJsonValue>& ActivityValue : ActivityArray)
                {
                    TSharedPtr<FJsonObject> ActivityObject = ActivityValue->AsObject();
                    if (ActivityObject.IsValid())
                    {
                        int32 ActivityId = ActivityObject->GetIntegerField(TEXT("activityId"));
                        FString Date = ActivityObject->GetStringField(TEXT("date"));
                        int32 Steps = ActivityObject->GetIntegerField(TEXT("stepCount"));
                        float Calories = ActivityObject->GetNumberField(TEXT("calories"));
                        float SleepScore = ActivityObject->GetNumberField(TEXT("sleepScore"));
                        // Extract other fields as needed
                        float TotalSleepTime = ActivityObject->GetNumberField(TEXT("totalSleepTime"));
                        float DeepSleepTime = ActivityObject->GetNumberField(TEXT("deepSleepTime"));
                        float RemSleepTime = ActivityObject->GetNumberField(TEXT("remSleepTime"));
                        float LightSleepTime = ActivityObject->GetNumberField(TEXT("lightSleepTime"));
                        float AwakeTime = ActivityObject->GetNumberField(TEXT("awakeTime"));

                        UE_LOG(LogTemp, Display, TEXT("Activity ID: %d, Date: %s, Steps: %d, Calories: %f, SleepScore: %f"), ActivityId, *Date, Steps, Calories, SleepScore);
                        // Log other fields if necessary
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON from fetched activities."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FetchAllActivities failed with response code %d: %s"),
                   Response->GetResponseCode(), *Response->GetContentAsString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FetchAllActivities request failed. Connection or server issue."));
    }
}

void UHealthConnectNetworkManager::FetchAggregatedActivityData()
{
    if (JWTToken.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot fetch aggregated activities: JWT Token is missing."));
        return;
    }

    FString Endpoint = BackendBaseUrl + TEXT("/activities/aggregate");
    UE_LOG(LogTemp, Display, TEXT("Fetching aggregated activities from: %s"), *Endpoint);

    TSharedRef<IHttpRequest> Request = CreateRequest(Endpoint, TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *JWTToken));
    Request->OnProcessRequestComplete().BindUObject(this, &UHealthConnectNetworkManager::OnFetchAggregatedActivitiesResponseReceived);
    Request->ProcessRequest();
}

void UHealthConnectNetworkManager::OnFetchAggregatedActivitiesResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (bWasSuccessful && Response.IsValid())
    {
        TSharedPtr<FJsonObject> JsonObject;
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            int32 TotalSteps = JsonObject->GetIntegerField(TEXT("totalSteps"));
            // Update stamina
            UpdatePlayerStamina(TotalSteps);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON from aggregated activities response."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FetchAggregatedActivities request failed."));
    }
}

void UHealthConnectNetworkManager::UpdatePlayerStamina(int32 TotalSteps)
{
    int32 StaminaPoints = TotalSteps / 100; // Adjust conversion logic as needed

    UWorld* World = GetWorld();
    if (World)
    {
        UEOS_GameInstance* GameInstance = Cast<UEOS_GameInstance>(World->GetGameInstance());
        if (GameInstance)
        {
            GameInstance->SetPlayerStamina(StaminaPoints);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("GameInstance is null in UpdatePlayerStamina."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("World is null in UpdatePlayerStamina."));
    }

    UE_LOG(LogTemp, Display, TEXT("Player's stamina updated to: %d"), StaminaPoints);
}

