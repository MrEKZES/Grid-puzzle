#include "GridPuzzleTile.h"
#include "GridController.h"
#include "Materials/MaterialInstanceDynamic.h"

AGridPuzzleTile::AGridPuzzleTile()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = MeshComponent;
    
    if (MeshComponent)
    {
        MeshComponent->SetRelativeScale3D(FVector(0.95f, 0.95f, 0.1f));
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
    
    ColorType = EColorType::Empty;
    GridRow = -1;
    GridCol = -1;
    DynamicMaterial = nullptr;
    bIsMoving = false;
    MoveSpeed = 1000.0f;
}

void AGridPuzzleTile::BeginPlay()
{
    Super::BeginPlay();
    UpdateMaterialColor();
}

void AGridPuzzleTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMoving)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
        SetActorLocation(NewLocation);
        
        if ((NewLocation - TargetLocation).Size() < 1.0f)
        {
            SetActorLocation(TargetLocation);
            bIsMoving = false;
        }
    }
}

void AGridPuzzleTile::NotifyActorOnClicked(FKey ButtonPressed)
{
    Super::NotifyActorOnClicked(ButtonPressed);
    
    AGridController* Controller = AGridController::GetInstance();
    if (Controller)
    {
        Controller->TryMoveTile(this);
    }
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

void AGridPuzzleTile::SetTargetPosition(const FVector& NewTarget)
{
    TargetLocation = NewTarget;
    bIsMoving = true;
}

void AGridPuzzleTile::UpdateMaterialColor()
{
    if (!MeshComponent) return;
    
    if (!DynamicMaterial)
    {
        UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
        if (!BaseMaterial)
        {
            BaseMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
        }
        
        if (BaseMaterial)
        {
            DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
    
    if (!DynamicMaterial) return;
    
    FLinearColor Color;
    switch (ColorType)
    {
    case EColorType::Red:        Color = FLinearColor(1.00f, 0.00f, 0.00f); break;
    case EColorType::Orange:     Color = FLinearColor(1.00f, 0.50f, 0.00f); break;
    case EColorType::Yellow:     Color = FLinearColor(1.00f, 1.00f, 0.00f); break;
    case EColorType::Green:      Color = FLinearColor(0.00f, 1.00f, 0.00f); break;
    case EColorType::Cyan:       Color = FLinearColor(0.00f, 1.00f, 1.00f); break;
    case EColorType::Blue:       Color = FLinearColor(0.00f, 0.00f, 1.00f); break;
    case EColorType::Purple:     Color = FLinearColor(0.20f, 0.00f, 0.20f); break;
    case EColorType::DarkGreen:   Color = FLinearColor(0.00f, 0.20f, 0.00f); break;
    case EColorType::Magenta:    Color = FLinearColor(1.00f, 0.00f, 1.00f); break;
    default:                     Color = FLinearColor(1.00f, 1.00f, 1.00f); break;
    }
    
    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}