#include "MyGameMode.h"
#include "BattleManager.h"
#include "PlacementManager.h"
#include "TurnManager.h"
#include "PAASchifanoFrancesco/UI/UICOinFlip.h"
#include "PAASchifanoFrancesco/UI/GameOverWidget.h"
#include "PAASchifanoFrancesco/UI/UIMainMenu.h"
#include "PAASchifanoFrancesco/UI/SelectPawn.h"
#include "PAASchifanoFrancesco/Input/CameraPawn.h"
#include "PAASchifanoFrancesco/Input/MyPlayerController.h"
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UObject/ConstructorHelpers.h"

AMyGameMode::AMyGameMode()
{
    CurrentGamePhase = EGamePhase::EInitial;

    static ConstructorHelpers::FClassFinder<UUIMainMenu> MainMenuBPClass(TEXT("WidgetBlueprint'/Game/UI/UIMainMenu.UIMainMenu_C'"));
    if (MainMenuBPClass.Succeeded())
    {
        MainMenuClass = MainMenuBPClass.Class;
    }

    static ConstructorHelpers::FClassFinder<UUICOinFlip> CoinFlipBPClass(TEXT("WidgetBlueprint'/Game/UI/UICoinFlip.UICoinFlip_C'"));
    if (CoinFlipBPClass.Succeeded())
    {
        CoinFlipWidgetClass = CoinFlipBPClass.Class;
    }
    
    static ConstructorHelpers::FClassFinder<USelectPawn> SelectPawnBPClass(TEXT("WidgetBlueprint'/Game/UI/UISelectPawn.UISelectPawn_C'"));
    if (SelectPawnBPClass.Succeeded())
    {
        SelectPawnClass = SelectPawnBPClass.Class;
    }

    static ConstructorHelpers::FClassFinder<UUITurnIndicator> TurnIndicatorBPClass(TEXT("WidgetBlueprint'/Game/UI/UITurnIndicator.UITurnIndicator_C'"));
    if (TurnIndicatorBPClass.Succeeded())
    {
        TurnIndicatorWidgetClass = TurnIndicatorBPClass.Class;
    }

    static ConstructorHelpers::FClassFinder<UStatusGameWidget> StatusGameBPClass(TEXT("WidgetBlueprint'/Game/UI/UIStatusGame.UIStatusGame_C'"));
    if (StatusGameBPClass.Succeeded())
    {
        StatusGameWidgetClass = StatusGameBPClass.Class;
    }

    static ConstructorHelpers::FClassFinder<UGameOverWidget> GameOverBPClass(TEXT("WidgetBlueprint'/Game/UI/UIGameOver.UIGameOver_C'"));
    if (GameOverBPClass.Succeeded())
    {
        GameOverWidgetClass = GameOverBPClass.Class;
    }

    static ConstructorHelpers::FClassFinder<UInfoWidget> InfoBPClass(TEXT("WidgetBlueprint'/Game/UI/UIInfo.UIInfo_C'"));
    if (InfoBPClass.Succeeded())
    {
        InfoWidgetClass = InfoBPClass.Class;
    }

    PlayerControllerClass = AMyPlayerController::StaticClass();
    DefaultPawnClass = ACameraPawn::StaticClass();
    StartingPlayer = EPlayer::Player1;
}

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (!GridManager)
    {
        GridManager = GetWorld()->SpawnActor<AGridManager>(AGridManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
        if (GridManager)
        {
            GridManager->GenerateGrid();
            GridManager->GenerateObstacles();
            UE_LOG(LogTemp, Warning, TEXT("✅ GridManager creato all'avvio!"));
        }
    }
    
    OnGamePhaseChanged.Broadcast(CurrentGamePhase);
    GlobalMovementManager = GetWorld()->SpawnActor<AUnitMovementManager>();
    if (StatusGameWidgetClass)
    {
        StatusGameWidget = CreateWidget<UStatusGameWidget>(GetWorld(), StatusGameWidgetClass);
    }
    
    
    HandleInitialPhase();
}

void AMyGameMode::HandleInitialPhase()
{
    if (MainMenuClass)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            UUIMainMenu* MainMenu = CreateWidget<UUIMainMenu>(PC, MainMenuClass);
            if (MainMenu)
            {
                MainMenu->AddToViewport();
                PC->bShowMouseCursor = true;
                PC->SetInputMode(FInputModeUIOnly());
            }
        }
    }
}

