// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "TutRerunSaveGame.h"
#include "ItemStorage.h"

// Sets default values
AMain::AMain()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	GetCapsuleComponent()->SetCapsuleSize(29.f,88.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	baseturnrate = 65.f;
	baselookuprate = 65.f;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.2;

	stamina = 65.f;
	health = 65.f;
	maxStamina = 150.f;
	maxHealth = 100.f;

	runningSpeed = 650.f;
	sprintingSpeed = 950.f;

	bShiftKeyDown = false;
	bEDown = false;
	bLMBDown = false;
	bESCDown = false;

	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaThreshold = 50.f;
	StaminaDrainRate = 25.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bHasCombatTarget = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();
	
	mainPlayerController = Cast<AMainPlayerController>(GetController());
	if (mainPlayerController)
	{
		mainPlayerController->GameModeOnly();
	}

	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	if (Map != "SunTemple" && Map != "Hub")
	{
		LoadGameHub();
		LoadGameNoSwitch();
	}

	if (Map == "Hub"){
		SaveGame();
		LoadGameHub();
	}
}


// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	float deltaStamina = StaminaDrainRate * DeltaTime;
	
	switch (StaminaStatus) {
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown && !bAttacking && !mainInAir && GetVelocity().Size() > 0.1f) {
			if ((stamina - deltaStamina) <= StaminaThreshold) {
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				stamina -= deltaStamina;
			}
			else {
				stamina -= deltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Sprinting);
		}
		else if (GetMovementComponent()->IsFalling()==false && !bAttacking) {
			if ((stamina + deltaStamina) >= maxStamina) {
				stamina = maxStamina;
			}
			else {
				stamina += deltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;


	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown && !bAttacking && !mainInAir && GetVelocity().Size()>0.1f) {
			if ((stamina - deltaStamina)<=0.f) {
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else {
				stamina -= deltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
		}
		else if (GetMovementComponent()->IsFalling() == false && !bAttacking) {
			if ((stamina + deltaStamina) >= StaminaThreshold) {
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				stamina += deltaStamina;
			}
			else {
				stamina += deltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;


	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown | GetMovementComponent()->IsFalling() | bAttacking) {
			stamina = 0;
		}
		else {
			stamina += deltaStamina;
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovery);
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;


	case EStaminaStatus::ESS_ExhaustedRecovery:
		if ((stamina + deltaStamina) >= StaminaThreshold && !bAttacking && mainInAir == false) {
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			stamina += deltaStamina;
		}
		else if (GetMovementComponent()->IsFalling() == false && !bAttacking) {
			stamina += deltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	default:
		;
	}

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator lookatyaw = getlookatrot(CombatTarget->GetActorLocation());
		FRotator interprot = FMath::RInterpTo(GetActorRotation(), lookatyaw, DeltaTime, InterpSpeed);
		SetActorRotation(interprot);
	}

	if (CombatTarget)
	{
		CombatTargetLoc = CombatTarget->GetActorLocation();

		if (mainPlayerController)
		{
			mainPlayerController->EnemyLoc = CombatTargetLoc;
		}
	}
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESC_down);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESC_up);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMain::E_down);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMain::E_up);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMain::LMB_down);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AMain::LMB_up);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::moveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::moveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

}

void AMain::moveForward(float Value) {
	if (canmove(Value)) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMain::moveRight(float Value) {
	if (canmove(Value)) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMain::TurnAtRate(float Rate){
	AddControllerYawInput(baseturnrate*Rate*GetWorld()->GetDeltaSeconds());
}

void AMain::LookUpAtRate(float Rate) {
	AddControllerPitchInput(baselookuprate * Rate * GetWorld()->GetDeltaSeconds());
}

void AMain::E_down() {
	bEDown = true;

	if (ActiveOverlapItem) {
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlapItem);
		if (Weapon) {
			Weapon->equip(this);
			ActiveOverlapItem = nullptr;
		}
	}
}

void AMain::E_up() {
	bEDown = false;
}



void AMain::die() {

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
	BP_UponDeath();
	SaveGame();
}

void AMain::deathEnd()
{
	GetMesh()->bPauseAnims = true;
}

void AMain::Jump()
{
	if (MovementStatus != EMovementStatus::EMS_Dead && !bAttacking)
	{
		if (mainPlayerController) if (mainPlayerController->bPauseMenuVisible) return;
		ACharacter::Jump();
	}
}

void AMain::SetMovementStatus(EMovementStatus status) {
	MovementStatus = status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting) {
		GetCharacterMovement()->MaxWalkSpeed = sprintingSpeed;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = runningSpeed;
	}
}

