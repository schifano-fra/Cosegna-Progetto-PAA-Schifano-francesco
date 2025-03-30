#include "Brawler.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Core/TurnManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

class AMyGameMode;

ABrawler::ABrawler()
{
	CurrentHealth = 40;
	MovementRange = 6;
	MinDamage = 1;
	MaxDamage = 6;
	AttackRange = 1;
	bIsRangeAttack = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BrawlerMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	
	UMaterialInterface* BrawlerMat = nullptr;

	// Ottieni la partita corrente
	AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if(GM && GM->TurnManager->GetCurrentPlayer() == EPlayer::Player1)
	{
		// Materiale per il giocatore
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMaterial(TEXT("/Game/Material/PlayerBrawler.PlayerBrawler"));
		if (PlayerMaterial.Succeeded())
		{
			BrawlerMat = PlayerMaterial.Object;
		}
	}
	else
	{
		// Materiale per l'AI
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIMaterial(TEXT("/Game/Material/AIBrawler.AIBrawler"));
		if (AIMaterial.Succeeded())
		{
			BrawlerMat = AIMaterial.Object;
		}
	}

	Mesh = BrawlerMesh.Object;
	Material = BrawlerMat;
}

void ABrawler::BeginPlay()
{
	Super::BeginPlay();
    
	if (Mesh && Material)
	{
		SetupMesh(Mesh, Material, FVector(1.6f));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Brawler Mesh or Material is NULL!"));
	}
}