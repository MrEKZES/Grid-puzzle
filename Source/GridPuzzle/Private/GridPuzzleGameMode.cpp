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
    GridSize = 3;
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
        }
        
        PlayerController->bShowMouseCursor = true;
        PlayerController->bEnableClickEvents = true;
        PlayerController->bEnableMouseOverEvents = true;
    }
    
    ValidateGridSize();
    InitializeGrid();
    SpawnTiles();
    SpawnColumnIndicators();
    ShuffleTiles(1000);
}

void AGridPuzzleGameMode::ValidateGridSize()
{
    if (GridSize < 2) GridSize = 2;
    if (GridSize > 9) GridSize = 9;
}

bool AGridPuzzleGameMode::IsAdjacent(int32 Row1, int32 Col1, int32 Row2, int32 Col2)
{
    return (FMath::Abs(Row1 - Row2) + FMath::Abs(Col1 - Col2)) == 1;
}

EColorType AGridPuzzleGameMode::GetColumnColor(int32 ColIndex) const
{
    int32 OneBasedCol = ColIndex + 1;
    int32 TotalCols = GridSize;
    
    // Первый столбец - красный
    if (OneBasedCol == 1)
        return EColorType::Red;
    
    // Последний столбец - синий
    if (OneBasedCol == TotalCols)
        return EColorType::Blue;
    
    // Цвета для разных размеров поля
    if (TotalCols == 3)
    {
        if (OneBasedCol == 2) return EColorType::Green;
    }
    else if (TotalCols == 4)
    {
        if (OneBasedCol == 2) return EColorType::Orange;
        if (OneBasedCol == 3) return EColorType::Cyan;
    }
    else if (TotalCols == 5)
    {
        if (OneBasedCol == 2) return EColorType::Orange;
        if (OneBasedCol == 3) return EColorType::Yellow;
        if (OneBasedCol == 4) return EColorType::Cyan;
    }
    else if (TotalCols == 6)
    {
        if (OneBasedCol == 2) return EColorType::Orange;
        if (OneBasedCol == 3) return EColorType::Yellow;
        if (OneBasedCol == 4) return EColorType::Green;
        if (OneBasedCol == 5) return EColorType::Cyan;
    }
    else if (TotalCols == 7)
    {
        if (OneBasedCol == 2) return EColorType::Orange;
        if (OneBasedCol == 3) return EColorType::Yellow;
        if (OneBasedCol == 4) return EColorType::Green;
        if (OneBasedCol == 5) return EColorType::Cyan;
        if (OneBasedCol == 6) return EColorType::Purple;
    }
    else if (TotalCols == 8)
    {
        if (OneBasedCol == 2) return EColorType::Orange;
        if (OneBasedCol == 3) return EColorType::Yellow;
        if (OneBasedCol == 4) return EColorType::Green;
        if (OneBasedCol == 5) return EColorType::Turquoise;
        if (OneBasedCol == 6) return EColorType::Cyan;
        if (OneBasedCol == 7) return EColorType::Purple;
    }
    else if (TotalCols == 9)
    {
        if (OneBasedCol == 2) return EColorType::Orange;
        if (OneBasedCol == 3) return EColorType::Yellow;
        if (OneBasedCol == 4) return EColorType::Green;
        if (OneBasedCol == 5) return EColorType::Turquoise;
        if (OneBasedCol == 6) return EColorType::Cyan;
        if (OneBasedCol == 7) return EColorType::Purple;
        if (OneBasedCol == 8) return EColorType::Pink;
    }
    
    return EColorType::Green;
}

void AGridPuzzleGameMode::InitializeGrid()
{
    int32 TotalTiles = GridSize * GridSize;
    Tiles.SetNum(TotalTiles);
    
    for (int32 i = 0; i < TotalTiles; i++)
    {
        Tiles[i] = nullptr;
    }
}

