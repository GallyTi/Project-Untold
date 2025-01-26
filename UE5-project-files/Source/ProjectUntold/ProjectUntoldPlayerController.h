#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ProjectUntoldPlayerController.generated.h"

class UUserWidget;
class UInventoryWidget;

UCLASS()
class PROJECTUNTOLD_API AProjectUntoldPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

	// Môžeme držať referenciu na náš inventárny widget
	UPROPERTY(BlueprintReadOnly, Category="UI")
	UInventoryWidget* InventoryWidget;

protected:

	// Trieda widgetu (nastavíte ju v blueprint properties alebo v C++)
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	// Helper funkcia na init UI pri spustení
	virtual void BeginPlay() override;

	// Samotné togglovanie inventára (otvorenie/zatvorenie)
	void ToggleInventory();
    
	// Indikátor, či je inventory aktuálne otvorené
	bool bIsInventoryOpen = false;

private:
	void ShowInventory();
	void HideInventory();
};
