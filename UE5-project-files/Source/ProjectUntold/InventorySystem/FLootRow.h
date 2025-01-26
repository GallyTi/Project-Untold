#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FLootRow.generated.h"

USTRUCT(BlueprintType)
struct FLootRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectID;   // e.g. "King", "Stone", "Skeleton"

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ItemID = 0;      // e.g. 1 = Gold, 2 = Iron, ...

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DropChance = 0.f;  // Percentage chance (0-100)
};
