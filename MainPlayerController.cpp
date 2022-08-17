// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"


void AMainPlayerController::BeginPlay() {
	Super::BeginPlay();

	if (HUDOverlayAsset) {
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}
	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);

	if (WEnemyHealthBar)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		
		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}

	}

	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);

		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D alignment(0.f);
		PauseMenu->SetAlignmentInViewport(alignment);

	}

}

void AMainPlayerController::displayEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::removeEnemyHealthBar()
{
	bEnemyHealthBarVisible = false;
	EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyHealthBar)
	{
		FVector2D posinview;

		ProjectWorldLocationToScreen(EnemyLoc, posinview);

		FVector2D sizeinview(300.f, 25.f);

		posinview.Y -= 100.f;
		EnemyHealthBar->SetPositionInViewport(posinview);
		EnemyHealthBar->SetDesiredSizeInViewport(sizeinview);
	}
}

void AMainPlayerController::displayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI mouseOnly;

		SetInputMode(mouseOnly);
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::removePauseMenu_Implementation()
{
	if (PauseMenu) {

		FInputModeGameOnly gameOnly;

		SetInputMode(gameOnly);
		bShowMouseCursor = false;

		bPauseMenuVisible = false;


	}
}

void AMainPlayerController::togglePauseMenu()
{	
	if (bPauseMenuVisible)
	{
		removePauseMenu();
	}
	else
	{
		displayPauseMenu();
	}
}

void AMainPlayerController::GameModeOnly()
{
	FInputModeGameOnly gameOnly;

	SetInputMode(gameOnly);
}