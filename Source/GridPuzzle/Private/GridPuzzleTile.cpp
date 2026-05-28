#include "GridPuzzleTile.h"
#include "GridPuzzleGameMode.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

AGridPuzzleTile::AGridPuzzleTile()
{
    PrimaryActorTick.bCanEverTick = false;
    
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = MeshComponent;
    
    ClickCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ClickCollision"));
    ClickCollision->SetupAttachment(RootComponent);
    ClickCollision->SetBoxExtent(FVector(50.0f, 50.0f, 10.0f));
    ClickCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ClickCollision->SetCollisionObjectType(ECC_WorldDynamic);
    ClickCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    ClickCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    ClickCollision->OnClicked.AddDynamic(this, &AGridPuzzleTile::OnClicked);
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMesh.Object);
    }
    
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (BaseMaterial.Succeeded())
    {
        MeshComponent->SetMaterial(0, BaseMaterial.Object);
    }
    
    ColorType = EColorType::Empty;
    GridRow = -1;
    GridCol = -1;
}

void AGridPuzzleTile::BeginPlay()
{
    Super::BeginPlay();
    UpdateMaterialColor();
}

void AGridPuzzleTile::Init(int32 Row, int32 Col, EColorType Color)
{
    GridRow = Row;
    GridCol = Col;
    ColorType = Color;
    UpdateMaterialColor();
}

void AGridPuzzleTile::SetColor(EColorType NewColor)
{
    ColorType = NewColor;
    UpdateMaterialColor();
}

void AGridPuzzleTile::SetGridPosition(int32 NewRow, int32 NewCol)
{
    GridRow = NewRow;
    GridCol = NewCol;
}

void AGridPuzzleTile::MoveToLocation(const FVector& NewLocation)
{
    SetActorLocation(NewLocation);
}

void AGridPuzzleTile::OnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    AGridPuzzleGameMode* GameMode = Cast<AGridPuzzleGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode)
    {
        GameMode->TryMoveTile(GridRow, GridCol);
    }
}

void AGridPuzzleTile::UpdateMaterialColor()
{
    if (!MeshComponent) return;
    
    DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
    if (!DynamicMaterial) return;
    
    FLinearColor Color;
    switch (ColorType)
    {
        case EColorType::Red:    Color = FLinearColor::Red; break;
        case EColorType::Green:  Color = FLinearColor::Green; break;
        case EColorType::Blue:   Color = FLinearColor::Blue; break;
        default:                 Color = FLinearColor::White; break;
    }
    
    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}