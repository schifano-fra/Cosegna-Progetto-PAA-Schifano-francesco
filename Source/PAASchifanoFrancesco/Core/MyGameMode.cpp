#include "MyGameMode.h" // Include l'header del GameMode personalizzato
#include "BattleManager.h" // Include la logica di battaglia
#include "PlacementManager.h" // Include il manager per il piazzamento delle unità
#include "TurnManager.h" // Include la gestione dei turni
#include "PAASchifanoFrancesco/UI/UICOinFlip.h" // Include il widget per il lancio della moneta
#include "PAASchifanoFrancesco/UI/GameOverWidget.h" // Include il widget di fine gioco
#include "PAASchifanoFrancesco/UI/UIMainMenu.h" // Include il menu principale
#include "PAASchifanoFrancesco/UI/SelectPawn.h" // Include il widget per la selezione delle pedine
#include "PAASchifanoFrancesco/Input/CameraPawn.h" // Include il Pawn per la visuale dall'alto
#include "PAASchifanoFrancesco/Input/MyPlayerController.h" // Include il PlayerController personalizzato
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h" // Include il manager del movimento
#include "Blueprint/UserWidget.h" // Include per usare i widget in C++
#include "Engine/World.h" // Include per accedere al mondo
#include "GameFramework/PlayerController.h" // Include per accedere ai controller
#include "Blueprint/WidgetBlueprintLibrary.h" // Include per operazioni generiche sui widget
#include "UObject/ConstructorHelpers.h" // Include per cercare blueprint/classi da codice

/**
* Descrizione: Costruttore della classe AMyGameMode. Viene eseguito all'avvio del gioco o quando viene istanziato
* il GameMode. In questo metodo:
* Si inizializza la fase di gioco a EInitial.
* Si caricano tutti i Widget Blueprint tramite ConstructorHelpers::FClassFinder.
* Si impostano il PlayerControllerClass e il DefaultPawnClass.
 */
AMyGameMode::AMyGameMode()
{
    // Imposta la fase iniziale del gioco su "EInitial"
    CurrentGamePhase = EGamePhase::EInitial;

    // Carica la classe del Main Menu da blueprint
    static ConstructorHelpers::FClassFinder<UUIMainMenu> MainMenuBPClass(TEXT("WidgetBlueprint'/Game/UI/UIMainMenu.UIMainMenu_C'"));
    if (MainMenuBPClass.Succeeded())
    {
        MainMenuClass = MainMenuBPClass.Class;
    }

    // Carica la classe del CoinFlip Widget da blueprint
    static ConstructorHelpers::FClassFinder<UUICOinFlip> CoinFlipBPClass(TEXT("WidgetBlueprint'/Game/UI/UICoinFlip.UICoinFlip_C'"));
    if (CoinFlipBPClass.Succeeded())
    {
        CoinFlipWidgetClass = CoinFlipBPClass.Class;
    }
    
    // Carica la classe del widget di selezione pedine
    static ConstructorHelpers::FClassFinder<USelectPawn> SelectPawnBPClass(TEXT("WidgetBlueprint'/Game/UI/UISelectPawn.UISelectPawn_C'"));
    if (SelectPawnBPClass.Succeeded())
    {
        SelectPawnClass = SelectPawnBPClass.Class;
    }

    // Carica la classe del Turn Indicator
    static ConstructorHelpers::FClassFinder<UUITurnIndicator> TurnIndicatorBPClass(TEXT("WidgetBlueprint'/Game/UI/UITurnIndicator.UITurnIndicator_C'"));
    if (TurnIndicatorBPClass.Succeeded())
    {
        TurnIndicatorWidgetClass = TurnIndicatorBPClass.Class;
    }

    // Carica la classe del widget dello stato delle unità
    static ConstructorHelpers::FClassFinder<UStatusGameWidget> StatusGameBPClass(TEXT("WidgetBlueprint'/Game/UI/UIStatusGame.UIStatusGame_C'"));
    if (StatusGameBPClass.Succeeded())
    {
        StatusGameWidgetClass = StatusGameBPClass.Class;
    }

    // Carica la classe del widget di Game Over
    static ConstructorHelpers::FClassFinder<UGameOverWidget> GameOverBPClass(TEXT("WidgetBlueprint'/Game/UI/UIGameOver.UIGameOver_C'"));
    if (GameOverBPClass.Succeeded())
    {
        GameOverWidgetClass = GameOverBPClass.Class;
    }

    // Carica la classe del widget informativo
    static ConstructorHelpers::FClassFinder<UInfoWidget> InfoBPClass(TEXT("WidgetBlueprint'/Game/UI/UIInfo.UIInfo_C'"));
    if (InfoBPClass.Succeeded())
    {
        InfoWidgetClass = InfoBPClass.Class;
    }

    // Imposta il controller usato dal giocatore
    PlayerControllerClass = AMyPlayerController::StaticClass();

    // Imposta il pawn usato (telecamera dall’alto)
    DefaultPawnClass = ACameraPawn::StaticClass();

    // Imposta il giocatore che inizia per default su Player1
    StartingPlayer = EPlayer::Player1;
}

