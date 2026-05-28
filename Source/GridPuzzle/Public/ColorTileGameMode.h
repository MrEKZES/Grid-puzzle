#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ColorTileGameMode.generated.h"

UCLASS()
class COLORPUZZLE_API AColorTileGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AColorTileGameMode();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
	int32 GridRows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
	int32 GridCols;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
	TSubclassOf<class AColorTileActor> TileActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Setup")
	float TileSize;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSimpleMulticastDelegate OnVictory;

	UFUNCTION(BlueprintCallable, Category = "Game Logic")
	void TryMoveTile(int32 Row, int32 Col, FVector WorldClickLocation);

	UFUNCTION(BlueprintCallable, Category = "Game Logic")
	void ResetGame();

protected:
	void InitializeGrid();
	void SpawnTiles();
	void GetEmptyTilePosition(int32& OutRow, int32& OutCol);
	bool IsAdjacent(int32 Row1, int32 Col1, int32 Row2, int32 Col2);
	void SwapTiles(int32 RowA, int32 ColA, int32 RowB, int32 ColB);
	void CheckVictory();
	bool IsTileInCorrectColumn(class AColorTileActor* Tile, int32 Col);

	UPROPERTY()
	TArray<TArray<AColorTileActor*>> Grid;

	int32 EmptyRow;
	int32 EmptyCol;
};