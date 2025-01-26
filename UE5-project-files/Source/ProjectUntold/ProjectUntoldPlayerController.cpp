#include "ProjectUntoldPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystem/InventoryWidget.h"

void AProjectUntoldPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
    
	// Bindneme akciu ToggleInventory (definovanú v ProjectSettings -> Input) na funkciu ToggleInventory
	InputComponent->BindAction("ToggleInventory", IE_Pressed, this, &AProjectUntoldPlayerController::ToggleInventory);
}

void AProjectUntoldPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Vytvoríme InventoryWidget – zatiaľ hidden
	if (InventoryWidgetClass)
	{
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(this, InventoryWidgetClass);
		if (CreatedWidget)
		{
			InventoryWidget = Cast<UInventoryWidget>(CreatedWidget);
			InventoryWidget->AddToViewport();
			InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AProjectUntoldPlayerController::ToggleInventory()
{
	bIsInventoryOpen = !bIsInventoryOpen;

	if (bIsInventoryOpen)
	{
		ShowInventory();
	}
	else
	{
		HideInventory();
	}
}

void AProjectUntoldPlayerController::ShowInventory()
{
	if (!InventoryWidget)
	{
		// Prvýkrát vytvoríme
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(this, InventoryWidgetClass);
		InventoryWidget = Cast<UInventoryWidget>(CreatedWidget);

		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport();
		}
	}

	// Zapneme viditeľnosť
	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
	}

	bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void AProjectUntoldPlayerController::HideInventory()
{
	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	bShowMouseCursor = false;
	FInputModeGameOnly GameOnlyMode;
	SetInputMode(GameOnlyMode);
}
