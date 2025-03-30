#include "MyMovementComponent.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "UnitBase.h"
#include "Kismet/GameplayStatics.h"

#include "GameFramework/Actor.h"

UMyMovementComponent::UMyMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsMoving = false;
	MovementSpeed = 300.f; // Default
	CurrentTargetIndex = 0;
}

void UMyMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMyMovementComponent::StartMovement(const TArray<ATile*>& Path, float Speed)
{
	if (Path.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ StartMovement: Path vuoto!"));
		return;
	}

	MovementPath = Path;
	MovementSpeed = Speed;
	CurrentTargetIndex = 0;
	bIsMoving = true;

	UE_LOG(LogTemp, Warning, TEXT("🚀 Inizio movimento su %d tiles con velocità %.1f"), Path.Num(), Speed);
	
	if (AUnitBase* Unit = Cast<AUnitBase>(GetOwner()))
	{
		Unit->bIsMoving = true;
	}
}

void UMyMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsMoving || MovementPath.Num() == 0 || CurrentTargetIndex >= MovementPath.Num())
		return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector CurrentLocation = Owner->GetActorLocation();
	FVector TargetLocation = MovementPath[CurrentTargetIndex]->GetPawnSpawnLocation();

	FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MovementSpeed);
	Owner->SetActorLocation(NewLocation);

	if (FVector::Dist(NewLocation, TargetLocation) < 5.f)
	{
		CurrentTargetIndex++;

		// ✅ Abbiamo raggiunto l’ultima tile!
		if (CurrentTargetIndex >= MovementPath.Num())
		{
			bIsMoving = false;

			if (AUnitBase* Unit = Cast<AUnitBase>(GetOwner()))
			{
				Unit->bIsMoving = false;
			}

			// Finalizza lo stato della tile
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
			UE_LOG(LogTemp, Warning, TEXT("📣 Broadcast: movimento completato in UMyMovementComponent"));
			OnMovementCompleted.Broadcast();
		}
	}
}