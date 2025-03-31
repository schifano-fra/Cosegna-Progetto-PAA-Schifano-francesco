#include "Brawler.h"  // Include del file header della classe ABrawler
#include "PAASchifanoFrancesco/Core/MyGameMode.h"  // Per ottenere il GameMode
#include "PAASchifanoFrancesco/Core/TurnManager.h" // Per accedere al turno corrente
#include "Kismet/GameplayStatics.h"                // Per ottenere il GameMode in modo sicuro
#include "UObject/ConstructorHelpers.h"            // Per il caricamento statico di asset

class AMyGameMode;  // Forward declaration opzionale (già incluso comunque)

/**
 * Costruttore della classe ABrawler.
 * Inizializza le caratteristiche base del personaggio e imposta Mesh e Materiale in base al tipo di giocatore.
 */
ABrawler::ABrawler()
{
	// Impostazioni di base per il personaggio Brawler
	CurrentHealth = 40;         // Vita iniziale del Brawler
	MovementRange = 6;          // Distanza di movimento
	MinDamage = 1;              // Danno minimo
	MaxDamage = 6;              // Danno massimo
	AttackRange = 1;            // Gittata (solo adiacente)
	bIsRangeAttack = false;     // Il Brawler non attacca a distanza

	// Carica una mesh di base (Plane) dal motore Unreal
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BrawlerMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));

	// Puntatore temporaneo al materiale da assegnare
	UMaterialInterface* BrawlerMat = nullptr;

	// Ottieni il GameMode corrente per sapere chi è il giocatore attivo
	AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if(GM && GM->TurnManager->GetCurrentPlayer() == EPlayer::Player1)
	{
		// Se è il turno del giocatore, carica il materiale del player
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMaterial(TEXT("/Game/Material/PlayerBrawler.PlayerBrawler"));
		if (PlayerMaterial.Succeeded())
		{
			BrawlerMat = PlayerMaterial.Object;
		}
	}
	else
	{
		// Altrimenti carica il materiale dell'IA
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIMaterial(TEXT("/Game/Material/AIBrawler.AIBrawler"));
		if (AIMaterial.Succeeded())
		{
			BrawlerMat = AIMaterial.Object;
		}
	}

	// Assegna la mesh e il materiale alla variabile membro
	Mesh = BrawlerMesh.Object;
	Material = BrawlerMat;
}

/**
 * Metodo BeginPlay
 * Chiamato automaticamente da Unreal quando l’attore entra in scena.
 */
void ABrawler::BeginPlay()
{
	Super::BeginPlay(); // Chiama BeginPlay della classe base

	if (Mesh && Material)
	{
		// Imposta la mesh e il materiale tramite metodo protetto della classe base
		SetupMesh(Mesh, Material, FVector(1.6f)); // Scala leggermente ingrandita
	}
	else
	{
		// In caso di errore, logga nel terminale
		UE_LOG(LogTemp, Error, TEXT("Brawler Mesh or Material is NULL!"));
	}
}