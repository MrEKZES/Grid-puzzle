#include "GridController.h"
#include "GridPuzzleTile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/UserWidget.h"

static AGridController* Instance = nullptr;

AGridController* AGridController::GetInstance()
{
    return Instance;
}

AGridController::AGridController()
{
    PrimaryActorTick.bCanEverTick = false;
    bGameEnded = false;
    EmptyRow = -1;
    EmptyCol = -1;
    VictoryWidget = nullptr;
    
    // Принудительно ограничиваем размеры при создании
    Rows = FMath::Clamp(Rows, MIN_GRID_SIZE, MAX_GRID_SIZE);
    Columns = FMath::Clamp(Columns, MIN_GRID_SIZE, MAX_GRID_SIZE);
}

void AGridController::BeginPlay()
{
    Super::BeginPlay();
    Instance = this;
    
    // Ограничиваем размеры сетки допустимыми значениями
    Rows = FMath::Clamp(Rows, MIN_GRID_SIZE, MAX_GRID_SIZE);
    Columns = FMath::Clamp(Columns, MIN_GRID_SIZE, MAX_GRID_SIZE);
    
    if (!TileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: TileClass is NOT set!"));
        return;
    }
    
    GenerateGrid();
}

void AGridController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AGridController::GenerateGrid()
{
    // Ограничиваем размеры перед генерацией
    Rows = FMath::Clamp(Rows, MIN_GRID_SIZE, MAX_GRID_SIZE);
    Columns = FMath::Clamp(Columns, MIN_GRID_SIZE, MAX_GRID_SIZE);
    
    ClearGrid();
    SpawnTiles();
    SpawnColumnIndicators();
    ShuffleTiles();
}

void AGridController::ClearGrid()
{
    for (AGridPuzzleTile* Tile : Tiles)
    {
        if (Tile)
        {
            Tile->Destroy();
        }
    }
    Tiles.Empty();
    bGameEnded = false;
    EmptyRow = -1;
    EmptyCol = -1;
    
    if (VictoryWidget)
    {
        VictoryWidget->RemoveFromParent();
        VictoryWidget = nullptr;
    }
}

FVector AGridController::GetTileWorldPosition(int32 Row, int32 Col) const
{
    float TotalWidth = Columns * TileSize;
    float TotalHeight = Rows * TileSize;
    float StartX = -TotalWidth / 2.0f + TileSize / 2.0f;
    float StartY = -TotalHeight / 2.0f + TileSize / 2.0f;
    
    return FVector(StartX + Col * TileSize, StartY + Row * TileSize, 0.0f);
}

