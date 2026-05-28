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
    SelectedRow = -1;
    SelectedCol = -1;
    bHasSelectedTile = false;
    CurrentDirectionIndex = 0;
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
    
    AGridPuzzleTile* Tile = GetTile(Row, Col);
    return (Tile && Tile->GetColorType() == EColorType::Empty);
}

EColorType AGridPuzzleGameMode::GetZoneColor(int32 Row, int32 Col) const
{
    if (IsBlackCell(Row, Col))
        return EColorType::Black;
    
    int32 OneBasedCol = Col + 1;
    int32 OneBasedRow = Row + 1;
    
    if (OneBasedCol % 2 == 0 && OneBasedRow % 2 == 1)
        return EColorType::Empty;
    
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
    int32 Directions[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    
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

bool AGridPuzzleGameMode::IsAdjacentToTile(int32 EmptyRow, int32 EmptyCol, int32 TileRow, int32 TileCol)
{
    return (FMath::Abs(EmptyRow - TileRow) + FMath::Abs(EmptyCol - TileCol)) == 1;
}

void AGridPuzzleGameMode::HighlightTile(int32 Row, int32 Col, bool bHighlight)
{
    AGridPuzzleTile* Tile = GetTile(Row, Col);
    if (Tile)
    {
        Tile->SetHighlight(bHighlight);
    }
}

void AGridPuzzleGameMode::SelectTile(int32 Row, int32 Col)
{
    TArray<FIntPoint> AdjacentEmpty = GetAdjacentEmptyCells(Row, Col);
    
    if (AdjacentEmpty.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot select tile at (%d, %d) - no adjacent empty cells!"), Row, Col);
        return;
    }
    
    if (bHasSelectedTile)
    {
        AGridPuzzleTile* OldTile = GetTile(SelectedRow, SelectedCol);
        if (OldTile)
        {
            OldTile->SetHighlight(false);
        }
        for (FIntPoint Pos : CurrentAvailableEmptyCells)
        {
            HighlightTile(Pos.X, Pos.Y, false);
        }
    }
    
    SelectedRow = Row;
    SelectedCol = Col;
    bHasSelectedTile = true;
    CurrentDirectionIndex = 0;
    
    AGridPuzzleTile* NewTile = GetTile(SelectedRow, SelectedCol);
    if (NewTile)
    {
        NewTile->SetHighlight(true);
    }
    
    UpdateCurrentDirection(SelectedRow, SelectedCol);
    
    UE_LOG(LogTemp, Log, TEXT("Tile selected at (%d, %d) with %d adjacent empty cells"), 
        Row, Col, AdjacentEmpty.Num());
}

void AGridPuzzleGameMode::ClearSelection()
{
    if (bHasSelectedTile)
    {
        AGridPuzzleTile* SelectedTile = GetTile(SelectedRow, SelectedCol);
        if (SelectedTile)
        {
            SelectedTile->SetHighlight(false);
        }
        
        for (FIntPoint Pos : CurrentAvailableEmptyCells)
        {
            HighlightTile(Pos.X, Pos.Y, false);
        }
        
        bHasSelectedTile = false;
        SelectedRow = -1;
        SelectedCol = -1;
        CurrentAvailableEmptyCells.Empty();
        CurrentDirectionIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("Selection cleared"));
    }
}

void AGridPuzzleGameMode::UpdateCurrentDirection(int32 Row, int32 Col)
{
    CurrentAvailableEmptyCells = GetAdjacentEmptyCells(Row, Col);
    
    if (CurrentAvailableEmptyCells.Num() > 0)
    {
        if (CurrentDirectionIndex >= CurrentAvailableEmptyCells.Num())
        {
            CurrentDirectionIndex = 0;
        }
        
        for (int32 i = 0; i < CurrentAvailableEmptyCells.Num(); i++)
        {
            if (i == CurrentDirectionIndex)
            {
                HighlightTile(CurrentAvailableEmptyCells[i].X, CurrentAvailableEmptyCells[i].Y, true);
            }
            else
            {
                HighlightTile(CurrentAvailableEmptyCells[i].X, CurrentAvailableEmptyCells[i].Y, false);
            }
        }
    }
}

void AGridPuzzleGameMode::RotateDirection()
{
    if (!bHasSelectedTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("No tile selected, cannot rotate direction"));
        return;
    }
    
    if (CurrentAvailableEmptyCells.Num() <= 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Only one direction available, cannot rotate"));
        return;
    }
    
    CurrentDirectionIndex++;
    if (CurrentDirectionIndex >= CurrentAvailableEmptyCells.Num())
    {
        CurrentDirectionIndex = 0;
    }
    
    UpdateCurrentDirection(SelectedRow, SelectedCol);
    
    UE_LOG(LogTemp, Log, TEXT("Direction rotated to index %d"), CurrentDirectionIndex);
}

