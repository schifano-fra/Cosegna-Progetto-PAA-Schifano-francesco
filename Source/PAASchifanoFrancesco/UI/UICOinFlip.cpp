// Creato da: Schifano Francesco, 5469994

// Include della classe UICOinFlip (header associato)
#include "UICOinFlip.h"

// Include dei componenti UI utilizzati nel widget
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Include del GameMode personalizzato del progetto
#include "PAASchifanoFrancesco/Core/MyGameMode.h"

// Include per la gestione delle animazioni nei widget UMG
#include "Animation/WidgetAnimation.h"

// Funzioni di utilità per ottenere riferimenti al GameMode
#include "Kismet/GameplayStatics.h"

/**
 * Metodo: NativeConstruct
 * Descrizione: Override del metodo chiamato automaticamente quando il widget viene inizializzato a runtime.
 * Collega eventi ai pulsanti, aggiorna il testo del risultato e avvia l'animazione della moneta.
 */
void UUICOinFlip::NativeConstruct()
{
	Super::NativeConstruct(); // Chiama la versione base del metodo

	// Ottiene riferimento al GameMode
	GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));

	// Se il GameMode è valido e il testo è disponibile, mostra il risultato del lancio della moneta
	if (GameMode && CoinFlipResultText)
	{
		// Determina chi inizia la partita e aggiorna il testo visibile nel widget
		FString ResultText = (GameMode->GetCoinFlipResult() == EPlayer::Player1)
			? TEXT("Player Starts!")    // Se vince il giocatore
			: TEXT("AI Starts!");       // Se vince l’IA

		// Imposta il testo nel blocco visivo
		CoinFlipResultText->SetText(FText::FromString(ResultText));
	}

	// Collega il pulsante "Easy" all'evento OnEasyClicked
	if (ButtonEasy)
		ButtonEasy->OnClicked.AddDynamic(this, &UUICOinFlip::OnEasyClicked);

	// Collega il pulsante "Hard" all'evento OnHardClicked
	if (ButtonHard)
		ButtonHard->OnClicked.AddDynamic(this, &UUICOinFlip::OnHardClicked);

	// Se esistono l’immagine della moneta e l’animazione, imposta la velocità
	if (CoinImage && FlipAnimation)
	{
		SetFlipAnimationSpeed(20.0f); // Imposta la velocità dell’animazione della moneta
	}
}

/**
 * Metodo: OnEasyClicked
 * Descrizione: Gestisce il click sul pulsante "Easy", imposta la difficoltà a facile e avanza alla fase di piazzamento.
 */
void UUICOinFlip::OnEasyClicked()
{
	if (GameMode)
	{
		GameMode->AILevel = EAILevel::Easy;                   // Imposta la difficoltà dell’IA
		GameMode->SetGamePhase(EGamePhase::EPlacement);       // Passa alla fase di piazzamento
	}
}

/**
 * Metodo: OnHardClicked
 * Descrizione: Gestisce il click sul pulsante "Hard", imposta la difficoltà a difficile e avanza alla fase di piazzamento.
 */
void UUICOinFlip::OnHardClicked()
{
	if (GameMode)
	{
		GameMode->AILevel = EAILevel::Hard;                   // Imposta la difficoltà dell’IA
		GameMode->SetGamePhase(EGamePhase::EPlacement);       // Passa alla fase di piazzamento
	}
}

/**
 * Metodo: SetFlipAnimationSpeed
 * Descrizione: Riproduce l’animazione della moneta alla velocità specificata.
 * @param Speed - velocità di riproduzione dell’animazione.
 */
void UUICOinFlip::SetFlipAnimationSpeed(float Speed)
{
	if (FlipAnimation)
	{
		// Riproduce l’animazione dalla posizione iniziale, in loop infinito, alla velocità specificata
		PlayAnimation(FlipAnimation, 0.0f, 0, EUMGSequencePlayMode::Forward, Speed);
	}
}
