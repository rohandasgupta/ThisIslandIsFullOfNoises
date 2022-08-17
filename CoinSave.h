// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CoinSave.generated.h"

/**
 * 
 */
UCLASS()
class TUTRERUN_API UCoinSave : public USaveGame
{
	GENERATED_BODY()
public:
		UCoinSave();
	
	UPROPERTY(VisibleAnywhere, Category = Basic)
		FString PlayerName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
		uint32 UserIndex;

	UPROPERTY(VisibleAnywhere, Category = SaveData)
		int coins;

	UPROPERTY(VisibleAnywhere, Category = SaveData)
		bool bLevel1Complete;

	UPROPERTY(VisibleAnywhere, Category = SaveData)
		bool bLevel2Complete;

	UPROPERTY(VisibleAnywhere, Category = SaveData)
		bool bLevel3Complete;

	UPROPERTY(VisibleAnywhere, Category = SaveData)
		bool bLevel4Complete;
};