void AGridController::SpawnTiles()
{
    if (!TileClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Дополнительная проверка перед спавном
    if (Rows < MIN_GRID_SIZE || Rows > MAX_GRID_SIZE ||
        Columns < MIN_GRID_SIZE || Columns > MAX_GRID_SIZE)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid grid size: %dx%d (must be %d-%d)"), 
            Rows, Columns, MIN_GRID_SIZE, MAX_GRID_SIZE);
        return;
    }
    
    // Создаём массив цветов для всех клеток
    TArray<EColorType> ColorsToPlace;
    
    for (int32 Col = 0; Col < Columns; Col++)
    {
        EColorType ColumnColor = GetColumnColor(Col);
        for (int32 Row = 0; Row < Rows; Row++)
        {
            ColorsToPlace.Add(ColumnColor);
        }
    }
    
    // Определяем позицию пустой клетки (последняя)
    int32 TotalTiles = Rows * Columns;
    EmptyRow = Rows - 1;
    EmptyCol = Columns - 1;
    int32 EmptyIndex = GetIndex(EmptyRow, EmptyCol);
    ColorsToPlace[EmptyIndex] = EColorType::Empty;
    
    // Перемешиваем цвета (но пустая клетка остаётся на своём месте)
    for (int32 i = 0; i < TotalTiles; i++)
    {
        if (i == EmptyIndex) continue;
        int32 j = FMath::RandRange(0, TotalTiles - 1);
        if (j == EmptyIndex) continue;
        ColorsToPlace.Swap(i, j);
    }
    
    // Создаём массив тайлов
    Tiles.SetNum(TotalTiles);
    
    // Спавним тайлы
    for (int32 Row = 0; Row < Rows; Row++)
    {
        for (int32 Col = 0; Col < Columns; Col++)
        {
            int32 Index = GetIndex(Row, Col);
            EColorType Color = ColorsToPlace[Index];
            
            // Пустую клетку НЕ спавним
            if (Color == EColorType::Empty)
            {
                Tiles[Index] = nullptr;
                continue;
            }
            
            FVector Location = GetTileWorldPosition(Row, Col);
            AGridPuzzleTile* Tile = World->SpawnActor<AGridPuzzleTile>(TileClass, Location, FRotator::ZeroRotator);
            
            if (Tile)
            {
                Tile->Init(Row, Col, Color);
                Tiles[Index] = Tile;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d tiles for grid %dx%d, empty at (%d,%d)"), 
        TotalTiles - 1, Rows, Columns, EmptyRow, EmptyCol);
}

void AGridController::SpawnColumnIndicators()
{
    if (!ColumnIndicatorClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    float TotalHeight = Rows * TileSize;
    float IndicatorY = TotalHeight / 2.0f + 50.0f;
    float TotalWidth = Columns * TileSize;
    float StartX = -TotalWidth / 2.0f + TileSize / 2.0f;
    
    for (int32 Col = 0; Col < Columns; Col++)
    {
        EColorType ColumnColor = GetColumnColor(Col);
        
        FLinearColor IndicatorColor;
        switch (ColumnColor)
        {
            case EColorType::Red:        IndicatorColor = FLinearColor(1.00f, 0.00f, 0.00f); break;
            case EColorType::Orange:     IndicatorColor = FLinearColor(1.00f, 0.50f, 0.00f); break;
            case EColorType::Yellow:     IndicatorColor = FLinearColor(1.00f, 1.00f, 0.00f); break;
            case EColorType::Green:      IndicatorColor = FLinearColor(0.00f, 1.00f, 0.00f); break;
            case EColorType::Cyan:       IndicatorColor = FLinearColor(0.00f, 1.00f, 1.00f); break;
            case EColorType::Blue:       IndicatorColor = FLinearColor(0.00f, 0.00f, 1.00f); break;
            case EColorType::Purple:     IndicatorColor = FLinearColor(0.20f, 0.00f, 0.20f); break;
            case EColorType::DarkGreen:   IndicatorColor = FLinearColor(0.00f, 0.20f, 0.00f); break;
            case EColorType::Magenta:    IndicatorColor = FLinearColor(1.00f, 0.00f, 1.00f); break;
            default:                     IndicatorColor = FLinearColor(1.00f, 1.00f, 1.00f); break;
        }
        
        FVector Location(StartX + Col * TileSize, IndicatorY, 0.0f);
        
        AActor* Indicator = World->SpawnActor<AActor>(ColumnIndicatorClass, Location, FRotator::ZeroRotator);
        if (Indicator)
        {
            Indicator->SetActorScale3D(FVector(0.8f, 0.3f, 0.2f));
            
            UStaticMeshComponent* MeshComp = Indicator->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(0);
                if (BaseMaterial)
                {
                    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Indicator);
                    DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), FVector(IndicatorColor));
                    MeshComp->SetMaterial(0, DynMaterial);
                }
            }
        }
    }
}

bool AGridController::IsAdjacentToEmpty(int32 Row, int32 Col) const
{
    return (FMath::Abs(Row - EmptyRow) + FMath::Abs(Col - EmptyCol)) == 1;
}

