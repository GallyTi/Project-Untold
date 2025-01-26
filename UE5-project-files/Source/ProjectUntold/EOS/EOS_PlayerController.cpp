#include "EOS_PlayerController.h"
#include "EOS_GameInstance.h"

void AEOS_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Show the mouse cursor
	bShowMouseCursor = true;

	// Set input mode to allow mouse interaction
	SetInputMode(FInputModeGameAndUI());
}

void AEOS_PlayerController::OnNetCleanup(UNetConnection* Connection)
{
	UEOS_GameInstance* GameInstanceRef = Cast<UEOS_GameInstance>(GetWorld()->GetGameInstance());
	if (GameInstanceRef)
	{
		GameInstanceRef->DestroySession();
	}

	Super::OnNetCleanup(Connection);
}
