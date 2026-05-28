#include "GridPuzzleBorder.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AGridPuzzleBorder::AGridPuzzleBorder()
{
	PrimaryActorTick.bCanEverTick = false;
    
	BorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BorderMesh"));
	RootComponent = BorderMesh;
    
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		BorderMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

void AGridPuzzleBorder::CreateBorder(int32 Rows, int32 Cols, float TileSize, float BorderHeight)
{
	float Width = Cols * TileSize;
	float Height = Rows * TileSize;
	float OffsetX = (Cols - 1) * TileSize * 0.5f;
	float OffsetY = (Rows - 1) * TileSize * 0.5f;
    
	// Просто делаем платформу под игровым полем
	SetActorScale3D(FVector(Width / 100.0f, Height / 100.0f, 1.0f));
	SetActorLocation(FVector(0, 0, -BorderHeight));
    
	// В Production версии здесь можно создать рамку из линий
}