#include "GridPuzzleGameMode.h"
#include "GridPuzzleTile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"

AGridPuzzleGameMode::AGridPuzzleGameMode()
{
    GridRows = 5;
    GridCols = 5;
    TileSize = 110.0f;
    EmptyRow = -1;
    EmptyCol = -1;
}

void AGridPuzzleGameMode::BeginPlay()
{
    Super::BeginPlay();
    
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
    }
    
    ValidateGridSize();
    InitializeGrid();
    SpawnTiles();
    ShuffleTiles(500);
}

void AGridPuzzleGameMode::ValidateGridSize()
{
    if (GridRows < 5) GridRows = 5;
    if (GridCols < 5) GridCols = 5;
    
    int32 MaxSize = FMath::Max(GridRows, GridCols);
    if (MaxSize % 2 == 0) MaxSize++;
    
    GridRows = MaxSize;
    GridCols = MaxSize;
}

bool AGridPuzzleGameMode::IsBlackCell(int32 Row, int32 Col) const
{
    int32 OneBasedRow = Row + 1;
    int32 OneBasedCol = Col + 1;
    return (OneBasedRow % 2 == 0 && OneBasedCol % 2 == 0);
}

bool AGridPuzzleGameMode::IsEmptyCell(int32 Row, int32 Col) const
{
    if (IsBlackCell(Row, Col)) return false;
    
    int32 OneBasedCol = Col + 1;
    int32 OneBasedRow = Row + 1;
    
    // Чётные столбцы на нечётных строках - пустые клетки
    if (OneBasedCol % 2 == 0 && OneBasedRow % 2 == 1)
        return true;
    
    return false;
}

EColorType AGridPuzzleGameMode::GetZoneColor(int32 Row, int32 Col) const
{
    if (IsBlackCell(Row, Col))
        return EColorType::Black;
    
    int32 OneBasedCol = Col + 1;
    int32 OneBasedRow = Row + 1;
    
    // Чётные столбцы на нечётных строках - пустые клетки
    if (OneBasedCol % 2 == 0 && OneBasedRow % 2 == 1)
        return EColorType::Empty;
    
    // Нечётные столбцы - цветные зоны
    if (OneBasedCol % 2 == 1)
    {
        if (OneBasedCol == 1)
            return EColorType::Red;
        
        int32 MidCol = (GridCols + 1) / 2;
        if (OneBasedCol == MidCol)
            return EColorType::Green;
        
        if (OneBasedCol == GridCols)
            return EColorType::Blue;
    }
    
    return EColorType::Empty;
}

void AGridPuzzleGameMode::InitializeGrid()
{
    int32 TotalTiles = GridRows * GridCols;
    Tiles.SetNum(TotalTiles);
    
    for (int32 i = 0; i < TotalTiles; i++)
    {
        Tiles[i] = nullptr;
    }
}

TArray<FIntPoint> AGridPuzzleGameMode::GetAdjacentEmptyCells(int32 Row, int32 Col)
{
    TArray<FIntPoint> EmptyCells;
    int32 Directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (int32 i = 0; i < 4; i++)
    {
        int32 NewRow = Row + Directions[i][0];
        int32 NewCol = Col + Directions[i][1];
        
        if (NewRow >= 0 && NewRow < GridRows && NewCol >= 0 && NewCol < GridCols)
        {
            if (!IsBlackCell(NewRow, NewCol))
            {
                AGridPuzzleTile* Tile = GetTile(NewRow, NewCol);
                if (Tile && Tile->GetColorType() == EColorType::Empty)
                {
                    EmptyCells.Add(FIntPoint(NewRow, NewCol));
                }
            }
        }
    }
    
    return EmptyCells;
}

