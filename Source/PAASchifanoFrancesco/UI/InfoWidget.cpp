#include "InfoWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UInfoWidget::OnBackClicked);
	}

	// Le immagini FirstInformation e SecondInformation sono già posizionate nel designer
	// Non è necessario settarle via codice se sono statiche
}

void UInfoWidget::OnBackClicked()
{
	RemoveFromParent();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = true;
	}
}