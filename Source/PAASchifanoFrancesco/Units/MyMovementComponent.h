// Creato da: Schifano Francesco 5469994

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PAASchifanoFrancesco/Grid/Tile.h"
#include "MyMovementComponent.generated.h"

/**
 * Delegato dinamico che notifica il completamento del movimento di un'unità.
 * Viene utilizzato da altri manager (es. PlayerController o BattleManager).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovementCompleted);

/**
 * Descrizione:
 * Componente associato a una unità (`AUnitBase`) che gestisce il movimento
 * lungo un percorso definito (array di `ATile*`). Il movimento è interpolato
 * in modo fluido e lineare.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API UMyMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Costruttore di default. Imposta tick abilitato e variabili iniziali */
	UMyMovementComponent();

	/**
	 * Avvia il movimento lungo il percorso specificato con una determinata velocità.
	 * @param Path - Lista di tile da seguire
	 * @param Speed - Velocità costante del movimento
	 */
	void StartMovement(const TArray<ATile*>& Path, float Speed);

	/** Delegato notificato alla fine del movimento */
	UPROPERTY(BlueprintAssignable)
	FOnMovementCompleted OnMovementCompleted;

protected:

	/** Metodo chiamato automaticamente quando il gioco inizia */
	virtual void BeginPlay() override;

	/** Tick eseguito ogni frame per aggiornare la posizione dell’unità */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	/** Percorso da seguire, array di tile */
	TArray<ATile*> MovementPath;

	/** Velocità di movimento costante */
	float MovementSpeed;

	/** Indice della tile target attualmente in corso */
	int32 CurrentTargetIndex;

	/** Indica se un movimento è attualmente in corso */
	bool bIsMoving;
};