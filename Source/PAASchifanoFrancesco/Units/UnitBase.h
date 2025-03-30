// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyMovementComponent.h"
#include "UnitBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitSelected, AUnitBase*, SelectedUnit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnitDeselected);

UENUM()
enum class EUnitAction : uint8
{
	Idle,
	Moved,
	Attacked,
	MoveAttack
};

UCLASS()
class PAASCHIFANOFRANCESCO_API AUnitBase : public APawn
{
	GENERATED_BODY()

public:
	AUnitBase();

	UFUNCTION()
	int32 GetMovementRange() const;
	bool IsPlayerControlled() const;
	void SetIsPlayerController(bool Condition);

	UPROPERTY()
	FOnUnitSelected OnUnitSelected;

	UPROPERTY()
	FOnUnitDeselected OnUnitDeselected;

	void SelectUnit();
	void DeselectUnit();

	int32 GetAttackRange() const;
	bool IsRangedAttack() const;

	UPROPERTY()
	bool bHasMovedThisTurn = false;

	UPROPERTY()
	UMyMovementComponent* MovementComponent;

	UPROPERTY()
	bool bIsMoving = false;
	
	UPROPERTY(EditAnywhere,  Category = "Stats")
	int32 MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Stats")
	int32 CurrentHealth;

	UPROPERTY(EditAnywhere,  Category = "Stats")
	int32 AttackRange;

	UPROPERTY(EditAnywhere,  Category = "Stats")
	int32 MinDamage;

	UPROPERTY(EditAnywhere,  Category = "Stats")
	int32 MaxDamage;

	UPROPERTY(EditAnywhere,  Category = "Stats")
	bool bIsRangedAttack; // true per Sniper, false per Brawler
	
	UPROPERTY(EditAnywhere, Category = "Unit Info")
	FString UnitDisplayName;
	
	// Funzioni
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	bool IsDead() const;

	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;

	float GetHealth() const { return CurrentHealth; }

	void AttackUnit(AUnitBase* Target);
	bool CanAct() const;
	
	EUnitAction GetCurrentAction() const { return CurrentAction; }
	void SetCurrentAction(EUnitAction NewAction) { CurrentAction = NewAction; }
	void ResetAction() { CurrentAction = EUnitAction::Idle; }

	UFUNCTION()
	void Die(AUnitBase* Target);
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Unit Stats")
	int32 MovementRange;

	UPROPERTY()
	bool bIsRangeAttack = false;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	virtual void SetupMesh(UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Scale);

	UPROPERTY(EditAnywhere, Category = "Unit")
	int32 TeamID;

	UPROPERTY()
	bool bIsPlayerControlled = false;

	EUnitAction CurrentAction = EUnitAction::Idle;
};