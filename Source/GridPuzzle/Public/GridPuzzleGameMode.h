#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridPuzzleTile.h"
#include "GridPuzzleGameMode.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnVictoryDelegate);

UCLASS()
class GRIDPUZZLE_API AGridPuzzleGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AGridPuzzleGameMode();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    int32 GridRows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    int32 GridCols;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    TSubclassOf<AGridPuzzleTile> TileActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    float TileSize;

    FOnVictoryDelegate OnVictory;

    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void TryMoveTile(int32 Row, int32 Col);

    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void ResetGame();

    UFUNCTION(BlueprintPure, Category = "Game Logic")
    bool IsBlackCell(int32 Row, int32 Col) const;

protected:
    void ValidateGridSize();
    void InitializeGrid();
    void SpawnTiles();
    void GetEmptyTilePosition(int32& OutRow, int32& OutCol);
    bool IsAdjacent(int32 Row1, int32 Col1, int32 Row2, int32 Col2);
    void SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB);
    void CheckVictory();
    EColorType GetZoneColor(int32 Row, int32 Col) const;  // ← Изменено: теперь принимает Row и Col
    void ShuffleTiles(int32 MovesCount = 1000);

    // Используем одномерный массив для простоты
    UPROPERTY()
    TArray<AGridPuzzleTile*> Tiles;
    
    // Вспомогательные методы для доступа к плиткам
    int32 GetIndex(int32 Row, int32 Col) const { return Row * GridCols + Col; }
    AGridPuzzleTile* GetTile(int32 Row, int32 Col) const 
    { 
        int32 Index = GetIndex(Row, Col);
        return (Tiles.IsValidIndex(Index)) ? Tiles[Index] : nullptr; 
    }
    void SetTile(int32 Row, int32 Col, AGridPuzzleTile* Tile) 
    { 
        Tiles[GetIndex(Row, Col)] = Tile; 
    }

    int32 EmptyRow;
    int32 EmptyCol;
};