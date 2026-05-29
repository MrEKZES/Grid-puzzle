#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GridPuzzleTile.generated.h"

UENUM(BlueprintType)
enum class EColorType : uint8
{
	Empty   UMETA(DisplayName = "Empty"),
	Red     UMETA(DisplayName = "Red"),
	Orange  UMETA(DisplayName = "Orange"),
	Yellow  UMETA(DisplayName = "Yellow"),
	Green   UMETA(DisplayName = "Green"),
	Cyan    UMETA(DisplayName = "Cyan"),
	Blue    UMETA(DisplayName = "Blue"),
	Purple  UMETA(DisplayName = "Purple"),
	DarkBlue    UMETA(DisplayName = "DarkBlue"),
	Magenta UMETA(DisplayName = "Magenta"),
	White   UMETA(DisplayName = "White")
};

UCLASS()
class GRIDPUZZLE_API AGridPuzzleTile : public AActor
{
	GENERATED_BODY()

public:
	AGridPuzzleTile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	void Init(int32 Row, int32 Col, EColorType Color);
	void SetColor(EColorType NewColor);
	void SetGridPosition(int32 NewRow, int32 NewCol);
	void SetTargetPosition(const FVector& NewTarget);
	bool IsMoving() const { return bIsMoving; }
    
	UFUNCTION(BlueprintPure, Category = "Tile")
	EColorType GetColorType() const { return ColorType; }
    
	UFUNCTION(BlueprintPure, Category = "Tile")
	int32 GetGridRow() const { return GridRow; }
    
	UFUNCTION(BlueprintPure, Category = "Tile")
	int32 GetGridCol() const { return GridCol; }

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

	UPROPERTY()
	bool bIsMoving;

	UPROPERTY()
	FVector TargetLocation;

	UPROPERTY()
	float MoveSpeed;
};