void AMain::ShiftKeyDown() {
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp() {
	bShiftKeyDown = false;
}

void AMain::ShowPickupLocations() {

	for (int32 i = 0; i < PickupLocs.Num(); i++) {
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocs[i], 25.f, 8, FLinearColor::Green, 10.f, 0.5f);
	}

}

void AMain::SetEquippedWeapon(AWeapon* weaponToSet) {
	
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = weaponToSet;
}

void AMain::LMB_down() {

	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	if (mainPlayerController) if (mainPlayerController->bPauseMenuVisible) return;
	if (EquippedWeapon && (!mainInAir)) {
		Attack();
	}

	if (!mainInAir) {
	}
}


void AMain::LMB_up() {
	bLMBDown = false;
}


void AMain::Attack() {
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {

		bAttacking = true;
		setInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage) {

			int8 Selection = FMath::RandRange(0, 1);
			switch (Selection) {
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 1.35f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 2.f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				break;
			default:
				;
			}
		}
		if (EquippedWeapon->swingSound)
		{
			//UGameplayStatics::PlaySound2D(this, EquippedWeapon->swingSound);
		}
	}	
}

void AMain::attackEnd() {
	bAttacking = false;
	setInterpToEnemy(false);
	if (bLMBDown == true) {
		Attack();
	}
}

void AMain::setMainInAir(bool value) {
	mainInAir = value;
}



void AMain::setInterpToEnemy(bool interp)
{
	bInterpToEnemy = interp;
}

FRotator AMain::getlookatrot(FVector target)
{
	FRotator lookatrot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), target);
	FRotator lookatrotyaw = FRotator(0.f, lookatrot.Yaw, 0.f);
	return lookatrotyaw;
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if ((health - DamageAmount) <= 0.f) {
		health -= DamageAmount;
		die();

		if (DamageCauser)
		{
			AEnemy* enemy = Cast<AEnemy>(DamageCauser);

			if (enemy)
			{
				enemy->bhasvalidtarget = false;
			}
		}
	}
	else {
		health -= DamageAmount;
	}

	return DamageAmount;
}

void AMain::decrementHealth(float x)
{
	if ((health - x) <= 0.f) {
		health -= x;
		die();
	}
	else {
		health -= x;
	}
}

void AMain::updateCombatTarget()
{
	TArray<AActor*> overlappingActors;
	GetOverlappingActors(overlappingActors, EnemyFilter);

	if (overlappingActors.Num() > 0)
	{
		float mindist;

		AEnemy* closestenemy = Cast<AEnemy>(overlappingActors[0]);

		if (closestenemy)
		{
			FVector Loc = GetActorLocation();
			mindist = (closestenemy->GetActorLocation() - Loc).Size();

			for (auto actor : overlappingActors)
			{
				if ((actor->GetActorLocation() - Loc).Size() < mindist)
				{
					mindist = (actor->GetActorLocation() - Loc).Size();
					AEnemy* newenemy = Cast<AEnemy>(actor);
					if (newenemy)
					{
						closestenemy = newenemy;
					}
				}
			}

			if (mainPlayerController)
			{
				mainPlayerController->displayEnemyHealthBar();
			}
			setCombatTarget(closestenemy);
			bHasCombatTarget = true;
		}
	}

	else
	{
		mainPlayerController->removeEnemyHealthBar();
		return;
	}
}

void AMain::incrementHealth(float Amount)
{
	if (health + Amount >= maxHealth)
	{
		health = maxHealth;
	}
	else
	{
		health += Amount;
	}
}

