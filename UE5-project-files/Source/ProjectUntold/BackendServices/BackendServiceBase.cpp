#include "BackendServiceBase.h"
#include "HealthConnectNetworkManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UBackendServiceBase::UBackendServiceBase()
{
}

void UBackendServiceBase::SetJWTToken(const FString& Token)
{
	JWTToken = Token;
}

TSharedRef<IHttpRequest> UBackendServiceBase::CreateRequest(const FString& URL, const FString& Verb, float Timeout)
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	if (!JWTToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString(TEXT("Bearer ")) + JWTToken);
	}
	// Set the timeout duration
	Request->SetTimeout(Timeout);
	return Request;
}

void UBackendServiceBase::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid())
	{
		int32 ResponseCode = Response->GetResponseCode();
		if (ResponseCode == 401)
		{
			// Token vypršal alebo je neplatný
			// Pokúsiť sa obnoviť token alebo opätovne prihlásiť
			AttemptTokenRefresh();
		}
		else
		{
			// Spracovať odpoveď
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Request failed"));
	}
}

void UBackendServiceBase::AttemptTokenRefresh()
{
	if (HealthConnectNetworkManager)
	{
		HealthConnectNetworkManager->RefreshToken();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HealthConnectNetworkManager is null"));
	}
}