void AGridPuzzleGameMode::SpawnTiles()
{
    if (!TileActorClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<FIntPoint> NonBlackPositions;
    TArray<EColorType> ColorsToPlace;
    
    // Подсчёт количества цветных клеток по зонам
    int32 RedCount = 0, GreenCount = 0, BlueCount = 0;
    
    for (int32 i = 0; i < GridRows; i++)
    {
        for (int32 j = 0; j < GridCols; j++)
        {
            if (IsBlackCell(i, j)) continue;
            
            NonBlackPositions.Add(FIntPoint(i, j));
            EColorType RequiredColor = GetZoneColor(i, j);
            
            if (RequiredColor == EColorType::Red)
                RedCount++;
            else if (RequiredColor == EColorType::Green)
                GreenCount++;
            else if (RequiredColor == EColorType::Blue)
                BlueCount++;
        }
    }
    
    // Добавляем цветные плитки
    for (int32 i = 0; i < RedCount; i++) 
        ColorsToPlace.Add(EColorType::Red);
    for (int32 i = 0; i < GreenCount; i++) 
        ColorsToPlace.Add(EColorType::Green);
    for (int32 i = 0; i < BlueCount; i++) 
        ColorsToPlace.Add(EColorType::Blue);
    
    // Добавляем пустые клетки
    int32 TotalNonBlackCells = NonBlackPositions.Num();
    int32 ColoredCount = ColorsToPlace.Num();
    
    for (int32 i = ColoredCount; i < TotalNonBlackCells; i++)
    {
        ColorsToPlace.Add(EColorType::Empty);
    }
    
    // Перемешиваем цвета
    for (int32 i = ColorsToPlace.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        ColorsToPlace.Swap(i, j);
    }
    
    float OffsetX = (GridCols - 1) * TileSize * 0.5f;
    float OffsetY = (GridRows - 1) * TileSize * 0.5f;
    
    for (int32 idx = 0; idx < NonBlackPositions.Num(); idx++)
    {
        int32 Row = NonBlackPositions[idx].X;
        int32 Col = NonBlackPositions[idx].Y;
        
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
            
            if (Color == EColorType::Empty)
            {
                EmptyRow = Row;
                EmptyCol = Col;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d tiles. Red: %d, Green: %d, Blue: %d"), 
        NonBlackPositions.Num(), RedCount, GreenCount, BlueCount);
}

void AGridPuzzleGameMode::SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB)
{
    AGridPuzzleTile* TileA = GetTile(RowA, ColA);
    AGridPuzzleTile* TileB = GetTile(RowB, ColB);
    
    if (!TileA || !TileB) return;
    
    SetTile(RowA, ColA, TileB);
    SetTile(RowB, ColB, TileA);
    
    TileA->SetGridPosition(RowB, ColB);
    TileB->SetGridPosition(RowA, ColA);
    
    float OffsetX = (GridCols - 1) * TileSize * 0.5f;
    float OffsetY = (GridRows - 1) * TileSize * 0.5f;
    
    FVector NewPosA(ColB * TileSize - OffsetX, RowB * TileSize - OffsetY, 0.0f);
    FVector NewPosB(ColA * TileSize - OffsetX, RowA * TileSize - OffsetY, 0.0f);
    
    TileA->MoveToLocation(NewPosA);
    TileB->MoveToLocation(NewPosB);
}

void AGridPuzzleGameMode::TryMoveTile(int32 Row, int32 Col)
{
    UE_LOG(LogTemp, Warning, TEXT("TryMoveTile called at (%d, %d)"), Row, Col);
    
    if (Row < 0 || Row >= GridRows || Col < 0 || Col >= GridCols)
    {
        UE_LOG(LogTemp, Warning, TEXT("Out of bounds!"));
        return;
    }
    
    if (IsBlackCell(Row, Col))
    {
        UE_LOG(LogTemp, Warning, TEXT("Black cell!"));
        return;
    }
    
    AGridPuzzleTile* Tile = GetTile(Row, Col);
    if (!Tile)
    {
        UE_LOG(LogTemp, Warning, TEXT("No tile at this position!"));
        return;
    }
    
    // Нельзя двигать пустые клетки
    if (Tile->GetColorType() == EColorType::Empty)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot move empty tile!"));
        return;
    }
    
    // Ищем соседнюю пустую клетку
    TArray<FIntPoint> AdjacentEmpty = GetAdjacentEmptyCells(Row, Col);
    
    if (AdjacentEmpty.Num() > 0)
    {
        FIntPoint Target = AdjacentEmpty[0];
        UE_LOG(LogTemp, Warning, TEXT("Moving tile from (%d, %d) to empty cell at (%d, %d)"), 
            Row, Col, Target.X, Target.Y);
        SwapTiles(Row, Col, Target.X, Target.Y);
        CheckVictory();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No empty cell adjacent to (%d, %d)!"), Row, Col);
    }
}

void AGridPuzzleGameMode::CheckVictory()
{
    for (int32 i = 0; i < GridRows; i++)
    {
        for (int32 j = 0; j < GridCols; j++)
        {
            if (IsBlackCell(i, j)) continue;
            
            AGridPuzzleTile* Tile = GetTile(i, j);
            if (!Tile) continue;
            
            EColorType TileColor = Tile->GetColorType();
            if (TileColor == EColorType::Empty) continue;
            
            EColorType RequiredColor = GetZoneColor(i, j);
            
            if (RequiredColor != EColorType::Empty && RequiredColor != EColorType::Black)
            {
                if (TileColor != RequiredColor)
                {
                    return;
                }
            }
        }
    }
    
    OnVictory.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("VICTORY!"));
}

void AGridPuzzleGameMode::ShuffleTiles(int32 MovesCount)
{
    for (int32 Move = 0; Move < MovesCount; Move++)
    {
        TArray<FIntPoint> MovableTiles;
        
        // Находим все цветные клетки, у которых есть соседняя пустая
        for (int32 i = 0; i < GridRows; i++)
        {
            for (int32 j = 0; j < GridCols; j++)
            {
                if (IsBlackCell(i, j)) continue;
                
                AGridPuzzleTile* Tile = GetTile(i, j);
                if (!Tile) continue;
                
                if (Tile->GetColorType() != EColorType::Empty)
                {
                    TArray<FIntPoint> AdjacentEmpty = GetAdjacentEmptyCells(i, j);
                    if (AdjacentEmpty.Num() > 0)
                    {
                        MovableTiles.Add(FIntPoint(i, j));
                    }
                }
            }
        }
        
        if (MovableTiles.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, MovableTiles.Num() - 1);
            FIntPoint TilePos = MovableTiles[RandomIndex];
            TArray<FIntPoint> EmptyCells = GetAdjacentEmptyCells(TilePos.X, TilePos.Y);
            
            if (EmptyCells.Num() > 0)
            {
                FIntPoint EmptyPos = EmptyCells[FMath::RandRange(0, EmptyCells.Num() - 1)];
                SwapTiles(TilePos.X, TilePos.Y, EmptyPos.X, EmptyPos.Y);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Shuffle complete"));
}

void AGridPuzzleGameMode::ResetGame()
{
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
    ShuffleTiles(500);
}