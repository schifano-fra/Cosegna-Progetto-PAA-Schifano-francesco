#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class PAASCHIFANOFRANCESCO_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWinnerText(const FString& Winner);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClicked_Exit();

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WinnerText;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
};