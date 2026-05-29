#include "GridPuzzleGameMode.h"
#include "GridPuzzleTile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AGridPuzzleGameMode::AGridPuzzleGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    GridSize = 3;
    TileSize = 110.0f;
    EmptyRow = -1;
    EmptyCol = -1;
    bLastClickProcessed = false;
    bGameEnded = false;
    VictoryWidget = nullptr;
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
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
    }
    
    ValidateGridSize();
    InitializeGrid();
    SpawnTiles();
    SpawnColumnIndicators();
    ShuffleTiles(500);
    
    UE_LOG(LogTemp, Error, TEXT("=== GAME MODE INITIALIZATION COMPLETE ==="));
}

void AGridPuzzleGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bGameEnded)
    {
        ProcessMouseClick();
    }
}

void AGridPuzzleGameMode::ProcessMouseClick()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    
    if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
    {
        UE_LOG(LogTemp, Error, TEXT("=== LEFT MOUSE BUTTON PRESSED ==="));
        
        FHitResult Hit;
        bool bHit = PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
        
        if (bHit)
        {
            UE_LOG(LogTemp, Error, TEXT("Raycast HIT: Actor=%s, Component=%s"), 
                Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("NULL"),
                Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("NULL"));
            
            AGridPuzzleTile* HitTile = Cast<AGridPuzzleTile>(Hit.GetActor());
            if (HitTile)
            {
                UE_LOG(LogTemp, Error, TEXT("SUCCESS: Hit tile at (%d, %d) with color %d"), 
                    HitTile->GetGridRow(), HitTile->GetGridCol(), (int32)HitTile->GetColorType());
                TryMoveTile(HitTile->GetGridRow(), HitTile->GetGridCol());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("FAIL: Hit actor is NOT AGridPuzzleTile! Actor class: %s"), 
                    Hit.GetActor() ? *Hit.GetActor()->GetClass()->GetName() : TEXT("NULL"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Raycast MISSED - no hit under cursor"));
        }
    }
}

void AGridPuzzleGameMode::ValidateGridSize()
{
    // Ограничиваем размер сетки от 2 до 9 (по количеству доступных цветов)
    if (GridSize < 2)
    {
        GridSize = 2;
        UE_LOG(LogTemp, Warning, TEXT("GridSize was less than 2, set to 2"));
    }
    
    if (GridSize > 9)
    {
        GridSize = 9;
        UE_LOG(LogTemp, Warning, TEXT("GridSize was greater than 9, set to 9 (maximum colors available)"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("GridSize validated: %d"), GridSize);
}

EColorType AGridPuzzleGameMode::GetColumnColor(int32 ColIndex) const
{
    // ColIndex от 0 до GridSize-1 (до 9 колонок)
    // Каждая колонка имеет свой уникальный цвет
    
    switch (ColIndex)
    {
    case 0:  return EColorType::Red;
    case 1:  return EColorType::Orange;
    case 2:  return EColorType::Yellow;
    case 3:  return EColorType::Green;
    case 4:  return EColorType::Cyan;
    case 5:  return EColorType::Blue;
    case 6:  return EColorType::Purple;
    case 7:  return EColorType::Pink;
    case 8:  return EColorType::Magenta;
    default: return EColorType::White;
    }
}

void AGridPuzzleGameMode::InitializeGrid()
{
    int32 TotalTiles = GridSize * GridSize;
    Tiles.SetNum(TotalTiles);
    for (int32 i = 0; i < TotalTiles; i++) Tiles[i] = nullptr;
    UE_LOG(LogTemp, Log, TEXT("Grid initialized with %d tiles"), TotalTiles);
}

void AGridPuzzleGameMode::UpdateAllPositions()
{
    float OffsetX = (GridSize - 1) * TileSize * 0.5f;
    float OffsetY = (GridSize - 1) * TileSize * 0.5f;
    
    for (int32 i = 0; i < GridSize; i++)
    {
        for (int32 j = 0; j < GridSize; j++)
        {
            AGridPuzzleTile* Tile = GetTile(i, j);
            if (Tile)
            {
                FVector TargetPos(j * TileSize - OffsetX, i * TileSize - OffsetY, 0.0f);
                Tile->SetTargetPosition(TargetPos);
            }
        }
    }
}

void AGridPuzzleGameMode::SpawnColumnIndicators()
{
    if (!ColumnIndicatorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ColumnIndicatorClass is NULL, skipping indicators"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    float OffsetX = (GridSize - 1) * TileSize * 0.5f;
    float OffsetY = (GridSize - 1) * TileSize * 0.5f;
    
    for (int32 j = 0; j < GridSize; j++)
    {
        EColorType ColumnColor = GetColumnColor(j);
        
        // Получаем цвет для индикатора
        FLinearColor IndicatorColor;
        switch (ColumnColor)
        {
            case EColorType::Red:     IndicatorColor = FLinearColor(1.0f, 0.0f, 0.0f); break;
            case EColorType::Orange:  IndicatorColor = FLinearColor(1.0f, 0.5f, 0.0f); break;
            case EColorType::Yellow:  IndicatorColor = FLinearColor(1.0f, 1.0f, 0.0f); break;
            case EColorType::Green:   IndicatorColor = FLinearColor(0.0f, 1.0f, 0.0f); break;
            case EColorType::Cyan:    IndicatorColor = FLinearColor(0.0f, 1.0f, 1.0f); break;
            case EColorType::Blue:    IndicatorColor = FLinearColor(0.0f, 0.0f, 1.0f); break;
            case EColorType::Purple:  IndicatorColor = FLinearColor(0.5f, 0.0f, 0.5f); break;
            case EColorType::Pink:    IndicatorColor = FLinearColor(1.0f, 0.75f, 0.8f); break;
            case EColorType::Magenta: IndicatorColor = FLinearColor(1.0f, 0.0f, 1.0f); break;
            default:                  IndicatorColor = FLinearColor::White; break;
        }
        
        FVector Location(
            j * TileSize - OffsetX,
            -OffsetY - TileSize * 0.8f,
            30.0f
        );
        
        AActor* Indicator = World->SpawnActor<AActor>(ColumnIndicatorClass, Location, FRotator::ZeroRotator);
        if (Indicator)
        {
            Indicator->SetActorScale3D(FVector(0.8f, 0.3f, 0.2f));
            
            // Пытаемся установить цвет индикатора
            UStaticMeshComponent* MeshComp = Indicator->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                // Создаём динамический материал для индикатора
                UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(0);
                if (BaseMaterial)
                {
                    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Indicator);
                    DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), FVector(IndicatorColor));
                    MeshComp->SetMaterial(0, DynMaterial);
                }
                else
                {
                    // Если нет материала, просто устанавливаем цвет через векторный параметр
                    MeshComp->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(IndicatorColor));
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Indicator spawned at column %d"), j + 1);
        }
    }
}

void AGridPuzzleGameMode::SpawnTiles()
{
    if (!TileActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TileActorClass is NULL! Cannot spawn tiles!"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<FIntPoint> AllPositions;
    TArray<EColorType> ColorsToPlace;
    
    for (int32 i = 0; i < GridSize; i++)
    {
        for (int32 j = 0; j < GridSize; j++)
        {
            AllPositions.Add(FIntPoint(i, j));
        }
    }
    
    for (int32 j = 0; j < GridSize; j++)
    {
        EColorType ColumnColor = GetColumnColor(j);
        for (int32 i = 0; i < GridSize; i++)
        {
            ColorsToPlace.Add(ColumnColor);
        }
    }
    
    ColorsToPlace[ColorsToPlace.Num() - 1] = EColorType::Empty;
    
    for (int32 i = ColorsToPlace.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        ColorsToPlace.Swap(i, j);
    }
    
    float OffsetX = (GridSize - 1) * TileSize * 0.5f;
    float OffsetY = (GridSize - 1) * TileSize * 0.5f;
    
    int32 SpawnCount = 0;
    for (int32 idx = 0; idx < AllPositions.Num(); idx++)
    {
        int32 Row = AllPositions[idx].X;
        int32 Col = AllPositions[idx].Y;
        
        FVector Location(
            Col * TileSize - OffsetX,
            Row * TileSize - OffsetY,
            0.0f
        );
        
        AGridPuzzleTile* Tile = World->SpawnActor<AGridPuzzleTile>(TileActorClass, Location, FRotator::ZeroRotator);
        if (Tile)
        {
            EColorType Color = ColorsToPlace[idx];
            Tile->Init(Row, Col, Color);
            SetTile(Row, Col, Tile);
            SpawnCount++;
            
            UE_LOG(LogTemp, Log, TEXT("Tile spawned at (%d,%d) with color %d"), Row, Col, (int32)Color);
            
            if (Color == EColorType::Empty)
            {
                EmptyRow = Row;
                EmptyCol = Col;
                UE_LOG(LogTemp, Error, TEXT("!!! EMPTY TILE at (%d, %d) !!!"), EmptyRow, EmptyCol);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FAILED to spawn tile at (%d,%d)"), Row, Col);
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Spawned %d of %d tiles. Empty at (%d, %d)"), 
        SpawnCount, AllPositions.Num(), EmptyRow, EmptyCol);
    
    // Отладочный вывод состояния сетки
    UE_LOG(LogTemp, Error, TEXT("=== FINAL GRID STATE ==="));
    for (int32 i = 0; i < GridSize; i++)
    {
        FString RowLog;
        for (int32 j = 0; j < GridSize; j++)
        {
            AGridPuzzleTile* Tile = GetTile(i, j);
            if (Tile)
            {
                RowLog += FString::Printf(TEXT("[%d] "), (int32)Tile->GetColorType());
            }
            else
            {
                RowLog += TEXT("[X] ");
            }
        }
        UE_LOG(LogTemp, Error, TEXT("Row %d: %s"), i, *RowLog);
    }
}

void AGridPuzzleGameMode::SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB)
{
    // Получаем плитки
    AGridPuzzleTile* TileA = GetTile(RowA, ColA);
    AGridPuzzleTile* TileB = GetTile(RowB, ColB);
    
    if (!TileA || !TileB) return;
    
    // МЕНЯЕМ ПОЗИЦИИ В МАССИВЕ Tiles
    SetTile(RowB, ColB, TileA);  // TileA встает на место TileB
    SetTile(RowA, ColA, TileB);  // TileB встает на место TileA
    
    // Обновляем логические координаты плиток
    TileA->SetGridPosition(RowB, ColB);
    TileB->SetGridPosition(RowA, ColA);
    
    // Обновляем пустую позицию
    if (TileA->GetColorType() == EColorType::Empty)
    {
        EmptyRow = RowB;
        EmptyCol = ColB;
    }
    else if (TileB->GetColorType() == EColorType::Empty)
    {
        EmptyRow = RowA;
        EmptyCol = ColA;
    }
    
    // Обновляем физические позиции (анимация)
    UpdateAllPositions();
    
    UE_LOG(LogTemp, Error, TEXT("Swapped tiles (%d,%d) <-> (%d,%d). Empty now at (%d,%d)"), 
        RowA, ColA, RowB, ColB, EmptyRow, EmptyCol);
}

void AGridPuzzleGameMode::TryMoveTile(int32 Row, int32 Col)
{
    if (bGameEnded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Game already ended, ignoring move"));
        return;
    }
    
    UE_LOG(LogTemp, Error, TEXT("=== TRY MOVE TILE at (%d, %d) ==="), Row, Col);
    UE_LOG(LogTemp, Error, TEXT("Current empty position: (%d, %d)"), EmptyRow, EmptyCol);
    
    if (Row < 0 || Row >= GridSize || Col < 0 || Col >= GridSize)
    {
        UE_LOG(LogTemp, Error, TEXT("FAIL: Out of bounds! Row=%d Col=%d GridSize=%d"), Row, Col, GridSize);
        return;
    }
    
    AGridPuzzleTile* Tile = GetTile(Row, Col);
    if (!Tile)
    {
        UE_LOG(LogTemp, Error, TEXT("FAIL: No tile at (%d, %d)"), Row, Col);
        return;
    }
    
    EColorType TileColor = Tile->GetColorType();
    UE_LOG(LogTemp, Error, TEXT("Tile color: %d"), (int32)TileColor);
    
    if (TileColor == EColorType::Empty)
    {
        UE_LOG(LogTemp, Error, TEXT("FAIL: Cannot move empty tile!"));
        return;
    }
    
    int32 Distance = FMath::Abs(Row - EmptyRow) + FMath::Abs(Col - EmptyCol);
    UE_LOG(LogTemp, Error, TEXT("Manhattan distance to empty: %d"), Distance);
    
    if (Distance == 1)
    {
        UE_LOG(LogTemp, Error, TEXT("SUCCESS: Moving tile!"));
        SwapTiles(Row, Col, EmptyRow, EmptyCol);
        CheckVictory();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FAIL: Not adjacent to empty tile! Distance=%d"), Distance);
    }
}

void AGridPuzzleGameMode::CheckVictory()
{
    bool bVictory = true;
    
    for (int32 i = 0; i < GridSize; i++)
    {
        for (int32 j = 0; j < GridSize; j++)
        {
            AGridPuzzleTile* Tile = GetTile(i, j);
            if (!Tile) continue;
            
            EColorType TileColor = Tile->GetColorType();
            if (TileColor == EColorType::Empty) continue;
            
            EColorType ExpectedColor = GetColumnColor(j);
            
            if (TileColor != ExpectedColor)
            {
                bVictory = false;
                UE_LOG(LogTemp, Log, TEXT("Victory not yet: tile at (%d,%d) is %d, should be %d (col %d expected %d)"), 
                    i, j, (int32)TileColor, (int32)ExpectedColor, j, (int32)ExpectedColor);
            }
        }
    }
    
    if (bVictory && !bGameEnded)
    {
        bGameEnded = true;
        OnVictory.Broadcast();
        UE_LOG(LogTemp, Error, TEXT("!!! VICTORY !!!"));
        
        // Показываем виджет победы
        ShowVictoryWidget();
    }
}

void AGridPuzzleGameMode::ShowVictoryWidget()
{
    if (VictoryWidgetClass && !VictoryWidget)
    {
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            VictoryWidget = CreateWidget<UUserWidget>(PlayerController, VictoryWidgetClass);
            if (VictoryWidget)
            {
                VictoryWidget->AddToViewport();
                
                // Показываем курсор мыши
                PlayerController->bShowMouseCursor = true;
                
                // Устанавливаем input mode для UI
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(VictoryWidget->TakeWidget());
                PlayerController->SetInputMode(InputMode);
                
                UE_LOG(LogTemp, Error, TEXT("Victory widget shown!"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to create victory widget!"));
                // Альтернативное сообщение, если виджет не создался
                PlayerController->ClientMessage(TEXT("You win!"));
            }
        }
    }
    else if (!VictoryWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("VictoryWidgetClass is not set! Showing simple message instead."));
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            PlayerController->ClientMessage(TEXT("You win!"));
        }
    }
}

void AGridPuzzleGameMode::ShuffleTiles(int32 MovesCount)
{
    UE_LOG(LogTemp, Error, TEXT("=== SHUFFLING TILES ==="));
    
    for (int32 Move = 0; Move < MovesCount; Move++)
    {
        TArray<FIntPoint> Neighbors;
        
        if (EmptyRow > 0) Neighbors.Add(FIntPoint(EmptyRow - 1, EmptyCol));
        if (EmptyRow < GridSize - 1) Neighbors.Add(FIntPoint(EmptyRow + 1, EmptyCol));
        if (EmptyCol > 0) Neighbors.Add(FIntPoint(EmptyRow, EmptyCol - 1));
        if (EmptyCol < GridSize - 1) Neighbors.Add(FIntPoint(EmptyRow, EmptyCol + 1));
        
        if (Neighbors.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, Neighbors.Num() - 1);
            FIntPoint Target = Neighbors[RandomIndex];
            SwapTiles(Target.X, Target.Y, EmptyRow, EmptyCol);
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("=== SHUFFLE COMPLETE ==="));
}

void AGridPuzzleGameMode::ResetGame()
{
    bGameEnded = false;
    
    // Скрываем виджет победы, если он есть
    if (VictoryWidget)
    {
        VictoryWidget->RemoveFromParent();
        VictoryWidget = nullptr;
    }
    
    for (int32 i = 0; i < Tiles.Num(); i++)
    {
        if (Tiles[i])
        {
            Tiles[i]->Destroy();
            Tiles[i] = nullptr;
        }
    }
    
    Tiles.Empty();
    EmptyRow = -1;
    EmptyCol = -1;
    
    InitializeGrid();
    SpawnTiles();
    SpawnColumnIndicators();
    ShuffleTiles(500);
    
    // Возвращаем управление игроку
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        PlayerController->bShowMouseCursor = true;
        FInputModeGameAndUI InputMode;
        PlayerController->SetInputMode(InputMode);
    }
}