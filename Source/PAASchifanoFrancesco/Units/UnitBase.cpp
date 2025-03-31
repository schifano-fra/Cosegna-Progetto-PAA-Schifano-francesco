//Creato da: Schifano Francesco 5469994

#include "UnitBase.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "MyMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PAASchifanoFrancesco/Input/MyPlayerController.h"
#include "Engine/DamageEvents.h"

// Costruttore dell'unità base, inizializza tutte le variabili di default
AUnitBase::AUnitBase()
{
	// Disabilita il tick per ottimizzare le performance (non serve aggiornamento continuo)
	PrimaryActorTick.bCanEverTick = false;

	// Impostazioni di default per gli attributi dell’unità
	MaxHealth = 20;				 // Vita massima
	CurrentHealth = MaxHealth;	 // All’inizio, la vita attuale è piena
	AttackRange = 1;			 // Range di attacco iniziale (1 per attacco corpo a corpo)
	MinDamage = 1;				 // Danno minimo base
	MaxDamage = 3;				 // Danno massimo base

	// Crea il componente grafico della mesh e lo imposta come radice dell’attore
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	// Configura le collisioni della mesh
	MeshComponent->SetCollisionProfileName("PlayerPawn");								 // Profilo collisione standard
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);	 // Abilita sia query che fisica
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);// Blocca i raggi di visibilità (necessario per click)

	// Crea il componente per la gestione del movimento personalizzato
	MovementComponent = CreateDefaultSubobject<UMyMovementComponent>(TEXT("MovementComponent"));
}

// Metodo chiamato automaticamente da Unreal all'inizio del gioco
void AUnitBase::BeginPlay()
{
	Super::BeginPlay(); // Chiama BeginPlay() della superclasse (APawn)

	CurrentAction = EUnitAction::Idle; // A inizio partita, l’unità è in stato “Idle”

	// Messaggio di debug per indicare che l’unità è pronta
	UE_LOG(LogTemp, Warning, TEXT("UnitBase: %s pronto e in attesa di selezione."), *GetName());
}

// Metodo utilizzato per assegnare dinamicamente la mesh, il materiale e la scala all'unità
void AUnitBase::SetupMesh(UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Scale)
{
	if (Mesh)
	{
		// Imposta la mesh 3D dell’unità
		MeshComponent->SetStaticMesh(Mesh);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh is NULL in SetupMesh!")); // Errore se la mesh è nulla
	}

	if (Material)
	{
		// Crea una istanza dinamica del materiale per permettere modifiche runtime
		UMaterialInstanceDynamic* PawnMaterial = UMaterialInstanceDynamic::Create(Material, this);
		if (PawnMaterial)
		{
			MeshComponent->SetMaterial(0, PawnMaterial); // Applica il materiale alla mesh
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create dynamic material!")); // Errore nella creazione del materiale
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Material is NULL in SetupMesh!")); // Errore se il materiale è nullo
	}

	// Imposta la scala 3D della mesh
	MeshComponent->SetWorldScale3D(Scale);

	// Salva il nome visualizzato dell'unità
	UnitDisplayName = GetName();
}

// Ritorna il range di movimento dell’unità
int32 AUnitBase::GetMovementRange() const
{
	return MovementRange;
}

// Verifica se l'unità è controllata dal player
bool AUnitBase::IsPlayerControlled() const
{
	return bIsPlayerControlled;
}

// Ritorna la distanza di attacco dell’unità
int32 AUnitBase::GetAttackRange() const
{
	return AttackRange;
}

// Verifica se l'unità è uno Sniper (attacco a distanza)
bool AUnitBase::IsRangedAttack() const
{
	return bIsRangeAttack;
}

// Imposta se l'unità è controllata dal player
void AUnitBase::SetIsPlayerController(bool Condition)
{
	bIsPlayerControlled = Condition;
}


// Verifica se l’unità è morta (vita <= 0)
bool AUnitBase::IsDead() const
{
	return CurrentHealth <= 0;
}

void AUnitBase::SelectUnit()
{
	OnUnitSelected.Broadcast(this);
	UE_LOG(LogTemp, Warning, TEXT("UnitBase: %s selezionato."), *GetName());
}

void AUnitBase::DeselectUnit()
{
	OnUnitDeselected.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("UnitBase: %s deselezionato."), *GetName());
}

// Calcola la percentuale di vita rimasta (per la barra della salute)
float AUnitBase::GetHealthPercent() const
{
	return static_cast<float>(CurrentHealth) / static_cast<float>(MaxHealth);
}

/**
 * Descrizione:
 * Permette a questa unità di attaccare un'altra unità specificata come bersaglio.
 * Calcola un danno casuale e lo applica tramite il metodo `TakeDamage()` del bersaglio.
 * Se il bersaglio muore, viene chiamato `Die()`; altrimenti aggiorna la UI della barra vita.
 */
