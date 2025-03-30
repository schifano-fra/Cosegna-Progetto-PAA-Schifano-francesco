// Fill out your copyright notice in the Description page of Project Settings.

#include "Tile.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = false;

	// Create the Static Mesh component
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	RootComponent = TileMesh;

	// Load the mesh asset
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (MeshRef.Succeeded())
	{
		TileMesh->SetStaticMesh(MeshRef.Object);
	}

	// Carica materiale albero
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TreeMaterialRef(TEXT("/Game/Material/Tree.Tree"));
	if(TreeMaterialRef.Succeeded())
	{
		TreeMaterial = UMaterialInstanceDynamic::Create(TreeMaterialRef.Object, this);
	}

	// Carica materiale montagna
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MountainMaterialRef(TEXT("/Game/Material/Mountain.Mountain"));
	if(MountainMaterialRef.Succeeded())
	{
		MountainMaterial = UMaterialInstanceDynamic::Create(MountainMaterialRef.Object, this);
	}

	// Load normal material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterialRef(TEXT("/Game/Material/Tile.Tile"));
	if (NormalMaterialRef.Succeeded())
	{
		NormalMaterial = UMaterialInstanceDynamic::Create(NormalMaterialRef.Object, this);
	}

	bIsObstacle = false;
	bHasPawn = false;

	//TileMesh->OnClicked.AddDynamic(this, &ATile::HandleTileClicked);
}

void ATile::BeginPlay()
{
	Super::BeginPlay();
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	TileMesh->SetGenerateOverlapEvents(false);
	
	EnableInput(GetWorld()->GetFirstPlayerController());
	SetAsObstacle(bIsObstacle);
}

// Sets the tile as an obstacle
void ATile::SetAsObstacle(bool NewbIsObstacle)
{
	bIsObstacle = NewbIsObstacle;

	if(bIsObstacle)
	{
		// Scegli casualmente tra i due materiali
		const bool bUseTree = FMath::RandBool();
		if(bUseTree && TreeMaterial)
		{
			TileMesh->SetMaterial(0, TreeMaterial);
		}
		else if(MountainMaterial)
		{
			TileMesh->SetMaterial(0, MountainMaterial);
		}
	}
	else if(NormalMaterial)
	{
		TileMesh->SetMaterial(0, NormalMaterial);
	}
	TileMesh->SetCollisionObjectType(bIsObstacle ? ECC_GameTraceChannel1 : ECC_WorldStatic);
}

void ATile::SetHighlight(bool bHighlight, FLinearColor HighlightColor)
{

	if (NormalMaterial)
	{
		NormalMaterial->SetVectorParameterValue("Color", HighlightColor);
		TileMesh->SetMaterial(0, NormalMaterial); // Assicura che il materiale venga riassegnato
	}
}

FVector ATile::GetPawnSpawnLocation() const
{
	return GetActorLocation() + FVector(0, 0, 50); // 50 unità sopra il tile
}

void ATile::SetTileIdentifier(const FString& NewIdentifier)
{
	TileIdentifier = NewIdentifier;
}

void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}