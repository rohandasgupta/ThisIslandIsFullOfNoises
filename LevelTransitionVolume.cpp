// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTransitionVolume.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Main.h"

// Sets default values
ALevelTransitionVolume::ALevelTransitionVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	TransitionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TransitionVolume"));

	RootComponent = TransitionVolume;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));

	Billboard->SetupAttachment(GetRootComponent());

	levelname = "SunTemple";
}

// Called when the game starts or when spawned
void ALevelTransitionVolume::BeginPlay()
{
	Super::BeginPlay();
	
	TransitionVolume->OnComponentBeginOverlap.AddDynamic(this, &ALevelTransitionVolume::OnOverlapBegin);
}

// Called every frame
void ALevelTransitionVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelTransitionVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMain* main = Cast<AMain>(OtherActor);
		if (main) {

			calcStar(main);

			if (levelname == FName("Level2_Gorge") && !main->bcomplevel1) {
				changeHUDTEXT(main);
			}
			else if (levelname == FName("Level3") && !main->bcomplevel2)
			{
				changeHUDTEXT(main);
			}
			else if (levelname == FName("Level4") && !main->bcomplevel3)
			{
				changeHUDTEXT(main);
			}
			else {
				main->SaveGame();
				main->switchLevel(levelname);
				main->LoadGameHub();
			}
		}
	}
}

void ALevelTransitionVolume::changeHUDTEXT_Implementation(AMain* main)
{
	;
}

void ALevelTransitionVolume::calcStar_Implementation(AMain* main)
{
	;
}