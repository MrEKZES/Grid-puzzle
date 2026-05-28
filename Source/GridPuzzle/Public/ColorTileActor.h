#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "ColorTileActor.generated.h"

UENUM(BlueprintType)
enum class EColorType : uint8
{
	Empty UMETA(DisplayName = "White"),
	Red UMETA(DisplayName = "Red"),
	Green UMETA(DisplayName = "Green"),
	Blue UMETA(DisplayName = "Blue"),
	Black UMETA(DisplayName = "Black")
};

UCLASS()
class COLORPUZZLE_API AColorTileActor : public AActor
{
	GENERATED_BODY()

public:
	AColorTileActor();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ClickCollision;

	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	EColorType ColorType;

	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	int32 GridRow;

	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	int32 GridCol;

	void Init(int32 Row, int32 Col, EColorType Color);
	void SetColor(EColorType NewColor);
	void SetGridPosition(int32 NewRow, int32 NewCol);
	void MoveToLocation(const FVector& NewLocation);

	UFUNCTION()
	void OnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

protected:
	void UpdateMaterialColor();

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;
};