// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PAASchifanoFrancesco/Grid/Tile.h"
#include "MyMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovementCompleted);
UCLASS()
class PAASCHIFANOFRANCESCO_API UMyMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyMovementComponent();

	void StartMovement(const TArray<ATile*>& Path, float Speed);
	void FinishMovement();

	UPROPERTY(BlueprintAssignable)
	FOnMovementCompleted OnMovementCompleted;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TArray<ATile*> MovementPath;
	float MovementSpeed;
	int32 CurrentTargetIndex;

	bool bIsMoving;
};
