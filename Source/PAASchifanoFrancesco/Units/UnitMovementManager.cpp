#include "UnitMovementManager.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Input/MyPlayerController.h"
#include "Kismet/GameplayStatics.h"

/**
 * Descrizione:
 * Disattiva il tick dell'attore poiché non è necessario il controllo per frame.
 * Inizializza il puntatore alla unità attualmente in movimento a nullptr.
 */
AUnitMovementManager::AUnitMovementManager()
{
	PrimaryActorTick.bCanEverTick = false;  // Non serve il tick per frame
	CurrentMovingUnit = nullptr;            // Nessuna unità in movimento all'inizio
}

/**
 * Descrizione:
 * Avvia il movimento di una specifica unità lungo un percorso (`Path`) a una velocità indicata (`Speed`).
 * Questo metodo gestisce anche l'aggiornamento delle celle della griglia (tile di partenza e di arrivo),
 * collega il delegato per la fine del movimento e blocca temporaneamente l'input del giocatore.
 *
 * Parametri:
 * - Unit: puntatore all'unità da muovere
 * - Path: array di celle (ATile*) da attraversare
 * - Speed: velocità del movimento
 */
void AUnitMovementManager::MoveUnit(AUnitBase* Unit, const TArray<ATile*>& Path, float Speed)
{
	// Controlla parametri validi
	if (!Unit || Path.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: Parametri non validi!"));
		return;
	}

	// Impedisce avvio di un nuovo movimento se uno è già in corso
	if (IsMovementInProgress())
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveUnit: Movimento già in corso!"));
		return;
	}

	// Ottiene il GridManager per modificare le celle della griglia
	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveUnit: GridManager NON TROVATO!"));
		return;
	}

	// Verifiche extra di sicurezza
	ensure(Unit != nullptr);
	ensure(Path.Num() > 0);

	// Registra l’unità attualmente in movimento
	CurrentMovingUnit = Unit;

	// Imposta lo stato a “Moved”
	Unit->SetCurrentAction(EUnitAction::Moved);

	// Libera la tile di partenza
	ATile* StartTile = GridManager->FindTileAtLocation(Unit->GetActorLocation());
	if (StartTile)
	{
		StartTile->SetHasPawn(false);
	}

	// Occupa la tile di destinazione
	ATile* EndTile = Path.Last();
	if (EndTile)
	{
		EndTile->SetHasPawn(true);
	}

	// Collega il delegato OnMovementCompleted della MovementComponent
	if (Unit->MovementComponent)
	{
		// Rimuove prima eventuali bind per evitare duplicazioni
		Unit->MovementComponent->OnMovementCompleted.RemoveDynamic(this, &AUnitMovementManager::OnUnitMovementComplete);
		Unit->MovementComponent->OnMovementCompleted.AddDynamic(this, &AUnitMovementManager::OnUnitMovementComplete);
	}

	// Avvia il movimento fisico
	Unit->MovementComponent->StartMovement(Path, Speed);

	// Blocca l'input del player durante il movimento
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(Unit, 0))
	{
		if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
		{
			MyPC->SetMovementLocked(true);
		}
	}
}

/**
 * Descrizione:
 * Viene chiamato automaticamente quando una unità ha terminato il proprio movimento.
 * Rimuove il riferimento all’unità corrente e notifica a chi è in ascolto del delegato.
 */
void AUnitMovementManager::OnUnitMovementComplete()
{
	if (CurrentMovingUnit)
	{
		// Logga il completamento
		UE_LOG(LogTemp, Warning, TEXT("Movimento completato per %s"), *CurrentMovingUnit->GetName());

		// Notifica tramite delegato chi ha terminato il movimento
		OnMovementFinished.Broadcast(CurrentMovingUnit);

		// Resetta il riferimento
		CurrentMovingUnit = nullptr;
	}
	else
	{
		// In caso il puntatore sia già nullo (errore logico)
		UE_LOG(LogTemp, Error, TEXT("OnUnitMovementComplete chiamato, ma CurrentMovingUnit è NULL!"));
	}
}

/**
 * Descrizione:
 * Restituisce true se attualmente c'è un'unità in movimento.
 * Utile per evitare sovrapposizione di movimenti.
 */
bool AUnitMovementManager::IsMovementInProgress() const
{
	return CurrentMovingUnit != nullptr;
}

