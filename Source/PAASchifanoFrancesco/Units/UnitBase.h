// Creato da: Schifano Francesco 5469994

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyMovementComponent.h"
#include "UnitBase.generated.h"

// Delegate utilizzato per notificare che un'unità è stata selezionata
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitSelected, AUnitBase*, SelectedUnit);

// Delegate utilizzato per notificare che un'unità è stata deselezionata
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnitDeselected);

// Enum che rappresenta lo stato corrente di un'unità durante il turno
UENUM()
enum class EUnitAction : uint8
{
	Idle,        // L'unità non ha ancora agito
	Moved,       // L'unità ha già effettuato un movimento
	Attacked,    // L'unità ha già attaccato
	MoveAttack   // L'unità ha già mosso e poi attaccato nello stesso turno
};

// Classe base per tutte le unità del gioco (sia player che AI)
UCLASS()
class PAASCHIFANOFRANCESCO_API AUnitBase : public APawn
{
	GENERATED_BODY()

public:
	// Costruttore della classe
	AUnitBase();

	// Restituisce il range di movimento dell'unità
	UFUNCTION()
	int32 GetMovementRange() const;

	// Verifica se l'unità è controllata dal giocatore
	bool IsPlayerControlled() const;

	// Imposta se l'unità è controllata dal giocatore
	void SetIsPlayerController(bool Condition);

	// Delegate per notificare la selezione dell'unità
	UPROPERTY()
	FOnUnitSelected OnUnitSelected;

	// Delegate per notificare la deselezione dell'unità
	UPROPERTY()
	FOnUnitDeselected OnUnitDeselected;

	// Seleziona l'unità (triggera il delegate)
	void SelectUnit();

	// Deseleziona l'unità (triggera il delegate)
	void DeselectUnit();

	// Restituisce il range di attacco
	int32 GetAttackRange() const;

	// Indica se l'unità è un'unità a distanza (Sniper)
	bool IsRangedAttack() const;

	// Indica se l'unità ha già effettuato un movimento durante il turno
	UPROPERTY()
	bool bHasMovedThisTurn = false;

	// Componente che gestisce il movimento personalizzato dell’unità
	UPROPERTY()
	UMyMovementComponent* MovementComponent;

	// Indica se l'unità è attualmente in movimento
	UPROPERTY()
	bool bIsMoving = false;

	// Dati relativi agli attributi di combattimento
	UPROPERTY(EditAnywhere, Category = "Stats")
	int32 MaxHealth;          // Vita massima

	UPROPERTY(EditAnywhere, Category = "Stats")
	int32 CurrentHealth;      // Vita attuale

	UPROPERTY(EditAnywhere, Category = "Stats")
	int32 AttackRange;        // Distanza di attacco

	UPROPERTY(EditAnywhere, Category = "Stats")
	int32 MinDamage;          // Danno minimo inflitto

	UPROPERTY(EditAnywhere, Category = "Stats")
	int32 MaxDamage;          // Danno massimo inflitto

	// Nome visualizzato dell’unità nel widget
	UPROPERTY(EditAnywhere, Category = "Unit Info")
	FString UnitDisplayName;

	// Sovrascrive la funzione di Unreal per la gestione del danno ricevuto
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Verifica se l’unità è morta (vita <= 0)
	bool IsDead() const;

	// Ritorna la percentuale di vita attuale (per la barra della vita)
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;

	// Ritorna la vita attuale (come valore assoluto)
	float GetHealth() const { return CurrentHealth; }

	// Esegue un attacco contro un’altra unità
	void AttackUnit(AUnitBase* Target);

	// Verifica se l'unità può ancora agire in questo turno
	bool CanAct() const;

	// Getter dello stato corrente dell'unità
	EUnitAction GetCurrentAction() const { return CurrentAction; }

	// Setter dello stato corrente
	void SetCurrentAction(EUnitAction NewAction) { CurrentAction = NewAction; }

	// Reset dello stato a inizio turno
	void ResetAction() { CurrentAction = EUnitAction::Idle; }

	// Gestione della morte dell'unità
	UFUNCTION()
	void Die(AUnitBase* Target);

protected:
	// Funzione chiamata all’inizio del gioco
	virtual void BeginPlay() override;

	// Distanza di movimento massima per turno
	UPROPERTY(EditAnywhere, Category = "Unit Stats")
	int32 MovementRange;

	//  non necessario se si usa bIsRangedAttack
	UPROPERTY()
	bool bIsRangeAttack = false;

	// Mesh visiva dell’unità
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	// Funzione per inizializzare la mesh dell’unità
	virtual void SetupMesh(UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Scale);

	// ID della squadra (non usato attualmente, ma utile per futuri team multipli)
	UPROPERTY(EditAnywhere, Category = "Unit")
	int32 TeamID;

	// Specifica se l'unità è controllata dal giocatore
	UPROPERTY()
	bool bIsPlayerControlled = false;

	// Stato corrente dell’unità (Idle, Moved, ecc.)
	EUnitAction CurrentAction = EUnitAction::Idle;
};