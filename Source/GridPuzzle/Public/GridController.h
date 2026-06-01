#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridPuzzleTile.h"
#include "GridController.generated.h"

UCLASS()
class GRIDPUZZLE_API AGridController : public AActor
{
    GENERATED_BODY()

public:
    AGridController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    // Статическая функция для получения контроллера
    static AGridController* GetInstance();
    
    // Константы для ограничения размера сетки
    static constexpr int32 MIN_GRID_SIZE = 2;
    static constexpr int32 MAX_GRID_SIZE = 9;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 Rows = 3;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 Columns = 3;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float TileSize = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    TSubclassOf<AGridPuzzleTile> TileClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    TSubclassOf<AActor> ColumnIndicatorClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> VictoryWidgetClass;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void GenerateGrid();
    
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void ClearGrid();
    
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void TryMoveTile(AGridPuzzleTile* Tile);
    
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void ResetGame();

    // Получить позицию в мире для клетки
    FVector GetTileWorldPosition(int32 Row, int32 Col) const;

protected:
    void SpawnTiles();
    void SpawnColumnIndicators();
    void CheckVictory();
    EColorType GetColumnColor(int32 ColIndex) const;
    void ShuffleTiles(int32 MovesCount = 500);
    bool IsAdjacentToEmpty(int32 Row, int32 Col) const;
    void MoveTileToEmpty(AGridPuzzleTile* Tile);

    // 2D массив тайлов
    UPROPERTY()
    TArray<AGridPuzzleTile*> Tiles;
    
    // Позиция пустой клетки
    int32 EmptyRow;
    int32 EmptyCol;
    
    UPROPERTY()
    UUserWidget* VictoryWidget;
    
    bool bGameEnded;
    
    // Вспомогательные функции
    int32 GetIndex(int32 Row, int32 Col) const { return Row * Columns + Col; }
    AGridPuzzleTile* GetTile(int32 Row, int32 Col) const
    {
        int32 Index = GetIndex(Row, Col);
        return Tiles.IsValidIndex(Index) ? Tiles[Index] : nullptr;
    }
    void SetTile(int32 Row, int32 Col, AGridPuzzleTile* Tile)
    {
        int32 Index = GetIndex(Row, Col);
        if (Tiles.IsValidIndex(Index))
        {
            Tiles[Index] = Tile;
        }
    }
};