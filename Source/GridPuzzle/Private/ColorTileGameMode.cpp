#include "ColorTileGameMode.h"
#include "ColorTileActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AColorTileGameMode::AColorTileGameMode()
{
    GridRows = 5;
    GridCols = 5;
    TileSize = 110.0f;
}

void AColorTileGameMode::BeginPlay()
{
    Super::BeginPlay();
    ValidateAndAdjustGridSize();
    InitializeGrid();
    SpawnTiles();
}

void AColorTileGameMode::ValidateAndAdjustGridSize()
{
    if (GridRows < 5) GridRows = 5;
    if (GridCols < 5) GridCols = 5;
    
    int32 MaxSize = FMath::Max(GridRows, GridCols);
    if (MaxSize % 2 == 0) MaxSize++;
    
    GridRows = MaxSize;
    GridCols = MaxSize;
}

void AColorTileGameMode::InitializeGrid()
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

void AColorTileGameMode::SpawnTiles()
{
    if (!TileActorClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<FIntPoint> AvailablePositions;
    TArray<EColorType> ColorsToPlace;
    
    // Determine color zones based on column index (1-based)
    auto GetZoneColor = [this](int32 ColIndex) -> EColorType
    {
        int32 OneBasedCol = ColIndex + 1;
        if (OneBasedCol == 1) return EColorType::Red;
        if (OneBasedCol == GridCols) return EColorType::Blue;
        int32 MidCol = (GridCols + 1) / 2;
        if (OneBasedCol == MidCol) return EColorType::Green;
        return EColorType::Empty;
    };
    
    // Count how many colored tiles per color
    int32 RedCount = 0, GreenCount = 0, BlueCount = 0;
    for (int32 i = 0; i < GridRows; i++)
    {
        for (int32 j = 0; j < GridCols; j++)
        {
            int32 OneBasedRow = i + 1;
            int32 OneBasedCol = j + 1;
            
            // Black cell on even row AND even column
            if (OneBasedRow % 2 == 0 && OneBasedCol % 2 == 0)
            {
                continue; // black cell - no tile here
            }
            
            EColorType ZoneColor = GetZoneColor(j);
            if (ZoneColor != EColorType::Empty)
            {
                AvailablePositions.Add(FIntPoint(i, j));
                
                if (ZoneColor == EColorType::Red) RedCount++;
                else if (ZoneColor == EColorType::Green) GreenCount++;
                else if (ZoneColor == EColorType::Blue) BlueCount++;
            }
            else
            {
                AvailablePositions.Add(FIntPoint(i, j));
            }
        }
    }
    
    // Prepare color distribution for random spawn
    for (int32 i = 0; i < RedCount; i++) ColorsToPlace.Add(EColorType::Red);
    for (int32 i = 0; i < GreenCount; i++) ColorsToPlace.Add(EColorType::Green);
    for (int32 i = 0; i < BlueCount; i++) ColorsToPlace.Add(EColorType::Blue);
    
    // Add empty tiles (white)
    int32 TotalNonBlack = AvailablePositions.Num();
    int32 ColoredCount = ColorsToPlace.Num();
    for (int32 i = ColoredCount; i < TotalNonBlack; i++)
    {
        ColorsToPlace.Add(EColorType::Empty);
    }
    
    // Shuffle colors
    for (int32 i = ColorsToPlace.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        ColorsToPlace.Swap(i, j);
    }
    
    // Spawn tiles
    float HalfSize = TileSize * 0.5f;
    for (int32 idx = 0; idx < AvailablePositions.Num(); idx++)
    {
        int32 Row = AvailablePositions[idx].X;
        int32 Col = AvailablePositions[idx].Y;
        
        FVector Location((Col - GridCols / 2.0f) * TileSize, (Row - GridRows / 2.0f) * TileSize, 0.0f);
        
        AColorTileActor* Tile = World->SpawnActor<AColorTileActor>(TileActorClass, Location, FRotator::ZeroRotator);
        if (Tile)
        {
            Tile->Init(Row, Col, ColorsToPlace[idx]);
            Grid[Row][Col] = Tile;
            
            if (ColorsToPlace[idx] == EColorType::Empty)
            {
                EmptyRow = Row;
                EmptyCol = Col;
            }
        }
    }
    
    // Ensure at least one empty tile exists
    if (EmptyRow == -1 && EmptyCol == -1)
    {
        // Fallback: make last tile empty
        for (int32 i = 0; i < GridRows && EmptyRow == -1; i++)
        {
            for (int32 j = 0; j < GridCols; j++)
            {
                if (Grid[i][j] && Grid[i][j]->ColorType != EColorType::Empty && Grid[i][j]->GetClass())
                {
                    Grid[i][j]->SetColor(EColorType::Empty);
                    EmptyRow = i;
                    EmptyCol = j;
                    break;
                }
            }
        }
    }
}

void AColorTileGameMode::GetEmptyTilePosition(int32& OutRow, int32& OutCol)
{
    OutRow = EmptyRow;
    OutCol = EmptyCol;
}

bool AColorTileGameMode::IsAdjacent(int32 Row1, int32 Col1, int32 Row2, int32 Col2)
{
    return (FMath::Abs(Row1 - Row2) + FMath::Abs(Col1 - Col2)) == 1;
}

void AColorTileGameMode::SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB)
{
    if (!Grid[RowA][ColA] || !Grid[RowB][ColB]) return;
    
    // Swap grid pointers
    AColorTileActor* Temp = Grid[RowA][ColA];
    Grid[RowA][ColA] = Grid[RowB][ColB];
    Grid[RowB][ColB] = Temp;
    
    // Update tile positions
    Grid[RowA][ColA]->SetGridPosition(RowA, ColA);
    Grid[RowB][ColB]->SetGridPosition(RowB, ColB);
    
    // Animate movement
    FVector NewPosA((ColA - GridCols / 2.0f) * TileSize, (RowA - GridRows / 2.0f) * TileSize, 0.0f);
    FVector NewPosB((ColB - GridCols / 2.0f) * TileSize, (RowB - GridRows / 2.0f) * TileSize, 0.0f);
    
    Grid[RowA][ColA]->MoveToLocation(NewPosA);
    Grid[RowB][ColB]->MoveToLocation(NewPosB);
    
    // Update empty tile tracking
    if (Grid[RowA][ColA]->ColorType == EColorType::Empty)
    {
        EmptyRow = RowA;
        EmptyCol = ColA;
    }
    else if (Grid[RowB][ColB]->ColorType == EColorType::Empty)
    {
        EmptyRow = RowB;
        EmptyCol = ColB;
    }
}

