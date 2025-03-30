#include "GameOverWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
    
	if (ExitButton)
		ExitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnClicked_Exit);
}

void UGameOverWidget::SetWinnerText(const FString& WinnerName)
{
	if (WinnerText)
	{
		WinnerText->SetText(FText::FromString(FString::Printf(TEXT("Vittoria: %s"), *WinnerName)));
	}
}

void UGameOverWidget::OnClicked_Exit()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}