/**
* Descrizione: Metodo chiamato automaticamente all'inizio della partita. Qui vengono inizializzati i componenti
* fondamentali della scena, tra cui:
* Il GridManager per la griglia e gli ostacoli.
* Il GlobalMovementManager per gestire i movimenti.
* Il StatusGameWidget per mostrare la vita delle unità.
* Infine, viene eseguita la logica della fase iniziale (HandleInitialPhase()).
 */
void AMyGameMode::BeginPlay()
{
    // Chiama la versione base di BeginPlay (superclasse)
    Super::BeginPlay();

    // Se non è ancora stato creato un GridManager, lo istanzia
    if (!GridManager)
    {
        GridManager = GetWorld()->SpawnActor<AGridManager>(AGridManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
        // Se la creazione ha avuto successo
        if (GridManager)
        {
            // Genera la griglia 25x25
            GridManager->GenerateGrid();
            // Genera gli ostacoli sulla griglia
            GridManager->GenerateObstacles();
            // Log di conferma creazione
            UE_LOG(LogTemp, Warning, TEXT("GridManager creato all'avvio!"));
        }
    }

    // Notifica che la fase attuale è attiva (utile per bind esterni)
    OnGamePhaseChanged.Broadcast(CurrentGamePhase);
    // Istanzia il manager globale dei movimenti
    GlobalMovementManager = GetWorld()->SpawnActor<AUnitMovementManager>();
    // Se il widget StatusGameWidget è stato assegnato correttamente
    if (StatusGameWidgetClass)
    {
        // Crea il widget e lo salva nella variabile StatusGameWidget
        StatusGameWidget = CreateWidget<UStatusGameWidget>(GetWorld(), StatusGameWidgetClass);
    }

    // Avvia la fase iniziale del gioco (menu principale)
    HandleInitialPhase();
}

/**
 * Metodo: HandleInitialPhase
 * Descrizione: Mostra il menu principale nella fase iniziale.
 */
void AMyGameMode::HandleInitialPhase()
{
    // Se è stata assegnata la classe del menu principale
    if (MainMenuClass)
    {
        // Ottiene il controller del giocatore
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            // Crea il widget del menu principale
            UUIMainMenu* MainMenu = CreateWidget<UUIMainMenu>(PC, MainMenuClass);
            if (MainMenu)
            {
                // Aggiunge il menu alla viewport
                MainMenu->AddToViewport();
                // Abilita il cursore del mouse
                PC->bShowMouseCursor = true;
                // Imposta la modalità input solo per UI
                PC->SetInputMode(FInputModeUIOnly());
            }
        }
    }
}

/**
 * Metodo: FlipCoin
 * Descrizione: Esegue un lancio di moneta per determinare il primo giocatore,
 *              crea il widget e inizializza il TurnManager.
 */
void AMyGameMode::FlipCoin()
{
    // Genera un numero casuale tra 0 e 1
    int FlipResult = FMath::RandRange(0, 1);
    // Imposta il giocatore iniziale in base al risultato
    StartingPlayer = (FlipResult == 0) ? EPlayer::Player1 : EPlayer::AI;

    // Stampa a log il risultato del lancio
    UE_LOG(LogTemp, Warning, TEXT("Coin Flip Result: %s"), *UEnum::GetValueAsString(StartingPlayer));

    // Ottiene il controller del giocatore
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && CoinFlipWidgetClass)
    {
        // Crea il widget del coin flip
        CoinFlipWidget = CreateWidget<UUICOinFlip>(PC, CoinFlipWidgetClass);
        if (CoinFlipWidget)
        {
            // Aggiunge il widget alla viewport
            CoinFlipWidget->AddToViewport();
        }
        // Imposta input in modalità gioco e mostra il cursore
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = true;
    }

    // Crea e inizializza il TurnManager
    TurnManager = NewObject<UTurnManager>(this);
    TurnManager->Initialize(this);
    TurnManager->SetInitialPlayer(StartingPlayer);
}

/**
 * Metodo: HandlePlacementPhase
 * Descrizione: Avvia la fase di piazzamento, mostra i widget necessari e inizializza il PlacementManager.
 */
