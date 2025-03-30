#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"  // ✅ QUESTA RIGA RISOLVE IL PROBLEMA
#include "StatusGameWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UProgressBar;

UCLASS()
class PAASCHIFANOFRANCESCO_API UStatusGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void OnClickedEndTurn();

	UFUNCTION()
	void ActiveButton(bool bIsVisible);

	// 🔴 Nuovo: Aggiungi e gestisci le barre della vita
	void AddUnitStatus(AUnitBase* Unit);
	void UpdateUnitHealth(AUnitBase* Unit, float NewHealthPercent);
	void RemoveUnitStatus(AUnitBase* Unit);

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* EndButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ButtonText;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* UnitStatusBox;

private:
	TMap<AUnitBase*, TPair<UProgressBar*, UBorder*>> UnitHealthBars;
};