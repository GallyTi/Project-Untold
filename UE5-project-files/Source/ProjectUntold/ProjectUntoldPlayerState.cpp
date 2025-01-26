#include "ProjectUntoldPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "InventorySystem/PlayerInventoryComponent.h"

AProjectUntoldPlayerState::AProjectUntoldPlayerState()
{
	InventoryComp = CreateDefaultSubobject<UPlayerInventoryComponent>(TEXT("InventoryComp"));
	// bReplicates = true; // PlayerState je už defaultne replikované
}

void AProjectUntoldPlayerState::BeginPlay()
{
	Super::BeginPlay();
    
	if (HasAuthority() && InventoryComp)
	{
		// Napr. hneď fetch z backendu:
		// InventoryComp->ServerFetchInventoryFromBackend();
	}
}

void AProjectUntoldPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(AProjectUntoldPlayerState, SomeReplicatedVariable);
}
