#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitBase.h"
#include "UnitMovementManager.generated.h"

// 🔥 Delegate per notificare quando un'unità ha finito di muoversi
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitMovementFinished, AUnitBase*, MovedUnit);

UCLASS()
class PAASCHIFANOFRANCESCO_API AUnitMovementManager : public AActor
{
	GENERATED_BODY()

public:
	AUnitMovementManager();

	void StartMovement(const TArray<ATile*>& Path, float Speed);
	
	// Delegate per notificare quando un'unità ha finito di muoversi
	UPROPERTY()
	FOnUnitMovementFinished OnMovementFinished;

	void MoveUnit(AUnitBase* Unit, const TArray<ATile*>& Path, float Speed);

private:
	UFUNCTION()
	void OnUnitMovementComplete();


	bool IsMovementInProgress() const;
	AUnitBase* CurrentMovingUnit;
};
