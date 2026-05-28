#include "GridPuzzleGameMode.h"
#include "GridPuzzleTile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStatics.h"

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

void AGridPuzzleGameMode::InitializeGrid()
{
    Grid.SetNum(GridRows);
    for (int32 i = 0; i < GridRows; i++)
    {
        Grid[i].SetNum(GridCols);
        for (int32 j = 0; j < GridCols; j++)
        {
            Grid[i][j] = nullptr;
        }
    }
}

EColorType AGridPuzzleGameMode::GetZoneColor(int32 ColIndex) const
{
    int32 OneBasedCol = ColIndex + 1;
    
    // Красная зона - первый столбец
    if (OneBasedCol == 1)
        return EColorType::Red;
    
    // Синяя зона - последний столбец
    if (OneBasedCol == GridCols)
        return EColorType::Blue;
    
    // Зелёная зона - средний столбец
    int32 MidCol = (GridCols + 1) / 2;
    if (OneBasedCol == MidCol)
        return EColorType::Green;
    
    return EColorType::Empty;
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
            EColorType ZoneColor = GetZoneColor(j);
            
            if (ZoneColor == EColorType::Red) RedCount++;
            else if (ZoneColor == EColorType::Green) GreenCount++;
            else if (ZoneColor == EColorType::Blue) BlueCount++;
        }
    }
    
    // Добавляем цветные плитки
    for (int32 i = 0; i < RedCount; i++) ColorsToPlace.Add(EColorType::Red);
    for (int32 i = 0; i < GreenCount; i++) ColorsToPlace.Add(EColorType::Green);
    for (int32 i = 0; i < BlueCount; i++) ColorsToPlace.Add(EColorType::Blue);
    
    // Остальные - пустые (белые)
    int32 TotalTiles = NonBlackPositions.Num();
    int32 ColoredCount = ColorsToPlace.Num();
    for (int32 i = ColoredCount; i < TotalTiles - 1; i++)
    {
        ColorsToPlace.Add(EColorType::Empty);
    }
    
    // Одна пустая клетка для перемещений
    ColorsToPlace.Add(EColorType::Empty);
    
    // Перемешиваем цвета
    for (int32 i = ColorsToPlace.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        ColorsToPlace.Swap(i, j);
    }
    
    // Создаём плитки
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
            Grid[Row][Col] = Tile;
            
            if (Color == EColorType::Empty)
            {
                EmptyRow = Row;
                EmptyCol = Col;
            }
        }
    }
}

void AGridPuzzleGameMode::GetEmptyTilePosition(int32& OutRow, int32& OutCol)
{
    OutRow = EmptyRow;
    OutCol = EmptyCol;
}

bool AGridPuzzleGameMode::IsAdjacent(int32 Row1, int32 Col1, int32 Row2, int32 Col2)
{
    return (FMath::Abs(Row1 - Row2) + FMath::Abs(Col1 - Col2)) == 1;
}

void AGridPuzzleGameMode::SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB)
{
    if (!Grid[RowA][ColA] || !Grid[RowB][ColB]) return;
    
    // Swap grid pointers
    AGridPuzzleTile* Temp = Grid[RowA][ColA];
    Grid[RowA][ColA] = Grid[RowB][ColB];
    Grid[RowB][ColB] = Temp;
    
    // Update tile positions
    Grid[RowA][ColA]->SetGridPosition(RowA, ColA);
    Grid[RowB][ColB]->SetGridPosition(RowB, ColB);
    
    // Animate movement
    float OffsetX = (GridCols - 1) * TileSize * 0.5f;
    float OffsetY = (GridRows - 1) * TileSize * 0.5f;
    
    FVector NewPosA(ColA * TileSize - OffsetX, RowA * TileSize - OffsetY, 0.0f);
    FVector NewPosB(ColB * TileSize - OffsetX, RowB * TileSize - OffsetY, 0.0f);
    
    Grid[RowA][ColA]->MoveToLocation(NewPosA);
    Grid[RowB][ColB]->MoveToLocation(NewPosB);
    
    // Update empty tile tracking
    if (Grid[RowA][ColA]->GetColorType() == EColorType::Empty)
    {
        EmptyRow = RowA;
        EmptyCol = ColA;
    }
    else if (Grid[RowB][ColB]->GetColorType() == EColorType::Empty)
    {
        EmptyRow = RowB;
        EmptyCol = ColB;
    }
}

