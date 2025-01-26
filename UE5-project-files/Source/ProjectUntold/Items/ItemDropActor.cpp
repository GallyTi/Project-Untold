#include "ItemDropActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "../InventorySystem/PlayerInventoryComponent.h"
#include "GameFramework/PlayerState.h"
#include "ProjectUntold/ProjectUntoldCharacter.h"

AItemDropActor::AItemDropActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Vytvoríme kolíznu sféru
    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    RootComponent = SphereCollision;
    SphereCollision->InitSphereRadius(100.f);
    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

    // Mesh
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Nastavenie default hodnôt
    ItemId = 1;
    Quantity = 1;
    DropChance = 100.f; // default 100% ak nepotrebuješ random

    // Overlap event
    SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AItemDropActor::OnSphereOverlapBegin);
}

void AItemDropActor::BeginPlay()
{
    Super::BeginPlay();
}

void AItemDropActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AItemDropActor::PickupItem(AActor* InteractingActor)
{
    if (!InteractingActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupItem: No InteractingActor!"));
        return;
    }

    // Hráč je (väčšinou) PlayerController->GetPawn() alebo priamo Character
    APlayerController* PC = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!PC)
    {
        // Alebo cast na APlayerController, ktorého owner je InteractingActor
        PC = Cast<APlayerController>(InteractingActor);
    }
    
    // Ak máme PC, vieme získať inventory
    if (PC)
    {
        // Napr. PlayerState, a v ňom UPlayerInventoryComponent
        if (APlayerState* PS = PC->PlayerState)
        {
            UPlayerInventoryComponent* InvComp = PS->FindComponentByClass<UPlayerInventoryComponent>();
            if (InvComp)
            {
                // Zavoláme server funkciu
                InvComp->ServerAddItem(ItemId, Quantity);
                // (Táto potom spraví aj volanie do backendu – AddItem -> Node.js)
            }
        }
    }

    // Zničíme item, lebo ho hráč zodvihol
    Destroy();
}


// void AItemDropActor::OnSphereOverlapBegin(
     // UPrimitiveComponent* OverlappedComp,
     // AActor* OtherActor,
     // UPrimitiveComponent* OtherComp,
     // int32 OtherBodyIndex,
     // bool bFromSweep,
     // const FHitResult& SweepResult
// )
// {
//     // Overíme, či OtherActor je hráč, atď.
//     if (!OtherActor) return;
//
//     // Príklad: Spúšťaj len ak hráč stlačí klávesu F alebo čokoľvek.
//     // Alebo to môžme robiť automaticky už tu. Je to na tebe.
//     // Nižšie je len ukážka, ak by sme chceli auto-pickup:
//     /*
//     if (AProjectUntoldCharacter* MyChar = Cast<AProjectUntoldCharacter>(OtherActor))
//     {
//         PickupItem(MyChar);
//     }
//     */
//     // Alebo nechaj prázdne, a funkciu PickupItem() zavoláš ručne pri stlačení klávesy.
// }

void AItemDropActor::OnSphereOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    AProjectUntoldCharacter* Char = Cast<AProjectUntoldCharacter>(OtherActor);
    if (Char)
    {
        PlayersInside.Add(Char);
    }
}

void AItemDropActor::OnSphereOverlapEnd(
UPrimitiveComponent* OverlappedComp,
AActor* OtherActor,
UPrimitiveComponent* OtherComp,
int32 OtherBodyIndex,
bool bFromSweep,
const FHitResult& SweepResult
)
{
    AProjectUntoldCharacter* Char = Cast<AProjectUntoldCharacter>(OtherActor);
    if (Char)
    {
        PlayersInside.Remove(Char);
    }
}
