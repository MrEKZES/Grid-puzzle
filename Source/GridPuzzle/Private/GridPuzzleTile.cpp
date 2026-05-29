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
    
    ColorType = EColorType::Empty;
    GridRow = -1;
    GridCol = -1;
    DynamicMaterial = nullptr;
}

void AGridPuzzleTile::BeginPlay()
{
    Super::BeginPlay();
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
    
    FLinearColor Color = FLinearColor::White;
    
    switch (ColorType)
    {
    case EColorType::Red:       Color = FLinearColor(1.0f, 0.0f, 0.0f); break;
    case EColorType::Orange:    Color = FLinearColor(1.0f, 0.5f, 0.0f); break;
    case EColorType::Yellow:    Color = FLinearColor(1.0f, 1.0f, 0.0f); break;
    case EColorType::Green:     Color = FLinearColor(0.0f, 1.0f, 0.0f); break;
    case EColorType::Turquoise: Color = FLinearColor(0.6f, 0.3f, 0.0f); break;
    case EColorType::Cyan:      Color = FLinearColor(0.0f, 1.0f, 1.0f); break;
    case EColorType::Purple:    Color = FLinearColor(0.8f, 0.2f, 1.0f); break;
    case EColorType::Pink:      Color = FLinearColor(1.0f, 0.6f, 0.8f); break;
    case EColorType::Blue:      Color = FLinearColor(0.0f, 0.0f, 1.0f); break;
    default:                    Color = FLinearColor(1.0f, 1.0f, 1.0f); break;
    }
    
    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}