void AMyGameMode::HandlePlacementPhase()
{
    // Rimuove tutti i widget attualmente visibili
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UUserWidget::StaticClass(), false);
    for (UUserWidget* Widget : FoundWidgets)
    {
        Widget->RemoveFromParent();
    }

    // Crea il PlacementManager
    PlacementManager = GetWorld()->SpawnActor<APlacementManager>(APlacementManager::StaticClass());
    if (PlacementManager)
    {
        // Se è stata assegnata la classe del TurnIndicator
        if (TurnIndicatorWidgetClass)
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                // Usa NewObject per assegnare prima le proprietà al widget
                TurnIndicatorWidget = NewObject<UUITurnIndicator>(this, TurnIndicatorWidgetClass);
                if (TurnIndicatorWidget)
                {
                    // Assegna la classe del widget informativo
                    TurnIndicatorWidget->InfoWidgetClass = InfoWidgetClass;
                    // Mostra il widget nella viewport
                    TurnIndicatorWidget->AddToViewport();
                }
                // Imposta input in modalità gioco e abilita il mouse
                PC->SetInputMode(FInputModeGameOnly());
                PC->bShowMouseCursor = true;
            }
        }

        // Mostra lo status widget (barra della vita)
        if (StatusGameWidget)
        {
            StatusGameWidget->AddToViewport();
        }

        // Inizializza il PlacementManager e avvia il piazzamento
        PlacementManager->Initialize(SelectPawnClass);
        PlacementManager->StartPlacement();
    }
}

/**
 * Metodo: HandleBattlePhase
 * Descrizione: Inizializza e avvia la fase di battaglia.
 */
void AMyGameMode::HandleBattlePhase()
{
    // Istanzia il BattleManager nella scena
    BattleManager = GetWorld()->SpawnActor<ABattleManager>(ABattleManager::StaticClass());

    if (BattleManager)
    {
        // Mostra lo status widget se disponibile
        if (StatusGameWidget)
        {
            StatusGameWidget->AddToViewport();
        }
        // Inizializza TurnManager con il BattleManager
        TurnManager->Initialize(this, BattleManager);
        // Imposta il giocatore iniziale
        TurnManager->SetInitialPlayer(StartingPlayer);
        // Avvia la battaglia
        BattleManager->StartBattle();
    }
}

/**
 * Metodo: HandleGameOver
 * Descrizione: Gestisce la fine della partita e mostra il vincitore nel widget dedicato.
 */
void AMyGameMode::HandleGameOver(const FString& WinnerName)
{
    // Se il widget GameOver non è assegnato, esce
    if (!GameOverWidgetClass) return;

    // Stampa a log il nome del vincitore
    UE_LOG(LogTemp, Warning, TEXT("Game Over! Vincitore: %s"), *WinnerName);

    // Ottiene il controller del giocatore
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    // Crea il widget di game over
    GameOverWidget = CreateWidget<UGameOverWidget>(PC, GameOverWidgetClass);
    if (!GameOverWidget) return;

    // Mostra il widget e imposta il testo con il nome del vincitore
    GameOverWidget->AddToViewport();
    GameOverWidget->SetWinnerText(WinnerName);

    // Abilita il cursore e input solo UI
    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeUIOnly());
}

/**
 * Metodo: AddMoveToHistory
 * Descrizione: Aggiunge una stringa allo storico visivo del widget del turno.
 */
void AMyGameMode::AddMoveToHistory(const FString& Entry)
{
    if (TurnIndicatorWidget)
    {
        TurnIndicatorWidget->AddHistoryEntry(Entry);
    }
}

/**
 * Metodo: SetGamePhase
 * Descrizione: Cambia la fase di gioco e gestisce il flusso tra le fasi.
 */
void AMyGameMode::SetGamePhase(EGamePhase NewPhase)
{
    // Se la fase è già quella attuale, non fa nulla
    if (CurrentGamePhase != NewPhase)
    {
        // Aggiorna la fase corrente
        CurrentGamePhase = NewPhase;

        // Log del cambio fase
        UE_LOG(LogTemp, Warning, TEXT("Fase di gioco cambiata: %s"), *UEnum::GetValueAsString(NewPhase));

        // Notifica a eventuali listener il cambio fase
        OnGamePhaseChanged.Broadcast(NewPhase);

        // In base alla nuova fase, chiama la funzione corrispondente
        switch (NewPhase)
        {
        case EGamePhase::EInitial:
            HandleInitialPhase();
            break;
        case EGamePhase::ECoinFlip:
            FlipCoin();
            break;
        case EGamePhase::EPlacement:
            HandlePlacementPhase();
            break;
        case EGamePhase::EBattle:
            HandleBattlePhase();
            break;
        case EGamePhase::EGameOver:
            HandleGameOver(PlayerUnits.Num() == 0 ? TEXT("AI") : TEXT("Player"));
            break;
        default:
            break;
        }
    }
}