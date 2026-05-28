#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridPuzzleGameMode.generated.h"

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
	TSubclassOf<class AGridPuzzleTile> TileActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
	float TileSize;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSimpleMulticastDelegate OnVictory;

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
	EColorType GetZoneColor(int32 ColIndex) const;
	void ShuffleTiles(int32 MovesCount = 1000);

	UPROPERTY()
	TArray<TArray<AGridPuzzleTile*>> Grid;

	int32 EmptyRow;
	int32 EmptyCol;
};