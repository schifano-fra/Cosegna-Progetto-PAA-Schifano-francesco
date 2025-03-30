#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InfoWidget.generated.h"

class UButton;
class UImage;

UCLASS()
class PAASCHIFANOFRANCESCO_API UInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnBackClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UImage* FirstInformation;

	UPROPERTY(meta = (BindWidget))
	UImage* SecondInformation;

	UPROPERTY(meta = (BindWidget))
	UImage* PawnDetails;
};