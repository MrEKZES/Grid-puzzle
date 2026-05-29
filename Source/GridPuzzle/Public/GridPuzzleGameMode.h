#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
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
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    int32 GridSize;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    TSubclassOf<AGridPuzzleTile> TileActorClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    TSubclassOf<AActor> ColumnIndicatorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
    float TileSize;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> VictoryWidgetClass;

    FOnVictoryDelegate OnVictory;

    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void TryMoveTile(int32 Row, int32 Col);

    UFUNCTION(BlueprintCallable, Category = "Game Logic")
    void ResetGame();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowVictoryWidget();

protected:
    void ValidateGridSize();
    void InitializeGrid();
    void SpawnTiles();
    void SpawnColumnIndicators();
    void UpdateAllPositions();
    void SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB);
    void CheckVictory();
    EColorType GetColumnColor(int32 ColIndex) const;
    void ShuffleTiles(int32 MovesCount = 500);
    void ProcessMouseClick();

    UPROPERTY()
    TArray<AGridPuzzleTile*> Tiles;
    
    UPROPERTY()
    UUserWidget* VictoryWidget;
    
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
    
    bool bLastClickProcessed;
    bool bGameEnded;
};