void AColorTileGameMode::TryMoveTile(int32 Row, int32 Col, FVector WorldClickLocation)
{
    if (Row < 0 || Row >= GridRows || Col < 0 || Col >= GridCols) return;
    if (!Grid[Row][Col]) return;
    
    // Black cells are not movable
    if ((Row + 1) % 2 == 0 && (Col + 1) % 2 == 0) return;
    
    if (IsAdjacent(Row, Col, EmptyRow, EmptyCol))
    {
        SwapTiles(Row, Col, EmptyRow, EmptyCol);
        CheckVictory();
    }
}

void AColorTileGameMode::CheckVictory()
{
    auto GetZoneColor = [this](int32 ColIndex) -> EColorType
    {
        int32 OneBasedCol = ColIndex + 1;
        if (OneBasedCol == 1) return EColorType::Red;
        if (OneBasedCol == GridCols) return EColorType::Blue;
        int32 MidCol = (GridCols + 1) / 2;
        if (OneBasedCol == MidCol) return EColorType::Green;
        return EColorType::Empty;
    };
    
    for (int32 i = 0; i < GridRows; i++)
    {
        for (int32 j = 0; j < GridCols; j++)
        {
            AColorTileActor* Tile = Grid[i][j];
            if (!Tile) continue;
            
            // Black cells are ignored
            if ((i + 1) % 2 == 0 && (j + 1) % 2 == 0) continue;
            
            if (Tile->ColorType == EColorType::Empty) continue;
            
            EColorType RequiredColor = GetZoneColor(j);
            if (Tile->ColorType != RequiredColor)
            {
                return;
            }
        }
    }
    
    // Victory!
    OnVictory.Broadcast();
}

void AColorTileGameMode::ResetGame()
{
    for (int32 i = 0; i < GridRows; i++)
    {
        for (int32 j = 0; j < GridCols; j++)
        {
            if (Grid[i][j])
            {
                Grid[i][j]->Destroy();
            }
        }
    }
    
    Grid.Empty();
    InitializeGrid();
    SpawnTiles();
}