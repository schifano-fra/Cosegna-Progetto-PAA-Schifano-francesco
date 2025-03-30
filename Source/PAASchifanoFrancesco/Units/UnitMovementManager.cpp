#include "UnitMovementManager.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Input/MyPlayerController.h"
#include "Kismet/GameplayStatics.h"

AUnitMovementManager::AUnitMovementManager()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentMovingUnit = nullptr;
}

void AUnitMovementManager::MoveUnit(AUnitBase* Unit, const TArray<ATile*>& Path, float Speed)
{
	if (!Unit || Path.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("❌ MoveUnit: Parametri non validi!"));
		return;
	}

	if (IsMovementInProgress())
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ MoveUnit: Movimento già in corso!"));
		return;
	}

	// 🔥 Controlla se GridManager è stato trovato
	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    
	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ MoveUnit: GridManager NON TROVATO!"));
		return;
	}

	// 🔥 Sicurezza extra: evita accessi a nullptr
	ensure(Unit != nullptr);
	ensure(Path.Num() > 0);

	CurrentMovingUnit = Unit;
	Unit->SetCurrentAction(EUnitAction::Moved); 

	ATile* StartTile = GridManager->FindTileAtLocation(Unit->GetActorLocation());
	if (StartTile)
	{
		StartTile->SetHasPawn(false);
	}

	ATile* EndTile = Path.Last();
	if (EndTile)
	{
		EndTile->SetHasPawn(true);
	}

	if (Unit->MovementComponent)
	{
		// Evita bind multipli (bug UE) ❗️
		Unit->MovementComponent->OnMovementCompleted.RemoveDynamic(this, &AUnitMovementManager::OnUnitMovementComplete);
		Unit->MovementComponent->OnMovementCompleted.AddDynamic(this, &AUnitMovementManager::OnUnitMovementComplete);
	}

	Unit->MovementComponent->StartMovement(Path, Speed);
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(Unit, 0))
	{
		if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
		{
			MyPC->SetMovementLocked(true);
		}
	}
}

void AUnitMovementManager::OnUnitMovementComplete()
{
	if (CurrentMovingUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("✅ Movimento completato per %s"), *CurrentMovingUnit->GetName());

		OnMovementFinished.Broadcast(CurrentMovingUnit);
		CurrentMovingUnit = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ OnUnitMovementComplete chiamato, ma CurrentMovingUnit è NULL!"));
	}
}

bool AUnitMovementManager::IsMovementInProgress() const
{
	return CurrentMovingUnit != nullptr;
}
