#include "GridPuzzleGameMode.h"
#include "GridController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

AGridPuzzleGameMode::AGridPuzzleGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    bLastClickProcessed = false;
    GridController = nullptr;
}

void AGridPuzzleGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Error, TEXT("=== GAME MODE BEGIN PLAY ==="));
    
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        APawn* DefaultPawn = PlayerController->GetPawn();
        if (DefaultPawn)
        {
            DefaultPawn->Destroy();
            PlayerController->UnPossess();
            UE_LOG(LogTemp, Log, TEXT("DefaultPawn destroyed"));
        }
        
        PlayerController->bShowMouseCursor = true;
        PlayerController->bEnableClickEvents = true;
        PlayerController->bEnableMouseOverEvents = true;
        
        UE_LOG(LogTemp, Log, TEXT("PlayerController configured: cursor shown, click events enabled"));
    }
    
    // Находим GridController на сцене
    for (TActorIterator<AGridController> It(GetWorld()); It; ++It)
    {
        GridController = *It;
        break;
    }
    
    if (GridController)
    {
        UE_LOG(LogTemp, Log, TEXT("GridController found on scene"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GridController not found on scene! Please place BP_GridController in the level."));
    }
    
    UE_LOG(LogTemp, Error, TEXT("=== GAME MODE INITIALIZATION COMPLETE ==="));
}

void AGridPuzzleGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ProcessMouseClick();
}

void AGridPuzzleGameMode::ProcessMouseClick()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    
    if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
    {
        FHitResult Hit;
        bool bHit = PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
        
        if (bHit)
        {
            AGridPuzzleTile* HitTile = Cast<AGridPuzzleTile>(Hit.GetActor());
            if (HitTile && GridController)
            {
                GridController->TryMoveTile(HitTile);
            }
        }
    }
}