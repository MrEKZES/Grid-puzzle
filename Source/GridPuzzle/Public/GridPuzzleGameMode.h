#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridPuzzleTile.h"
#include "GridPuzzleGameMode.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnVictoryDelegate);

UCLASS()
class GRIDPUZZLE_API AGridPuzzleGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AGridPuzzleGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    FOnVictoryDelegate OnVictory;

protected:
    void ProcessMouseClick();

    UPROPERTY()
    class AGridController* GridController;

    bool bLastClickProcessed;
};