#include "ProjectUntoldGameMode.h"

#include "EOS_PlayerController.h"
#include "OnlineSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "UObject/ConstructorHelpers.h"

AProjectUntoldGameMode::AProjectUntoldGameMode()
{
	// Set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// Set your custom PlayerController class
	PlayerControllerClass = AEOS_PlayerController::StaticClass();
}

// In AProjectUntoldGameMode.cpp
void AProjectUntoldGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer && NewPlayer->PlayerState)
	{
		FUniqueNetIdRepl UniqueNetIdRepl = NewPlayer->PlayerState->GetUniqueId();

		if (!UniqueNetIdRepl.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Player UniqueNetId is invalid"));
			return;
		}

		TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
		if (!UniqueNetId.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get UniqueNetId"));
			return;
		}

		IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(GetWorld());
		if (!SubsystemRef)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get Online Subsystem"));
			return;
		}

		IOnlineSessionPtr SessionRef = SubsystemRef->GetSessionInterface();
		if (!SessionRef.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get Session Interface"));
			return;
		}

		// Register the player without a delegate
		bool bRegistrationSuccess = SessionRef->RegisterPlayer(NAME_GameSession, *UniqueNetId, false);
		if (bRegistrationSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("Registration Successful"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Registration Failed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("NewPlayer or PlayerState is null"));
	}
}

void AProjectUntoldGameMode::OnRegisterPlayersComplete(FName SessionName, const TArray< TSharedRef<const FUniqueNetId> >& PlayerIds, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnRegisterPlayersComplete for session: %s, success: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	// Remove the delegate after handling the event
	IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(GetWorld());
	if (SubsystemRef)
	{
		IOnlineSessionPtr SessionRef = SubsystemRef->GetSessionInterface();
		if (SessionRef.IsValid())
		{
			SessionRef->ClearOnRegisterPlayersCompleteDelegate_Handle(OnRegisterPlayersCompleteDelegateHandle);
		}
	}
}


void AProjectUntoldGameMode::StartPlay()
{
	Super::StartPlay();

	/*
	// Initialize NetworkManager here and call FetchActivityData only once
	if (!NetworkManager)
	{
		NetworkManager = NewObject<UHealthConnectNetworkManager>(this);
		if (NetworkManager)
		{
			// Register the player first if needed, then fetch data
			NetworkManager->RegisterPlayer();
			NetworkManager->FetchActivityData();  // This will run only once at game start
		}
	}*/
}