// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TutRerunSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FCharacterStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	float health;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	float maxHealth;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	float stamina;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	bool blevel1comp;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	bool blevel2comp;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	bool blevel3comp;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	bool blevel4comp;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	float maxStamina;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	int32 coins;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	FVector location;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	FRotator rotation;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	FString weaponName;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
	FString levelName;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
		int level1stars;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
		int level2stars;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
		int level3stars;

	UPROPERTY(VisibleAnywhere, Category = "Save Game Data")
		int level4stars;
};

/**
 * 
 */
UCLASS()
class TUTRERUN_API UTutRerunSaveGame : public USaveGame
{
	GENERATED_BODY()
public:

	UTutRerunSaveGame();

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString PlayerName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FCharacterStats CharacterStats;

};
