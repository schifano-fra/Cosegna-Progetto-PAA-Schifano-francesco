#pragma once

#include "CoreMinimal.h"
#include "UnitBase.h"
#include "Brawler.generated.h"

/**
 * Classe ABrawler
 * Rappresenta l’unità corpo a corpo controllata dal giocatore o dall’IA.
 * Estende la classe AUnitBase e personalizza mesh, materiale, e statistiche.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API ABrawler : public AUnitBase
{
	GENERATED_BODY()

public:
	ABrawler();

protected:
	virtual void BeginPlay() override;

private:
	// Puntatori alle risorse Mesh e Material
	UStaticMesh* Mesh;
	UMaterialInterface* Material;
};