void AUnitBase::AttackUnit(AUnitBase* Target)
{
	if (!Target) return; // Controllo di sicurezza sul puntatore

	// Calcola il danno da infliggere scegliendo un valore casuale tra MinDamage e MaxDamage
	int32 Damage = FMath::RandRange(MinDamage, MaxDamage);
	UE_LOG(LogTemp, Warning, TEXT("%s attacca %s con %d danni!"), *GetName(), *Target->GetName(), Damage);

	// Crea un evento danno necessario per il metodo TakeDamage
	FPointDamageEvent DamageEvent;
	Target->TakeDamage(Damage, DamageEvent, GetController(), this); // Infligge il danno al bersaglio

	// Ottiene riferimento al GameMode e al widget per aggiornare la UI della salute
	AMyGameMode* GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	UStatusGameWidget* StatusGame = nullptr;
	if (GameMode)
	{
		StatusGame = GameMode->GetStatusGameWidget();
	}
	
	// Se il bersaglio è morto dopo l’attacco, viene rimosso dal gioco
	if (Target->IsDead())
	{
		Target->Die(Target);
	}
	else
	{
		// Altrimenti, aggiorna la barra vita nel widget di status
		StatusGame->UpdateUnitHealth(Target, Target->GetHealthPercent());
	}
}

/**
 * Descrizione:
 * Restituisce true se l’unità può ancora agire durante il turno attuale.
 * Le unità possono agire se sono in stato `Idle` (non hanno ancora mosso o attaccato)
 * oppure se hanno solo mosso (`Moved`) e possono ancora attaccare.
 */
bool AUnitBase::CanAct() const
{
	return CurrentAction == EUnitAction::Idle || CurrentAction == EUnitAction::Moved;
}


/**
 * Descrizione:
 * Gestisce la ricezione del danno da parte dell’unità.
 * Aggiorna `CurrentHealth` in base alla quantità di danno ricevuto.
 * 
 * Parametri:
 * - DamageAmount: Quantità di danno inflitta.
 * - DamageEvent, EventInstigator, DamageCauser: richiesti da Unreal Engine, ma inutilizzati.
 * 
 * Ritorno:
 * - Restituisce la quantità di danno ricevuto.
 */
float AUnitBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Arrotonda il danno a intero
	float Damage = FMath::RoundToInt(DamageAmount);

	// Se il danno supera la vita attuale, setta la vita a 0
	if (Damage > CurrentHealth)
	{
		CurrentHealth = 0;
	}
	else
	{
		// Altrimenti scala la vita
		CurrentHealth -= Damage;
	}
	
	// Messaggio di debug con stato aggiornato
	UE_LOG(LogTemp, Warning, TEXT("%s ha subito %f danni! HP rimanenti: %d"), *GetName(), Damage, CurrentHealth);

	return DamageAmount;
}

/**
 * Descrizione:
 * Viene chiamato quando l'unità muore (CurrentHealth <= 0).
 * Esegue tutte le operazioni di rimozione: libera la tile, aggiorna la UI,
 * rimuove l'unità dalla lista e controlla se la partita è terminata.
 * 
 * Parametri:
 * - Unit: unità da rimuovere (in genere this).
 */
void AUnitBase::Die(AUnitBase* Unit)
{
	// Controlla se l’unità è già in fase di distruzione
	if (IsPendingKillPending()) return;

	// Recupera il GameMode
	AMyGameMode* GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode) return;

	// Libera la tile occupata, settando HasPawn = false
	if (AGridManager* GridManager = GameMode->GetGridManager())
	{
		ATile* Tile = GridManager->FindTileAtLocation(GetActorLocation());
		if (Tile)
		{
			Tile->SetHasPawn(false);
			UE_LOG(LogTemp, Warning, TEXT("Tile %s liberata"), *Tile->GetName());
		}
	}

	// Rimuove la barra della vita dal widget dello status
	if (UStatusGameWidget* Status = GameMode->GetStatusGameWidget())
	{
		Status->RemoveUnitStatus(Unit);
	}

	// Rimuove l’unità dalla lista corretta (player o AI)
	if (IsPlayerControlled())
	{
		GameMode->PlayerUnits.Remove(Unit);
	}
	else
	{
		GameMode->AIUnits.Remove(Unit);
	}

	// Distrugge fisicamente l’unità nella scena
	Destroy();

	// Logga quante unità restano in gioco
	UE_LOG(LogTemp, Warning, TEXT("PlayerUnits.Num(): %d"), GameMode->PlayerUnits.Num());
	UE_LOG(LogTemp, Warning, TEXT("AIUnits.Num(): %d"), GameMode->AIUnits.Num());

	// Controlla se il gioco deve terminare (una delle due liste è vuota)
	if (GameMode->PlayerUnits.Num() == 0 || GameMode->AIUnits.Num() == 0)
	{
		// Determina il vincitore
		FString Winner = GameMode->PlayerUnits.Num() == 0 ? TEXT("AI") : TEXT("Player");

		// Imposta la fase di fine gioco
		GameMode->SetGamePhase(EGamePhase::EGameOver);

		// Mostra il widget di fine partita
		GameMode->HandleGameOver(Winner);
	}
}