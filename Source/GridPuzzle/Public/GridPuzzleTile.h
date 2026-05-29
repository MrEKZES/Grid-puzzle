#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GridPuzzleTile.generated.h"

UENUM(BlueprintType)
enum class EColorType : uint8
{
	Empty UMETA(DisplayName = "White"),
	Red UMETA(DisplayName = "Red"),
	Orange UMETA(DisplayName = "Orange"),
	Yellow UMETA(DisplayName = "Yellow"),
	Green UMETA(DisplayName = "Green"),
	Turquoise UMETA(DisplayName = "Turquoise"),
	Cyan UMETA(DisplayName = "Cyan"),
	Purple UMETA(DisplayName = "Purple"),
	Pink UMETA(DisplayName = "Pink"),
	Blue UMETA(DisplayName = "Blue")
};

UCLASS()
class GRIDPUZZLE_API AGridPuzzleTile : public AActor
{
	GENERATED_BODY()

public:
	AGridPuzzleTile();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ClickCollision;

	void Init(int32 Row, int32 Col, EColorType Color);
	void SetColor(EColorType NewColor);
	void SetGridPosition(int32 NewRow, int32 NewCol);
	void MoveToLocation(const FVector& NewLocation);
    
	UFUNCTION(BlueprintPure, Category = "Tile")
	EColorType GetColorType() const { return ColorType; }
    
	UFUNCTION(BlueprintPure, Category = "Tile")
	int32 GetGridRow() const { return GridRow; }
    
	UFUNCTION(BlueprintPure, Category = "Tile")
	int32 GetGridCol() const { return GridCol; }

	UFUNCTION()
	void OnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

protected:
	void UpdateMaterialColor();

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;

	UPROPERTY()
	EColorType ColorType;

	UPROPERTY()
	int32 GridRow;

	UPROPERTY()
	int32 GridCol;
};