#pragma once

#include "CoreMinimal.h"
#include "UnitBase.h"
#include "Sniper.generated.h"

UCLASS()
class PAASCHIFANOFRANCESCO_API ASniper : public AUnitBase
{
	GENERATED_BODY()

public:
	ASniper();

protected:
	virtual void BeginPlay() override;

private:
	// Puntatori alle risorse Mesh e Material
	UStaticMesh* Mesh;
	UMaterialInterface* Material;
};
