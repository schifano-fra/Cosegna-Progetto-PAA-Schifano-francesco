#include "UITurnIndicator.h"
#include "InfoWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UUITurnIndicator::NativeConstruct()
{
	Super::NativeConstruct();
	if (TurnText)
	{
		TurnText->SetText(FText::FromString("Inizio Partita"));
	}
	if (HistoryBox)
	{
		HistoryBox->ClearChildren();
	}
	if (InfoButton)
	{
		InfoButton->OnClicked.AddDynamic(this, &UUITurnIndicator::OnInfoButtonClicked);
	}
}

void UUITurnIndicator::UpdateTurnText(FString PlayerName)
{
	if (TurnText)
	{
		TurnText->SetText(FText::FromString("Turno di: " + PlayerName));
	}
}

void UUITurnIndicator::AddHistoryEntry(const FString& Entry)
{
	if (!HistoryBox) return;

	UTextBlock* NewText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
	if (NewText)
	{
		NewText->SetText(FText::FromString(Entry));
		NewText->Font.Size = 16;
		HistoryBox->AddChildToVerticalBox(NewText);
	}
}

void UUITurnIndicator::OnInfoButtonClicked()
{
	if (!InfoWidgetClass) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	if (!InfoWidgetInstance)
	{
		InfoWidgetInstance = CreateWidget<UInfoWidget>(PC, InfoWidgetClass);
	}

	if (InfoWidgetInstance && !InfoWidgetInstance->IsInViewport())
	{
		InfoWidgetInstance->AddToViewport();
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}
}
