#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UITurnIndicator.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UInfoWidget;

UCLASS()
class PAASCHIFANOFRANCESCO_API UUITurnIndicator : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateTurnText(FString PlayerName);

	UFUNCTION(BlueprintCallable)
	void AddHistoryEntry(const FString& Entry);

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UInfoWidget> InfoWidgetClass;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnInfoButtonClicked();
	
	UPROPERTY(meta = (BindWidget))
	UButton* InfoButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TurnText;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* HistoryBox;

private:
	UPROPERTY()
	UInfoWidget* InfoWidgetInstance;
};