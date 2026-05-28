#include "GridPuzzleTile.h"
#include "GridPuzzleGameMode.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

AGridPuzzleTile::AGridPuzzleTile()
{
    PrimaryActorTick.bCanEverTick = false;  // ← Устанавливаем false, так как Tick не нужен
    
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = MeshComponent;
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMesh.Object);
    }
    
    MeshComponent->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.1f));
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    ClickCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ClickCollision"));
    ClickCollision->SetupAttachment(RootComponent);
    ClickCollision->SetBoxExtent(FVector(50.0f, 50.0f, 5.0f));
    ClickCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ClickCollision->SetCollisionObjectType(ECC_WorldDynamic);
    ClickCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    ClickCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    ClickCollision->OnClicked.AddDynamic(this, &AGridPuzzleTile::OnClicked);
    
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (BaseMaterialFinder.Succeeded())
    {
        MeshComponent->SetMaterial(0, BaseMaterialFinder.Object);
    }
    
    ColorType = EColorType::Empty;
    GridRow = -1;
    GridCol = -1;
    DynamicMaterial = nullptr;
}

void AGridPuzzleTile::BeginPlay()
{
    Super::BeginPlay();
    // Не вызываем UpdateMaterialColor здесь
}

void AGridPuzzleTile::Init(int32 Row, int32 Col, EColorType Color)
{
    GridRow = Row;
    GridCol = Col;
    ColorType = Color;
    UpdateMaterialColor();
    
    UE_LOG(LogTemp, Log, TEXT("Tile INIT at (%d, %d) with color %d"), Row, Col, (int32)Color);
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
    
    if (!DynamicMaterial)
    {
        DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
        if (!DynamicMaterial) return;
        UE_LOG(LogTemp, Log, TEXT("Dynamic material created for tile at (%d, %d)"), GridRow, GridCol);
    }
    
    FLinearColor Color = FLinearColor::White;
    switch (ColorType)
    {
        case EColorType::Red:    Color = FLinearColor::Red;   break;
        case EColorType::Green:  Color = FLinearColor::Green; break;
        case EColorType::Blue:   Color = FLinearColor::Blue;  break;
        case EColorType::Black:  Color = FLinearColor::Black; break;
        default:                 Color = FLinearColor::White; break;
    }
    
    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
    UE_LOG(LogTemp, Log, TEXT("Color set for tile at (%d, %d)"), GridRow, GridCol);
}