void AMain::switchLevel(FName levelName)
{
	UWorld* world = GetWorld();
	if (world)
	{
		FString CurrentLevel = world->GetMapName();

		FName currentLevelName = *CurrentLevel;

		if (currentLevelName != levelName)
		{
			UGameplayStatics::OpenLevel(world, levelName);

		}
	}
}

void AMain::SaveGame()
{
	UTutRerunSaveGame* SaveGameInst = Cast<UTutRerunSaveGame>(UGameplayStatics::CreateSaveGameObject(UTutRerunSaveGame::StaticClass()));

	SaveGameInst->CharacterStats.health = health;
	SaveGameInst->CharacterStats.maxHealth = maxHealth;
	SaveGameInst->CharacterStats.stamina = stamina;
	SaveGameInst->CharacterStats.location = GetActorLocation();
	SaveGameInst->CharacterStats.rotation = GetActorRotation();
	SaveGameInst->CharacterStats.blevel1comp = bcomplevel1;
	SaveGameInst->CharacterStats.blevel2comp = bcomplevel2;
	SaveGameInst->CharacterStats.blevel3comp = bcomplevel3;
	SaveGameInst->CharacterStats.blevel4comp = bcomplevel4;
	SaveGameInst->CharacterStats.level1stars = level1stars;
	SaveGameInst->CharacterStats.level2stars = level2stars;
	SaveGameInst->CharacterStats.level3stars = level3stars;
	SaveGameInst->CharacterStats.level4stars = level4stars;

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInst->CharacterStats.levelName = MapName;

	if (EquippedWeapon)
	{
		SaveGameInst->CharacterStats.weaponName = EquippedWeapon->Name;
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInst, SaveGameInst->PlayerName, SaveGameInst->UserIndex);
}

void AMain::LoadGame(bool setPos)
{
	UTutRerunSaveGame* LoadGameInst = Cast<UTutRerunSaveGame>(UGameplayStatics::CreateSaveGameObject(UTutRerunSaveGame::StaticClass()));

	LoadGameInst = Cast<UTutRerunSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInst->PlayerName, LoadGameInst->UserIndex));

	health = LoadGameInst->CharacterStats.health;
	maxHealth = LoadGameInst->CharacterStats.maxHealth;
	stamina = LoadGameInst->CharacterStats.stamina;

	level1stars = LoadGameInst->CharacterStats.level1stars;
	level2stars = LoadGameInst->CharacterStats.level2stars;
	level3stars = LoadGameInst->CharacterStats.level3stars;
	level4stars = LoadGameInst->CharacterStats.level4stars;

	if (weaponStorage) {
		AItemStorage* weapons = GetWorld()->SpawnActor<AItemStorage>(weaponStorage);

		if (weapons)
		{
			FString weaponName = LoadGameInst->CharacterStats.weaponName;

			if (weapons->weaponMap.Contains(weaponName)) {
				AWeapon* weapontoequip = GetWorld()->SpawnActor<AWeapon>(weapons->weaponMap[weaponName]);
				weapontoequip->equip(this);
			}
		}
	}


	if (setPos) {
		SetActorLocation(LoadGameInst->CharacterStats.location);
		SetActorRotation(LoadGameInst->CharacterStats.rotation);
	}
	

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	if (LoadGameInst->CharacterStats.levelName != TEXT(""))
	{
		FName levelname = *LoadGameInst->CharacterStats.levelName;
		switchLevel(levelname);
	}
}

