#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EOS_PlayerController.generated.h"

UCLASS()
class PROJECTUNTOLD_API AEOS_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override; // Add this declaration
	virtual void OnNetCleanup(UNetConnection* Connection) override;
};
