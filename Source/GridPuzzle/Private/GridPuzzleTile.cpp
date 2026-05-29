#include "GridPuzzleTile.h"
#include "GridPuzzleGameMode.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

AGridPuzzleTile::AGridPuzzleTile()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = MeshComponent;
    
    MeshComponent->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.1f));
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    
    ColorType = EColorType::Empty;
    GridRow = -1;
    GridCol = -1;
    DynamicMaterial = nullptr;
    bIsMoving = false;
    MoveSpeed = 800.0f;
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
    
    // Проверяем, есть ли уже динамический материал
    if (!DynamicMaterial)
    {
        // Пробуем получить материал из MeshComponent
        UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
        if (BaseMaterial)
        {
            DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("No material found on MeshComponent!"));
            return;
        }
    }
    
    if (!DynamicMaterial) return;
    
    FLinearColor Color;
    switch (ColorType)
    {
    case EColorType::Red:        Color = FLinearColor(1.00f, 0.00f, 0.00f); break;  // Чистый красный
    case EColorType::Orange:     Color = FLinearColor(1.00f, 0.40f, 0.00f); break;  // Чистый оранжевый
    case EColorType::Yellow:     Color = FLinearColor(1.00f, 1.00f, 0.00f); break;  // Чистый жёлтый
    case EColorType::Green:      Color = FLinearColor(0.00f, 1.00f, 0.00f); break;  // Чистый зелёный
    case EColorType::Cyan:       Color = FLinearColor(0.00f, 1.00f, 1.00f); break;  // Чистый голубой
    case EColorType::Blue:       Color = FLinearColor(0.00f, 0.00f, 0.80f); break;  // Чистый синий
    case EColorType::Purple:     Color = FLinearColor(0.50f, 0.00f, 1.00f); break;  // Чистый фиолетовый
    case EColorType::DarkBlue:   Color = FLinearColor(0.00f, 0.00f, 0.30f); break;  // Тёмно-синий
    case EColorType::Magenta:    Color = FLinearColor(1.00f, 0.00f, 1.00f); break;  // Чистый пурпурный
    case EColorType::Empty:      Color = FLinearColor(0.10f, 0.10f, 0.10f); break;  // Почти чёрный
    default:                     Color = FLinearColor(1.00f, 1.00f, 1.00f); break;  // Белый
    }
    
    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}