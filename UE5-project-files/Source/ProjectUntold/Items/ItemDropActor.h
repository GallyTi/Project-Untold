#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"
#include "ItemStructs.h" // ak máš definované nejaké FItemData, FItemMeta, atď.
#include "ProjectUntold/ProjectUntoldCharacter.h"
#include "ItemDropActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 *  AItemDropActor - master trieda/actor pre "voľne pohodené itemy",
 *  ktoré hráč môže zobrať do inventára stlačením F alebo overlapom
 */
UCLASS()
class PROJECTUNTOLD_API AItemDropActor : public AActor
{
    GENERATED_BODY()

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Komponenty
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup Actor|Components")
    USphereComponent* SphereCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pickup Actor|Components")
    UStaticMeshComponent* MeshComp;

    // --------------------
    // Vlastnosti itemu
    // --------------------

    /** Aké ID itemu z nášho systému (napr. 4 = meč, 5 = potion, atď.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Actor|Item")
    int32 ItemId;

    /** Množstvo (ak je to stackovateľné), inak 1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Actor|Item")
    int32 Quantity;

    /** DropChance, ak by si mal random spawny, atď. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Actor|Item")
    float DropChance;

    /** Ikona itemu, ak ju chceš zobraziť v 3D (Billboard) alebo v UI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Actor|Item")
    UTexture2D* ItemIcon;

public:
    AItemDropActor();
    
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    /** Hlavná funkcia, ktorou dáme item hráčovi - bude volať AddItemToInventory(...) na serveri, atď. */
    UFUNCTION(BlueprintCallable, Category="Pickup Actor|Item")
    void PickupItem(AActor* InteractingActor);

    /** Pokiaľ chceš spúšťať zbieranie pri overlape */
    UFUNCTION()
    void OnSphereOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnSphereOverlapEnd(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

private:
    UPROPERTY()
    TSet<AProjectUntoldCharacter*> PlayersInside;  // všetci hráči v Overlape
};
