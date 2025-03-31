//Creato da: Schifano Francesco 5469994

#include "MyMovementComponent.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "UnitBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UMyMovementComponent::UMyMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true; // Abilita il Tick per aggiornare il movimento
	bIsMoving = false;                        // Inizialmente non si sta muovendo
	MovementSpeed = 300.f;                    // Velocità di default
	CurrentTargetIndex = 0;                   // Nessun indice attivo all’inizio
}

void UMyMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * Metodo che avvia un movimento lungo un certo percorso.
 * Imposta le variabili interne e abilita lo stato di movimento.
 */
void UMyMovementComponent::StartMovement(const TArray<ATile*>& Path, float Speed)
{
	if (Path.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("StartMovement: Path vuoto!"));
		return;
	}

	MovementPath = Path;           // Salva il percorso
	MovementSpeed = Speed;         // Imposta la velocità
	CurrentTargetIndex = 0;        // Parte dal primo punto
	bIsMoving = true;              // Movimento in corso
	
	UE_LOG(LogTemp, Warning, TEXT("Inizio movimento su %d tiles con velocità %.1f"), Path.Num(), Speed);

	// Notifica lo stato di movimento alla UnitBase proprietaria
	if (AUnitBase* Unit = Cast<AUnitBase>(GetOwner()))
	{
		Unit->bIsMoving = true;
	}
}

/**
 * Metodo chiamato ogni frame se il componente è attivo.
 * Gestisce il movimento fluido tra una tile e l’altra del percorso.
 */
void UMyMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Se il movimento non è attivo o il path è finito, esce
	if (!bIsMoving || MovementPath.Num() == 0 || CurrentTargetIndex >= MovementPath.Num())
		return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Posizione attuale e posizione target della prossima tile
	FVector CurrentLocation = Owner->GetActorLocation();
	FVector TargetLocation = MovementPath[CurrentTargetIndex]->GetPawnSpawnLocation();

	// Calcola nuova posizione con interpolazione lineare costante
	FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MovementSpeed);
	Owner->SetActorLocation(NewLocation); // Aggiorna la posizione attuale

	// Se è vicino abbastanza al target, passa alla prossima tile
	if (FVector::Dist(NewLocation, TargetLocation) < 5.f)
	{
		CurrentTargetIndex++;

		// Se ha raggiunto l’ultima tile → fine movimento
		if (CurrentTargetIndex >= MovementPath.Num())
		{
			bIsMoving = false;

			if (AUnitBase* Unit = Cast<AUnitBase>(GetOwner()))
			{
				Unit->bIsMoving = false;
			}

			// Aggiorna lo stato della tile finale nel GridManager
			if (AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this)))
			{
				if (AGridManager* GridManager = GM->GetGridManager())
				{
					if (MovementPath.Num() > 0)
					{
						ATile* LastTile = MovementPath.Last();
						GridManager->FinalizeUnitMovement(Cast<AUnitBase>(Owner), LastTile);
					}
				}
			}

			// Notifica a chi ascolta che il movimento è terminato
			UE_LOG(LogTemp, Warning, TEXT("Broadcast: movimento completato in UMyMovementComponent"));
			OnMovementCompleted.Broadcast();
		}
	}
}