void AGridPuzzleGameMode::SpawnTiles()
{
    if (!TileActorClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<FIntPoint> NonBlackPositions;
    TArray<EColorType> ColorsToPlace;
    
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
    
    for (int32 i = 0; i < RedCount; i++) 
        ColorsToPlace.Add(EColorType::Red);
    for (int32 i = 0; i < GreenCount; i++) 
        ColorsToPlace.Add(EColorType::Green);
    for (int32 i = 0; i < BlueCount; i++) 
        ColorsToPlace.Add(EColorType::Blue);
    
    int32 TotalNonBlackCells = NonBlackPositions.Num();
    int32 ColoredCount = ColorsToPlace.Num();
    
    for (int32 i = ColoredCount; i < TotalNonBlackCells; i++)
    {
        ColorsToPlace.Add(EColorType::Empty);
    }
    
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
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d tiles"), NonBlackPositions.Num());
}

void AGridPuzzleGameMode::SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB)
{
    AGridPuzzleTile* TileA = GetTile(RowA, ColA);
    AGridPuzzleTile* TileB = GetTile(RowB, ColB);
    
    if (!TileA || !TileB) return;
    
    EColorType ColorA = TileA->GetColorType();
    EColorType ColorB = TileB->GetColorType();
    
    SetTile(RowA, ColA, TileB);
    SetTile(RowB, ColB, TileA);
    
    TileA->SetGridPosition(RowB, ColB);
    TileB->SetGridPosition(RowA, ColA);
    
    TileA->SetColor(ColorB);
    TileB->SetColor(ColorA);
    
    TileA->SetHighlight(false);
    TileB->SetHighlight(false);
    
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
    
    if (bHasSelectedTile)
    {
        if (Tile->GetColorType() == EColorType::Empty)
        {
            if (CurrentDirectionIndex < CurrentAvailableEmptyCells.Num())
            {
                FIntPoint Target = CurrentAvailableEmptyCells[CurrentDirectionIndex];
                if (Target.X == Row && Target.Y == Col)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Moving selected tile from (%d, %d) to empty cell at (%d, %d)"), 
                        SelectedRow, SelectedCol, Row, Col);
                    SwapTiles(SelectedRow, SelectedCol, Row, Col);
                    ClearSelection();
                    CheckVictory();
                    return;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Not the selected direction! Use RMB to rotate"));
                    return;
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Selecting new tile"));
            SelectTile(Row, Col);
            return;
        }
    }
    
    if (Tile->GetColorType() != EColorType::Empty)
    {
        SelectTile(Row, Col);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Clicked empty tile with no selection - nothing happens"));
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
    
    ClearSelection();
    OnVictory.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("VICTORY!"));
}

void AGridPuzzleGameMode::ShuffleTiles(int32 MovesCount)
{
    for (int32 Move = 0; Move < MovesCount; Move++)
    {
        TArray<FIntPoint> MovableTiles;
        
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
    
    ClearSelection();
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
    ClearSelection();
    
    InitializeGrid();
    SpawnTiles();
    ShuffleTiles(500);
}