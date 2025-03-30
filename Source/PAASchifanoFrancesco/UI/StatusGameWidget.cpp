#include "StatusGameWidget.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Core/TurnManager.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"

void UStatusGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EndButton)
	{
		EndButton->SetIsEnabled(false);
		EndButton->SetVisibility(ESlateVisibility::Hidden);
		EndButton->OnClicked.AddDynamic(this, &UStatusGameWidget::OnClickedEndTurn);
	}
}

void UStatusGameWidget::OnClickedEndTurn()
{
	if (AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		if (UTurnManager* TurnManager = GM->GetTurnManager())
		{
			TurnManager->EndTurn();
			EndButton->SetVisibility(ESlateVisibility::Hidden);
			EndButton->SetIsEnabled(false);
		}
	}
}

void UStatusGameWidget::ActiveButton(bool bIsVisible)
{
	if (EndButton)
	{
		EndButton->SetIsEnabled(bIsVisible);
		EndButton->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UStatusGameWidget::AddUnitStatus(AUnitBase* Unit)
{
	if (!Unit || !UnitStatusBox || UnitHealthBars.Contains(Unit)) return;

	// 🔳 Contenitore con sfondo bianco
	UBorder* BackgroundBox = NewObject<UBorder>(this);
	BackgroundBox->SetBrushColor(FLinearColor::White);
	BackgroundBox->SetPadding(FMargin(10.f));
	BackgroundBox->SetHorizontalAlignment(HAlign_Fill);

	// 🧱 VBox con nome sopra e barra sotto
	UVerticalBox* InnerBox = NewObject<UVerticalBox>(this);

	// 🔤 Nome unità
	UTextBlock* NameText = NewObject<UTextBlock>(this);
	NameText->SetText(FText::FromString(Unit->UnitDisplayName));
	FSlateFontInfo FontInfo = NameText->GetFont();
	FontInfo.Size = 24;
	NameText->SetFont(FontInfo);
	NameText->SetJustification(ETextJustify::Left);
	NameText->SetColorAndOpacity(Unit->IsPlayerControlled() ? FSlateColor(FColor::Blue) : FSlateColor(FColor(139, 69, 19)));

	UVerticalBoxSlot* NameSlot = InnerBox->AddChildToVerticalBox(NameText);
	NameSlot->SetPadding(FMargin(5.f, 5.f, 5.f, 2.f));
	NameSlot->SetHorizontalAlignment(HAlign_Left);

	// 🔋 Barra vita
	UProgressBar* HealthBar = NewObject<UProgressBar>(this);
	HealthBar->SetPercent(1.0f);
	HealthBar->SetFillColorAndOpacity(FLinearColor::Green);

	USizeBox* SizeBox = NewObject<USizeBox>(this);
	SizeBox->SetWidthOverride(300.f);
	SizeBox->SetHeightOverride(25.f);
	SizeBox->AddChild(HealthBar);

	UVerticalBoxSlot* BarSlot = InnerBox->AddChildToVerticalBox(SizeBox);
	BarSlot->SetPadding(FMargin(5.f, 2.f, 5.f, 5.f));
	BarSlot->SetHorizontalAlignment(HAlign_Left);

	BackgroundBox->SetContent(InnerBox);

	// Aggiungi il contenitore alla lista
	UVerticalBoxSlot* ContainerSlot = Cast<UVerticalBoxSlot>(UnitStatusBox->AddChild(BackgroundBox));
	if (ContainerSlot)
	{
		ContainerSlot->SetPadding(FMargin(10.f, 5.f, 10.f, 5.f));
		ContainerSlot->SetHorizontalAlignment(HAlign_Left);
	}

	// 🔗 Salva entrambi: barra + contenitore
	UnitHealthBars.Add(Unit, TPair<UProgressBar*, UBorder*>(HealthBar, BackgroundBox));
}

void UStatusGameWidget::UpdateUnitHealth(AUnitBase* Unit, float NewHealthPercent)
{
	if (!Unit || !IsValid(Unit))
	{
		UE_LOG(LogTemp, Error, TEXT("❌ UpdateUnitHealth: Unità nulla o non valida"));
		return;
	}

	if (!UnitHealthBars.Contains(Unit))
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ UpdateUnitHealth: Nessuna barra trovata per %s"), *Unit->GetName());
		return;
	}

	TPair<UProgressBar*, UBorder*>* FoundPair = UnitHealthBars.Find(Unit);
	if (!FoundPair)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ UpdateUnitHealth: Pair non trovato per %s"), *Unit->GetName());
		return;
	}

	UProgressBar* Bar = FoundPair->Key;
	if (!Bar)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ UpdateUnitHealth: Barra NULL per %s"), *Unit->GetName());
		return;
	}

	Bar->SetPercent(NewHealthPercent);

	if (NewHealthPercent < 0.3f)
	{
		Bar->SetFillColorAndOpacity(FLinearColor::Red);
	}
	else if (NewHealthPercent < 0.7f)
	{
		Bar->SetFillColorAndOpacity(FLinearColor::Yellow);
	}
	else
	{
		Bar->SetFillColorAndOpacity(FLinearColor::Green);
	}
}

void UStatusGameWidget::RemoveUnitStatus(AUnitBase* Unit)
{
	if (!UnitStatusBox || !UnitHealthBars.Contains(Unit)) return;

	TPair<UProgressBar*, UBorder*> Pair = UnitHealthBars[Unit];
	UBorder* Container = Pair.Value;

	if (Container && Container->IsValidLowLevel())
	{
		UnitStatusBox->RemoveChild(Container);
	}

	UnitHealthBars.Remove(Unit);
}