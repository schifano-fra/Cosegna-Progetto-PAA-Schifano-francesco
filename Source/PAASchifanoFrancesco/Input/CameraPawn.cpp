//Creato da: Schifano Francesco 5469994
#include "CameraPawn.h"

/**
 * Imposta il possesso automatico da parte del giocatore principale (Player0),
 * permettendo alla telecamera di ricevere input senza doverla assegnare manualmente nel GameMode.
 */
ACameraPawn::ACameraPawn()
{
	// Imposta il possesso automatico al primo giocatore (index 0), ovvero il giocatore umano
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}