void AMyGameMode::FlipCoin()
{
    int FlipResult = FMath::RandRange(0, 1);  // Genera 0 o 1 casuale
    StartingPlayer = (FlipResult == 0) ? EPlayer::Player1 : EPlayer::AI;
    
    UE_LOG(LogTemp, Warning, TEXT("Coin Flip Result: %s"), *UEnum::GetValueAsString(StartingPlayer));

    // Mostra il widget UI del Coin Flip
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && CoinFlipWidgetClass)
    {
        CoinFlipWidget = CreateWidget<UUICOinFlip>(PC, CoinFlipWidgetClass);
        if (CoinFlipWidget)
        {
            CoinFlipWidget->AddToViewport();
        }
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = true;
    }
    
    TurnManager = NewObject<UTurnManager>(this);
    TurnManager->Initialize(this);
    TurnManager->SetInitialPlayer(StartingPlayer);
}

void AMyGameMode::HandlePlacementPhase()
{
    // ✅ Rimuove TUTTI i widget attualmente a schermo
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UUserWidget::StaticClass(), false);
    for (UUserWidget* Widget : FoundWidgets)
    {
        Widget->RemoveFromParent();
    }

    PlacementManager = GetWorld()->SpawnActor<APlacementManager>(APlacementManager::StaticClass());
    if (PlacementManager)
    {
        if (TurnIndicatorWidgetClass)
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                if (TurnIndicatorWidgetClass)
                {
                    // CreateWidget chiama automaticamente NativeConstruct,
                    // quindi usiamo NewObject per assegnare prima la variabile
                    TurnIndicatorWidget = NewObject<UUITurnIndicator>(this, TurnIndicatorWidgetClass);
                    if (TurnIndicatorWidget)
                    {
                        TurnIndicatorWidget->InfoWidgetClass = InfoWidgetClass;  // 🔧 ASSEGNA PRIMA
                        TurnIndicatorWidget->AddToViewport();                     // ✅ Solo dopo
                    }
                }
                PC->SetInputMode(FInputModeGameOnly());
                PC->bShowMouseCursor = true;
            }
        }
        // ✅ Mostra StatusGameWidget (aggiunta necessaria dopo reset)
        if (StatusGameWidget)
        {
            StatusGameWidget->AddToViewport();
        }

        PlacementManager->Initialize(SelectPawnClass);
        PlacementManager->StartPlacement();
    }
}

void AMyGameMode::InitGridManager()
{
    if (!GridManager)
    {
        GridManager = GetWorld()->SpawnActor<AGridManager>(AGridManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
        if (GridManager)
        {
            GridManager->GenerateGrid();
            GridManager->GenerateObstacles();
            UE_LOG(LogTemp, Warning, TEXT("✅ GridManager ricreato nel reset!"));
        }
    }
}


APlacementManager* AMyGameMode::GetPlacementManager() const
{
    return PlacementManager;
}

void AMyGameMode::HandleBattlePhase()
{
    BattleManager = GetWorld()->SpawnActor<ABattleManager>(ABattleManager::StaticClass());

    if (BattleManager)
    {
        if (StatusGameWidget)
        {
            StatusGameWidget->AddToViewport();
        }
        TurnManager->Initialize(this, BattleManager);
        TurnManager->SetInitialPlayer(StartingPlayer);
        BattleManager->StartBattle();
    }
}

void AMyGameMode::HandleGameOver(const FString& WinnerName)
{
    
    if (!GameOverWidgetClass) return;
    UE_LOG(LogTemp, Warning, TEXT("🎉 Game Over! Vincitore: %s"), *WinnerName);
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    GameOverWidget = CreateWidget<UGameOverWidget>(PC, GameOverWidgetClass);
    if (!GameOverWidget) return;

    GameOverWidget->AddToViewport();
    GameOverWidget->SetWinnerText(WinnerName);  // 🎯 Qui stampi il vincitore

    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeUIOnly());
}

void AMyGameMode::AddMoveToHistory(const FString& Entry)
{
    if (TurnIndicatorWidget)
    {
        TurnIndicatorWidget->AddHistoryEntry(Entry);
    }
}

void AMyGameMode::SetGamePhase(EGamePhase NewPhase)
{
    if (CurrentGamePhase != NewPhase)
    {
        CurrentGamePhase = NewPhase;
        UE_LOG(LogTemp, Warning, TEXT("Fase di gioco cambiata: %s"), *UEnum::GetValueAsString(NewPhase));
        OnGamePhaseChanged.Broadcast(NewPhase);

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