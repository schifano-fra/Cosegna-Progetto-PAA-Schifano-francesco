// Include standard generato da Unreal
#include "Sniper.h"

// Include per accedere al GameMode e TurnManager
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Core/TurnManager.h"

// Include per ottenere il GameMode a runtime
#include "Kismet/GameplayStatics.h"

// Per usare ConstructorHelpers e caricare asset statici
#include "UObject/ConstructorHelpers.h"

/**
 * Costruttore della classe ASniper.
 * Inizializza le statistiche dell'unità e carica dinamicamente la mesh e il materiale,
 * differenziando tra giocatore e IA in base al turno attivo.
 */
ASniper::ASniper()
{
	// Impostazioni specifiche per l’unità Sniper
	CurrentHealth = 20;         // Vita base più bassa (unità fragile)
	MovementRange = 3;          // Movimento limitato
	MinDamage = 4;              // Danno minimo alto
	MaxDamage = 8;              // Danno massimo alto
	AttackRange = 10;           // Gittata molto ampia (unità da distanza)
	bIsRangeAttack = true;      // Attacco a distanza attivo

	// Caricamento della mesh (stessa per tutte le unità, un semplice Plane)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SniperMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));

	// Puntatore temporaneo per il materiale dell’unità
	UMaterialInterface* SniperMat = nullptr;

	// Recupera il GameMode corrente per determinare il tipo di giocatore attivo
	AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));

	// Se è il turno del giocatore (Player1), carica il materiale del player
	if(GM && GM->TurnManager->GetCurrentPlayer() == EPlayer::Player1)
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMaterial(TEXT("/Game/Material/PlayerSniper.PlayerSniper"));
		if (PlayerMaterial.Succeeded())
		{
			SniperMat = PlayerMaterial.Object;
		}
	}
	else
	{
		// Altrimenti carica il materiale dell’IA
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIMaterial(TEXT("/Game/Material/AISniper.AISniper"));
		if (AIMaterial.Succeeded())
		{
			SniperMat = AIMaterial.Object;
		}
	}

	// Assegna mesh e materiale alle variabili membro
	Mesh = SniperMesh.Object;
	Material = SniperMat;
}

/**
 * Metodo BeginPlay
 * Viene eseguito quando l’attore viene inizializzato nella scena.
 * Applica la mesh e il materiale all’attore tramite SetupMesh (eredito da AUnitBase).
 */
void ASniper::BeginPlay()
{
	Super::BeginPlay(); // Chiama la versione base del metodo

	// Se sia Mesh che Materiale sono validi, li applica all’unità
	if (Mesh && Material)
	{
		SetupMesh(Mesh, Material, FVector(1.2f)); // Applica scala più piccola del Brawler
	}
}