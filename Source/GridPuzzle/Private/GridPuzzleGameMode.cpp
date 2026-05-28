#include "GridPuzzleGameMode.h"
#include "GridPuzzleTile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
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

EColorType AGridPuzzleGameMode::GetZoneColor(int32 Row, int32 Col) const
{
    // Чёрные клетки возвращаем как Black
    if (IsBlackCell(Row, Col))
        return EColorType::Black;
    
    int32 OneBasedCol = Col + 1;
    
    // Только нечётные столбцы имеют цветные зоны
    if (OneBasedCol % 2 == 1)  // Нечётный столбец
    {
        // Первый столбец (1) - красный
        if (OneBasedCol == 1)
            return EColorType::Red;
        
        // Средний столбец - зелёный
        int32 MidCol = (GridCols + 1) / 2;
        if (OneBasedCol == MidCol)
            return EColorType::Green;
        
        // Последний столбец - синий
        if (OneBasedCol == GridCols)
            return EColorType::Blue;
    }
    
    // Все остальные клетки - пустые (белые)
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

void AGridPuzzleGameMode::SpawnTiles()
{
    if (!TileActorClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<FIntPoint> NonBlackPositions;
    TArray<EColorType> ColorsToPlace;
    
    // Подсчёт количества цветных клеток по зонам
    int32 RedCount = 0, GreenCount = 0, BlueCount = 0;
    
    // Сначала проходим по всем клеткам, чтобы подсчитать количество нужных цветов
    for (int32 i = 0; i < GridRows; i++)
    {
        for (int32 j = 0; j < GridCols; j++)
        {
            // Пропускаем чёрные клетки
            if (IsBlackCell(i, j)) continue;
            
            // Добавляем позицию в список
            NonBlackPositions.Add(FIntPoint(i, j));
            
            // Определяем, какой цвет должен быть в этой клетке при победе
            EColorType RequiredColor = GetZoneColor(i, j);
            
            // Считаем, сколько клеток каждого цвета нужно создать
            if (RequiredColor == EColorType::Red)
                RedCount++;
            else if (RequiredColor == EColorType::Green)
                GreenCount++;
            else if (RequiredColor == EColorType::Blue)
                BlueCount++;
        }
    }
    
    // Добавляем цветные плитки в соответствии с подсчитанным количеством
    for (int32 i = 0; i < RedCount; i++) 
        ColorsToPlace.Add(EColorType::Red);
    for (int32 i = 0; i < GreenCount; i++) 
        ColorsToPlace.Add(EColorType::Green);
    for (int32 i = 0; i < BlueCount; i++) 
        ColorsToPlace.Add(EColorType::Blue);
    
    // Остальные клетки (не цветные зоны) будут пустыми (белыми)
    int32 TotalNonBlackCells = NonBlackPositions.Num();
    int32 ColoredCount = ColorsToPlace.Num();
    
    // Добавляем пустые клетки (на одну меньше, чем нужно, так как одна будет пустой для перемещений)
    for (int32 i = ColoredCount; i < TotalNonBlackCells - 1; i++)
    {
        ColorsToPlace.Add(EColorType::Empty);
    }
    
    // Добавляем последнюю пустую клетку (для возможности перемещений)
    ColorsToPlace.Add(EColorType::Empty);
    
    // Перемешиваем цвета случайным образом
    for (int32 i = ColorsToPlace.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        ColorsToPlace.Swap(i, j);
    }
    
    // Вычисляем смещение для центрирования поля
    float OffsetX = (GridCols - 1) * TileSize * 0.5f;
    float OffsetY = (GridRows - 1) * TileSize * 0.5f;
    
    // Создаём плитки только для не-чёрных клеток
    for (int32 idx = 0; idx < NonBlackPositions.Num(); idx++)
    {
        int32 Row = NonBlackPositions[idx].X;
        int32 Col = NonBlackPositions[idx].Y;
        
        // Вычисляем позицию для спавна
        FVector Location(
            Col * TileSize - OffsetX,
            Row * TileSize - OffsetY,
            0.0f
        );
        
        // Спавним плитку
        AGridPuzzleTile* Tile = World->SpawnActor<AGridPuzzleTile>(TileActorClass, Location, FRotator::ZeroRotator);
        if (Tile)
        {
            EColorType Color = ColorsToPlace[idx];
            Tile->Init(Row, Col, Color);
            SetTile(Row, Col, Tile);
            
            // Запоминаем позицию пустой клетки
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
    AGridPuzzleTile* TileA = GetTile(RowA, ColA);
    AGridPuzzleTile* TileB = GetTile(RowB, ColB);
    
    if (!TileA || !TileB) return;
    
    // Swap in array
    SetTile(RowA, ColA, TileB);
    SetTile(RowB, ColB, TileA);
    
    // Update tile positions
    TileA->SetGridPosition(RowB, ColB);
    TileB->SetGridPosition(RowA, ColA);
    
    // Animate movement
    float OffsetX = (GridCols - 1) * TileSize * 0.5f;
    float OffsetY = (GridRows - 1) * TileSize * 0.5f;
    
    FVector NewPosA(ColB * TileSize - OffsetX, RowB * TileSize - OffsetY, 0.0f);
    FVector NewPosB(ColA * TileSize - OffsetX, RowA * TileSize - OffsetY, 0.0f);
    
    TileA->MoveToLocation(NewPosA);
    TileB->MoveToLocation(NewPosB);
    
    // Update empty tile tracking
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
}

void AGridPuzzleGameMode::TryMoveTile(int32 Row, int32 Col)
{
    // Проверка границ
    if (Row < 0 || Row >= GridRows || Col < 0 || Col >= GridCols) return;
    
    // Проверка на чёрную клетку
    if (IsBlackCell(Row, Col)) return;
    
    // Проверка существования плитки
    AGridPuzzleTile* Tile = GetTile(Row, Col);
    if (!Tile) return;
    
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
            // Пропускаем чёрные клетки
            if (IsBlackCell(i, j)) continue;
            
            AGridPuzzleTile* Tile = GetTile(i, j);
            if (!Tile) continue;
            
            EColorType TileColor = Tile->GetColorType();
            
            // Пустые клетки пропускаем
            if (TileColor == EColorType::Empty) continue;
            
            // Получаем требуемый цвет для этой позиции
            EColorType RequiredColor = GetZoneColor(i, j);
            
            // Если это цветная зона (не Empty и не Black), проверяем соответствие
            if (RequiredColor != EColorType::Empty && RequiredColor != EColorType::Black)
            {
                if (TileColor != RequiredColor)
                {
                    return; // Не победа
                }
            }
        }
    }
    
    // Победа!
    OnVictory.Broadcast();
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
    
    // Создаём заново
    InitializeGrid();
    SpawnTiles();
    ShuffleTiles(500);
}