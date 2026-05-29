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
    case EColorType::Red:     Color = FLinearColor(1.0f, 0.0f, 0.0f); break;      // Красный
    case EColorType::Orange:  Color = FLinearColor(1.0f, 0.5f, 0.0f); break;      // Оранжевый
    case EColorType::Yellow:  Color = FLinearColor(1.0f, 1.0f, 0.0f); break;      // Жёлтый
    case EColorType::Green:   Color = FLinearColor(0.0f, 1.0f, 0.0f); break;      // Зелёный
    case EColorType::Cyan:    Color = FLinearColor(0.0f, 1.0f, 1.0f); break;      // Голубой
    case EColorType::Blue:    Color = FLinearColor(0.0f, 0.0f, 1.0f); break;      // Синий
    case EColorType::Purple:  Color = FLinearColor(0.5f, 0.0f, 0.5f); break;      // Фиолетовый
    case EColorType::Pink:    Color = FLinearColor(1.0f, 0.75f, 0.8f); break;     // Розовый
    case EColorType::Magenta: Color = FLinearColor(1.0f, 0.0f, 1.0f); break;      // Пурпурный
    case EColorType::Empty:   Color = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); break; // Тёмно-серый
    default:                  Color = FLinearColor::White; break;
    }
    
    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}