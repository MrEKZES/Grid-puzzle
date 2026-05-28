#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridPuzzleBorder.generated.h"

UCLASS()
class GRIDPUZZLE_API AGridPuzzleBorder : public AActor
{
	GENERATED_BODY()

public:
	AGridPuzzleBorder();

	void CreateBorder(int32 Rows, int32 Cols, float TileSize, float BorderHeight = 20.0f);

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BorderMesh;
};