void AGridController::MoveTileToEmpty(AGridPuzzleTile* Tile)
{
    if (!Tile) return;
    
    int32 TileRow = Tile->GetGridRow();
    int32 TileCol = Tile->GetGridCol();
    
    // Обновляем логические позиции
    Tile->SetGridPosition(EmptyRow, EmptyCol);
    
    // Обновляем массив
    SetTile(EmptyRow, EmptyCol, Tile);
    SetTile(TileRow, TileCol, nullptr);
    
    // Анимируем движение в пустую клетку
    FVector TargetPos = GetTileWorldPosition(EmptyRow, EmptyCol);
    Tile->SetTargetPosition(TargetPos);
    
    // Обновляем позицию пустой клетки
    EmptyRow = TileRow;
    EmptyCol = TileCol;
}

void AGridController::TryMoveTile(AGridPuzzleTile* Tile)
{
    if (bGameEnded || !Tile) return;
    
    int32 TileRow = Tile->GetGridRow();
    int32 TileCol = Tile->GetGridCol();
    
    if (IsAdjacentToEmpty(TileRow, TileCol))
    {
        MoveTileToEmpty(Tile);
        CheckVictory();
    }
}

void AGridController::CheckVictory()
{
    bool bVictory = true;
    
    for (int32 Row = 0; Row < Rows; Row++)
    {
        for (int32 Col = 0; Col < Columns; Col++)
        {
            AGridPuzzleTile* Tile = GetTile(Row, Col);
            
            // Пустая клетка всегда в правильном месте (в конце)
            if (!Tile) continue;
            
            if (Tile->GetColorType() != GetColumnColor(Col))
            {
                bVictory = false;
                break;
            }
        }
    }
    
    if (bVictory && !bGameEnded)
    {
        bGameEnded = true;
        UE_LOG(LogTemp, Warning, TEXT("VICTORY!"));
        
        if (VictoryWidgetClass)
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                VictoryWidget = CreateWidget<UUserWidget>(PC, VictoryWidgetClass);
                if (VictoryWidget)
                {
                    VictoryWidget->AddToViewport();
                    PC->bShowMouseCursor = true;
                    FInputModeUIOnly InputMode;
                    InputMode.SetWidgetToFocus(VictoryWidget->TakeWidget());
                    PC->SetInputMode(InputMode);
                }
            }
        }
    }
}

EColorType AGridController::GetColumnColor(int32 ColIndex) const
{
    // Циклически повторяем цвета для колонок больше 8
    int32 ColorIndex = ColIndex % 9;
    
    switch (ColorIndex)
    {
        case 0:  return EColorType::Red;
        case 1:  return EColorType::Orange;
        case 2:  return EColorType::Yellow;
        case 3:  return EColorType::Green;
        case 4:  return EColorType::Cyan;
        case 5:  return EColorType::Blue;
        case 6:  return EColorType::Purple;
        case 7:  return EColorType::DarkGreen;
        case 8:  return EColorType::Magenta;
        default: return EColorType::Empty;
    }
}

void AGridController::ShuffleTiles(int32 MovesCount)
{
    if (EmptyRow == -1 || EmptyCol == -1) return;
    
    for (int32 Move = 0; Move < MovesCount; Move++)
    {
        TArray<AGridPuzzleTile*> Neighbors;
        
        // Собираем соседей пустой клетки
        if (EmptyRow > 0) 
        {
            AGridPuzzleTile* Tile = GetTile(EmptyRow - 1, EmptyCol);
            if (Tile) Neighbors.Add(Tile);
        }
        if (EmptyRow < Rows - 1) 
        {
            AGridPuzzleTile* Tile = GetTile(EmptyRow + 1, EmptyCol);
            if (Tile) Neighbors.Add(Tile);
        }
        if (EmptyCol > 0) 
        {
            AGridPuzzleTile* Tile = GetTile(EmptyRow, EmptyCol - 1);
            if (Tile) Neighbors.Add(Tile);
        }
        if (EmptyCol < Columns - 1) 
        {
            AGridPuzzleTile* Tile = GetTile(EmptyRow, EmptyCol + 1);
            if (Tile) Neighbors.Add(Tile);
        }
        
        if (Neighbors.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, Neighbors.Num() - 1);
            MoveTileToEmpty(Neighbors[RandomIndex]);
        }
    }
}

void AGridController::ResetGame()
{
    ClearGrid();
    GenerateGrid();
}