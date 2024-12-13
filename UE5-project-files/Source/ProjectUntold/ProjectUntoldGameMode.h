#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectUntoldGameMode.generated.h"

UCLASS(minimalapi)
class AProjectUntoldGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AProjectUntoldGameMode();

protected:
	virtual void StartPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Declare the method here
	void OnRegisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);

private:
	FDelegateHandle OnRegisterPlayersCompleteDelegateHandle;
};
