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
    int32 GridSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    TSubclassOf<AGridPuzzleTile> TileActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    TSubclassOf<AActor> ColumnIndicatorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    float TileSize;

    FOnVictoryDelegate OnVictory;

    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void TryMoveTile(int32 Row, int32 Col);

    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void ResetGame();

protected:
    void ValidateGridSize();
    void InitializeGrid();
    void SpawnTiles();
    void SpawnColumnIndicators();
    bool IsAdjacent(int32 Row1, int32 Col1, int32 Row2, int32 Col2);
    void SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB);
    void CheckVictory();
    EColorType GetColumnColor(int32 ColIndex) const;
    void ShuffleTiles(int32 MovesCount = 1000);

    UPROPERTY()
    TArray<AGridPuzzleTile*> Tiles;
    
    int32 GetIndex(int32 Row, int32 Col) const { return Row * GridSize + Col; }
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