void AMain::LoadGameHub()
{
	UTutRerunSaveGame* LoadGameInst = Cast<UTutRerunSaveGame>(UGameplayStatics::CreateSaveGameObject(UTutRerunSaveGame::StaticClass()));

	LoadGameInst = Cast<UTutRerunSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInst->PlayerName, LoadGameInst->UserIndex));

	health = maxHealth;
	stamina = maxStamina;

	bcomplevel1 = LoadGameInst->CharacterStats.blevel1comp;
	bcomplevel2 = LoadGameInst->CharacterStats.blevel2comp;
	bcomplevel3 = LoadGameInst->CharacterStats.blevel3comp;
	bcomplevel4 = LoadGameInst->CharacterStats.blevel4comp;

	level1stars = LoadGameInst->CharacterStats.level1stars;
	level2stars = LoadGameInst->CharacterStats.level2stars;
	level3stars = LoadGameInst->CharacterStats.level3stars;
	level4stars = LoadGameInst->CharacterStats.level4stars;

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

}

void AMain::ESC_down()
{
	bESCDown = true;

	if (mainPlayerController)
	{
		mainPlayerController->togglePauseMenu();
	}
}

void AMain::ESC_up()
{
	bESCDown = false;
}

bool AMain::canmove(float value)
{
	if (mainPlayerController)
	{
		return (Controller != nullptr) && (value != 0.f) && (!bAttacking) && (MovementStatus != EMovementStatus::EMS_Dead && !mainPlayerController->bPauseMenuVisible);
	}
	return false;
}

void AMain::Turn(float Value)
{
	if ((mainPlayerController != nullptr) && (Value != 0.f) && (MovementStatus != EMovementStatus::EMS_Dead && !mainPlayerController->bPauseMenuVisible))
	{
		AddControllerYawInput(Value);
	}
}

void AMain::LookUp(float Value)
{
	if ((mainPlayerController != nullptr) && (Value != 0.f) && (MovementStatus != EMovementStatus::EMS_Dead && !mainPlayerController->bPauseMenuVisible))
	{
		AddControllerPitchInput(Value);
	}
}

void AMain::LoadGameNoSwitch()
{
	UTutRerunSaveGame* LoadGameInst = Cast<UTutRerunSaveGame>(UGameplayStatics::CreateSaveGameObject(UTutRerunSaveGame::StaticClass()));

	LoadGameInst = Cast<UTutRerunSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInst->PlayerName, LoadGameInst->UserIndex));

	health = LoadGameInst->CharacterStats.health;
	maxHealth = LoadGameInst->CharacterStats.maxHealth;
	stamina = LoadGameInst->CharacterStats.stamina;

	if (weaponStorage) {
		AItemStorage* weapons = GetWorld()->SpawnActor<AItemStorage>(weaponStorage);

		if (weapons)
		{
			FString weaponName = LoadGameInst->CharacterStats.weaponName;

			if (weapons->weaponMap.Contains(weaponName)) {
				AWeapon* weapontoequip = GetWorld()->SpawnActor<AWeapon>(weapons->weaponMap[weaponName]);
				weapontoequip->equip(this);
			}
		}
	}



	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

void AMain::resetGame()
{
	UTutRerunSaveGame* SaveGameInst = Cast<UTutRerunSaveGame>(UGameplayStatics::CreateSaveGameObject(UTutRerunSaveGame::StaticClass()));


	SaveGameInst->CharacterStats.health = 100.f;
	SaveGameInst->CharacterStats.maxHealth = 100.f;
	SaveGameInst->CharacterStats.stamina = 150.f;
	SaveGameInst->CharacterStats.location = GetActorLocation();
	SaveGameInst->CharacterStats.rotation = GetActorRotation();
	SaveGameInst->CharacterStats.blevel1comp = false;
	SaveGameInst->CharacterStats.blevel2comp = false;
	SaveGameInst->CharacterStats.blevel3comp = false;
	SaveGameInst->CharacterStats.blevel4comp = false;


	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInst->CharacterStats.levelName = MapName;

	if (EquippedWeapon)
	{
		SaveGameInst->CharacterStats.weaponName = EquippedWeapon->Name;
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInst, SaveGameInst->PlayerName, SaveGameInst->UserIndex);

	LoadGameHub();
}

void AMain::BP_UponDeath_Implementation()
{
	switchLevel(FName("Hub"));
}

void AMain::returnToHub()
{
	killCount = 0;
	SaveGame();
	switchLevel(FName("Hub"));
	LoadGameHub();
}