void AGridPuzzleGameMode::SpawnColumnIndicators()
{
    if (!ColumnIndicatorClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    float OffsetX = (GridSize - 1) * TileSize * 0.5f;
    float OffsetY = (GridSize - 1) * TileSize * 0.5f;
    float IndicatorZ = 30.0f;
    
    for (int32 j = 0; j < GridSize; j++)
    {
        EColorType ColumnColor = GetColumnColor(j);
        
        FLinearColor IndicatorColor = FLinearColor::White;
        switch (ColumnColor)
        {
            case EColorType::Red:       IndicatorColor = FLinearColor(1.0f, 0.0f, 0.0f); break;
            case EColorType::Orange:    IndicatorColor = FLinearColor(1.0f, 0.5f, 0.0f); break;
            case EColorType::Yellow:    IndicatorColor = FLinearColor(1.0f, 1.0f, 0.0f); break;
            case EColorType::Green:     IndicatorColor = FLinearColor(0.0f, 1.0f, 0.0f); break;
            case EColorType::Turquoise: IndicatorColor = FLinearColor(0.6f, 0.3f, 0.0f); break; // Коричневый
            case EColorType::Cyan:      IndicatorColor = FLinearColor(0.0f, 1.0f, 1.0f); break;
            case EColorType::Purple:    IndicatorColor = FLinearColor(0.8f, 0.2f, 1.0f); break;
            case EColorType::Pink:      IndicatorColor = FLinearColor(1.0f, 0.6f, 0.8f); break;
            case EColorType::Blue:      IndicatorColor = FLinearColor(0.0f, 0.0f, 1.0f); break;
            default:                    IndicatorColor = FLinearColor(1.0f, 1.0f, 1.0f); break;
        }
        
        FVector Location(
            j * TileSize - OffsetX,
            -OffsetY - TileSize * 0.8f,
            IndicatorZ
        );
        
        AActor* Indicator = World->SpawnActor<AActor>(ColumnIndicatorClass, Location, FRotator::ZeroRotator);
        if (Indicator)
        {
            Indicator->SetActorScale3D(FVector(0.8f, 0.3f, 0.2f));
            
            UStaticMeshComponent* MeshComp = Indicator->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                UMaterialInstanceDynamic* DynMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
                if (DynMaterial)
                {
                    DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), IndicatorColor);
                }
            }
        }
    }
}

void AGridPuzzleGameMode::SpawnTiles()
{
    if (!TileActorClass) return;
    
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
            
            if (Color == EColorType::Empty)
            {
                EmptyRow = Row;
                EmptyCol = Col;
            }
        }
    }
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
    
    if (ColorA == EColorType::Empty)
    {
        EmptyRow = RowB;
        EmptyCol = ColB;
    }
    else if (ColorB == EColorType::Empty)
    {
        EmptyRow = RowA;
        EmptyCol = ColA;
    }
    
    float OffsetX = (GridSize - 1) * TileSize * 0.5f;
    float OffsetY = (GridSize - 1) * TileSize * 0.5f;
    
    FVector NewPosA(ColB * TileSize - OffsetX, RowB * TileSize - OffsetY, 0.0f);
    FVector NewPosB(ColA * TileSize - OffsetX, RowA * TileSize - OffsetY, 0.0f);
    
    TileA->SetActorLocation(NewPosA);
    TileB->SetActorLocation(NewPosB);
}

void AGridPuzzleGameMode::TryMoveTile(int32 Row, int32 Col)
{
    if (Row < 0 || Row >= GridSize || Col < 0 || Col >= GridSize) return;
    
    AGridPuzzleTile* Tile = GetTile(Row, Col);
    if (!Tile) return;
    
    if (Tile->GetColorType() == EColorType::Empty) return;
    
    if (IsAdjacent(Row, Col, EmptyRow, EmptyCol))
    {
        SwapTiles(Row, Col, EmptyRow, EmptyCol);
        CheckVictory();
    }
}

void AGridPuzzleGameMode::CheckVictory()
{
    for (int32 i = 0; i < GridSize; i++)
    {
        for (int32 j = 0; j < GridSize; j++)
        {
            AGridPuzzleTile* Tile = GetTile(i, j);
            if (!Tile) continue;
            
            EColorType TileColor = Tile->GetColorType();
            if (TileColor == EColorType::Empty) continue;
            
            EColorType RequiredColor = GetColumnColor(j);
            
            if (TileColor != RequiredColor)
            {
                return;
            }
        }
    }
    
    OnVictory.Broadcast();
}

void AGridPuzzleGameMode::ShuffleTiles(int32 MovesCount)
{
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
    SpawnColumnIndicators();
    ShuffleTiles(1000);
}