void AGridPuzzleGameMode::TryMoveTile(int32 Row, int32 Col)
{
    // Проверка границ
    if (Row < 0 || Row >= GridRows || Col < 0 || Col >= GridCols) return;
    
    // Проверка на чёрную клетку
    if (IsBlackCell(Row, Col)) return;
    
    // Проверка существования плитки
    if (!Grid[Row][Col]) return;
    
    // Проверка соседства с пустой клеткой
    if (IsAdjacent(Row, Col, EmptyRow, EmptyCol))
    {
        SwapTiles(Row, Col, EmptyRow, EmptyCol);
        CheckVictory();
    }
}

void AGridPuzzleGameMode::CheckVictory()
{
    for (int32 i = 0; i < GridRows; i++)
    {
        for (int32 j = 0; j < GridCols; j++)
        {
            if (IsBlackCell(i, j)) continue;
            
            AGridPuzzleTile* Tile = Grid[i][j];
            if (!Tile) continue;
            
            EColorType TileColor = Tile->GetColorType();
            if (TileColor == EColorType::Empty) continue;
            
            EColorType RequiredColor = GetZoneColor(j);
            if (TileColor != RequiredColor)
            {
                return;
            }
        }
    }
    
    // Победа!
    OnVictory.Broadcast();
    
    // Лог в консоль
    UE_LOG(LogTemp, Warning, TEXT("VICTORY! All tiles are in correct columns!"));
}

void AGridPuzzleGameMode::ShuffleTiles(int32 MovesCount)
{
    // Делаем случайные перемещения для перемешивания
    for (int32 Move = 0; Move < MovesCount; Move++)
    {
        TArray<FIntPoint> AdjacentTiles;
        
        // Ищем соседние с пустой клеткой
        if (EmptyRow > 0 && !IsBlackCell(EmptyRow - 1, EmptyCol))
            AdjacentTiles.Add(FIntPoint(EmptyRow - 1, EmptyCol));
        if (EmptyRow < GridRows - 1 && !IsBlackCell(EmptyRow + 1, EmptyCol))
            AdjacentTiles.Add(FIntPoint(EmptyRow + 1, EmptyCol));
        if (EmptyCol > 0 && !IsBlackCell(EmptyRow, EmptyCol - 1))
            AdjacentTiles.Add(FIntPoint(EmptyRow, EmptyCol - 1));
        if (EmptyCol < GridCols - 1 && !IsBlackCell(EmptyRow, EmptyCol + 1))
            AdjacentTiles.Add(FIntPoint(EmptyRow, EmptyCol + 1));
        
        if (AdjacentTiles.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, AdjacentTiles.Num() - 1);
            FIntPoint Target = AdjacentTiles[RandomIndex];
            SwapTiles(Target.X, Target.Y, EmptyRow, EmptyCol);
        }
    }
}

void AGridPuzzleGameMode::ResetGame()
{
    // Уничтожаем все плитки
    for (int32 i = 0; i < GridRows; i++)
    {
        for (int32 j = 0; j < GridCols; j++)
        {
            if (Grid[i][j])
            {
                Grid[i][j]->Destroy();
                Grid[i][j] = nullptr;
            }
        }
    }
    
    Grid.Empty();
    EmptyRow = -1;
    EmptyCol = -1;
    
    // Создаём заново
    InitializeGrid();
    SpawnTiles();
    ShuffleTiles(500);
}