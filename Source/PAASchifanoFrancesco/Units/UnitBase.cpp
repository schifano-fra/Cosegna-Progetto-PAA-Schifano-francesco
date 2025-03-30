// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitBase.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "MyMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PAASchifanoFrancesco/Input/MyPlayerController.h"
#include "Engine/DamageEvents.h"

AUnitBase::AUnitBase()
{
	PrimaryActorTick.bCanEverTick = false;
	MaxHealth = 20;
	CurrentHealth = MaxHealth;
	AttackRange = 1;
	MinDamage = 1;
	MaxDamage = 3;
	bIsRangedAttack = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionProfileName("PlayerPawn");
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MovementComponent = CreateDefaultSubobject<UMyMovementComponent>(TEXT("MovementComponent"));
}

void AUnitBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentAction = EUnitAction::Idle;
	// Nessun abilitazione diretta dell'input, gestito da MyPlayerController
	UE_LOG(LogTemp, Warning, TEXT("UnitBase: %s pronto e in attesa di selezione."), *GetName());
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

void AUnitBase::SetupMesh(UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Scale)
{
	if (Mesh)
	{
		MeshComponent->SetStaticMesh(Mesh);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh is NULL in SetupMesh!"));
	}

	if (Material)
	{
		UMaterialInstanceDynamic* PawnMaterial = UMaterialInstanceDynamic::Create(Material, this);
		if (PawnMaterial)
		{
			MeshComponent->SetMaterial(0, PawnMaterial);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create dynamic material!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Material is NULL in SetupMesh!"));
	}

	MeshComponent->SetWorldScale3D(Scale);
	UnitDisplayName = GetName();
}

int32 AUnitBase::GetMovementRange() const
{
	return MovementRange; // Default
}

bool AUnitBase::IsPlayerControlled() const
{
	return bIsPlayerControlled;
}

int32 AUnitBase::GetAttackRange() const
{
	return AttackRange;
}

bool AUnitBase::IsRangedAttack() const
{
	return bIsRangeAttack;
}

void AUnitBase::SetIsPlayerController(bool Condition)
{
	bIsPlayerControlled = Condition;
}

bool AUnitBase::IsDead() const
{
	return CurrentHealth <= 0;
}

float AUnitBase::GetHealthPercent() const
{
	return static_cast<float>(CurrentHealth) / static_cast<float>(MaxHealth);
}

void AUnitBase::AttackUnit(AUnitBase* Target)
{
	if (!Target) return;

	// Calcola danno casuale
	int32 Damage = FMath::RandRange(MinDamage, MaxDamage);
	UE_LOG(LogTemp, Warning, TEXT("⚔️ %s attacca %s con %d danni!"), *GetName(), *Target->GetName(), Damage);

	// Creiamo un evento danno vuoto perché TakeDamage lo richiede
	FPointDamageEvent DamageEvent;
	Target->TakeDamage(Damage, DamageEvent, GetController(), this);

	AMyGameMode* GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	UStatusGameWidget* StatusGame = nullptr;
	if (GameMode)
	{
		StatusGame = GameMode->GetStatusGameWidget();
	}
	
	// Se il bersaglio è morto, rimuoverlo dal gioco
	if (Target->IsDead())
	{
		Target->Die(Target);
	}
	else
	{
		StatusGame->UpdateUnitHealth(Target, Target->GetHealthPercent());
	}
}

bool AUnitBase::CanAct() const
{
	return CurrentAction == EUnitAction::Idle || CurrentAction == EUnitAction::Moved;
}

float AUnitBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = FMath::RoundToInt(DamageAmount); // Converte il danno a intero se necessario
	if (Damage > CurrentHealth)
	{
		CurrentHealth = 0;
	}
	else
	{
		CurrentHealth -= Damage;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%s ha subito %f danni! HP rimanenti: %d"), *GetName(), Damage, CurrentHealth);

	return DamageAmount;
}

void AUnitBase::Die(AUnitBase* Unit)
{
	if (IsPendingKillPending()) return;

	AMyGameMode* GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode) return;

	// 🔓 Libera tile
	if (AGridManager* GridManager = GameMode->GetGridManager())
	{
		ATile* Tile = GridManager->FindTileAtLocation(GetActorLocation());
		if (Tile)
		{
			Tile->SetHasPawn(false);
			UE_LOG(LogTemp, Warning, TEXT("🧱 Tile %s liberata"), *Tile->GetName());
		}
	}

	// 🧼 Rimuovi dal widget status
	if (UStatusGameWidget* Status = GameMode->GetStatusGameWidget())
	{
		Status->RemoveUnitStatus(Unit);
	}

	// 🧹 Rimuovi dalla lista
	if (IsPlayerControlled())
	{
		GameMode->PlayerUnits.Remove(Unit);
	}
	else
	{
		GameMode->AIUnits.Remove(Unit);
	}
	// 💥 Distruggi
	Destroy();
	UE_LOG(LogTemp, Warning, TEXT("🧮 PlayerUnits.Num(): %d"), GameMode->PlayerUnits.Num());
	UE_LOG(LogTemp, Warning, TEXT("🧮 AIUnits.Num(): %d"), GameMode->AIUnits.Num());
	
	// 🏁 Controllo fine partita
	// In Die()
	if (GameMode->PlayerUnits.Num() == 0 || GameMode->AIUnits.Num() == 0)
	{
		FString Winner = GameMode->PlayerUnits.Num() == 0 ? TEXT("AI") : TEXT("Player");
		GameMode->SetGamePhase(EGamePhase::EGameOver);
		GameMode->HandleGameOver(Winner);
	}
	
}