#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

/**
 * Descrizione:
 * Non ha mesh o funzionalità avanzate: serve solo per ricevere input del giocatore
 * e permettere il controllo da `PlayerController`.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:

	/**
	 * Costruttore di default
	 * Imposta il possesso automatico da parte del primo giocatore (Player0)
	 */
	ACameraPawn();
};