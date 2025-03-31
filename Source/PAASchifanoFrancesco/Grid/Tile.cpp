// Creato da: Schifano Francesco 5469994

#include "Tile.h"

/**
 * Costruttore della classe ATile.
 * Inizializza la mesh, i materiali e i flag logici.
 */
ATile::ATile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Crea la mesh e la imposta come RootComponent
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	RootComponent = TileMesh;

	// Carica la mesh del piano
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (MeshRef.Succeeded())
	{
		TileMesh->SetStaticMesh(MeshRef.Object);
	}

	// Carica il materiale dell’albero
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TreeMaterialRef(TEXT("/Game/Material/Tree.Tree"));
	if(TreeMaterialRef.Succeeded())
	{
		TreeMaterial = UMaterialInstanceDynamic::Create(TreeMaterialRef.Object, this);
	}

	// Carica il materiale della montagna
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MountainMaterialRef(TEXT("/Game/Material/Mountain.Mountain"));
	if(MountainMaterialRef.Succeeded())
	{
		MountainMaterial = UMaterialInstanceDynamic::Create(MountainMaterialRef.Object, this);
	}

	// Carica il materiale della tile normale
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterialRef(TEXT("/Game/Material/Tile.Tile"));
	if (NormalMaterialRef.Succeeded())
	{
		NormalMaterial = UMaterialInstanceDynamic::Create(NormalMaterialRef.Object, this);
	}

	// Inizializza i flag logici
	bIsObstacle = false;
	bHasPawn = false;
}

/**
 * Metodo chiamato all’avvio della partita.
 * Imposta la collisione della tile e assegna il materiale corretto.
 */
void ATile::BeginPlay()
{
	Super::BeginPlay();

	// Impostazioni di collisione per click con il mouse
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	TileMesh->SetGenerateOverlapEvents(false);

	// Abilita l’input per essere cliccabile
	EnableInput(GetWorld()->GetFirstPlayerController());

	// Applica lo stato iniziale di ostacolo (se impostato)
	SetAsObstacle(bIsObstacle);
}

/**
 * Imposta la tile come ostacolo o no e aggiorna il materiale e il tipo di collisione.
 */
void ATile::SetAsObstacle(bool NewbIsObstacle)
{
	bIsObstacle = NewbIsObstacle;

	if (bIsObstacle)
	{
		// Se è un ostacolo, scegle se usare albero o montagna
		const bool bUseTree = FMath::RandBool();

		if (bUseTree && TreeMaterial)
		{
			TileMesh->SetMaterial(0, TreeMaterial);
		}
		else if (MountainMaterial)
		{
			TileMesh->SetMaterial(0, MountainMaterial);
		}
	}
	else if (NormalMaterial)
	{
		TileMesh->SetMaterial(0, NormalMaterial);
	}

	// Imposta il tipo di oggetto collisione per ostacoli
	TileMesh->SetCollisionObjectType(bIsObstacle ? ECC_GameTraceChannel1 : ECC_WorldStatic);
}

/**
 * Evidenzia la tile con un colore specificato.
 * Utilizzato per mostrare movimenti, attacchi, selezione, ecc.
 */
void ATile::SetHighlight(bool bHighlight, FLinearColor HighlightColor)
{
	if (NormalMaterial)
	{
		// Cambia il colore del parametro "Color" nel materiale dinamico
		NormalMaterial->SetVectorParameterValue("Color", HighlightColor);

		// Reapplica il materiale modificato
		TileMesh->SetMaterial(0, NormalMaterial);
	}
}

/**
 * Ritorna la posizione 3D dove far spawnare una pedina su questa tile.
 * Viene alzato di 50 unità in Z rispetto al piano della mesh.
 */
FVector ATile::GetPawnSpawnLocation() const
{
	return GetActorLocation() + FVector(0, 0, 50);
}

/**
 * Imposta l’identificativo testuale della tile, es. “A3”.
 */
void ATile::SetTileIdentifier(const FString& NewIdentifier)
{
	TileIdentifier = NewIdentifier;
}

/**
 * Tick dell’attore. Disabilitato.
 */
void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}