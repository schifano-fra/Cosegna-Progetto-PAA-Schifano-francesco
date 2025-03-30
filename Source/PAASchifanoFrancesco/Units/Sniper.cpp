// Fill out your copyright notice in the Description page of Project Settings.


#include "Sniper.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Core/TurnManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ASniper::ASniper()
{
	CurrentHealth = 20;
	MovementRange = 3;
	MinDamage = 4;
	MaxDamage = 8;
	AttackRange = 10;
	bIsRangeAttack = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SniperMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	UMaterialInterface* SniperMat = nullptr;

	// Ottieni la partita corrente
	AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if(GM && GM->TurnManager->GetCurrentPlayer() == EPlayer::Player1)
	{
		// Materiale per il giocatore
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMaterial(TEXT("/Game/Material/PlayerSniper.PlayerSniper"));
		if (PlayerMaterial.Succeeded())
		{
			SniperMat = PlayerMaterial.Object;
		}
	}
	else
	{
		// Materiale per l'AI
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIMaterial(TEXT("/Game/Material/AISniper.AISniper"));
		if (AIMaterial.Succeeded())
		{
			SniperMat = AIMaterial.Object;
		}
	}
	
	Mesh = SniperMesh.Object;
	Material = SniperMat;
}

void ASniper::BeginPlay()
{
	Super::BeginPlay();
	if (Mesh && Material)
	{
		SetupMesh(Mesh, Material, FVector(1.2f));
	}
}