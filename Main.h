// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),

	EMS_Sprinting UMETA(DisplayName = "Sprinting"),

	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	ESS_BelowMinimum UMETA(DisplayName = "Below Minimum"),
	ESS_ExhaustedRecovery UMETA(DisplayName = "Exhausted Recovery"),

	ESS_MAX UMETA(DisplayName = "DefaultMax")

};

UCLASS()
class TUTRERUN_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	TArray<FVector> PickupLocs;

	UPROPERTY(EditDefaultsOnly, Category = SaveData)
	TSubclassOf<class AItemStorage> weaponStorage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat)
		bool bHasCombatTarget;

	FORCEINLINE void setHasCombatTarget(bool b) { bHasCombatTarget = b; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat)
		FVector CombatTargetLoc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Controller)
		class AMainPlayerController* mainPlayerController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items)
		class AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items)
		class AWeapon* NullWeapon = nullptr;

	void SetEquippedWeapon(AWeapon* weaponToSet);
	FORCEINLINE AWeapon* GetEquippedWeapon() { return EquippedWeapon; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
		class AItem* ActiveOverlapItem;

	FORCEINLINE void SetOverlapItem(AItem* weaponToSet) { ActiveOverlapItem = weaponToSet; }

	UFUNCTION(BlueprintCallable)
		void ShowPickupLocations();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Enums)
		EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Enums)
		EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus status) { StaminaStatus = status; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float StaminaThreshold;

	void SetMovementStatus(EMovementStatus status);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Running)
		float runningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Running)
		float sprintingSpeed;

	bool bShiftKeyDown;

	void ShiftKeyDown();

	void ShiftKeyUp();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float baseturnrate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float baselookuprate;

	/*

		Player Stats

	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
		float health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = PlayerStats)
		float maxHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = PlayerStats)
		float maxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
		float stamina;

	void decrementHealth(float x);

	UFUNCTION(BlueprintCallable)
		void incrementHealth(float Amount);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCaused) override;

	void die();

	float InterpSpeed;

	bool bInterpToEnemy;

	void setInterpToEnemy(bool interp);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
		class AEnemy* CombatTarget;

	FORCEINLINE void setCombatTarget(AEnemy* target) { CombatTarget = target; }

	FRotator getlookatrot(FVector target);

	virtual void Jump() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void moveForward(float Value);

	void moveRight(float Value);

	void Turn(float Value);

	void LookUp(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	bool bEDown;

	void E_down();
	void E_up();

	void LMB_down();
	void LMB_up();

	void ESC_down();
	void ESC_up();

	bool bESCDown;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Anims)
		bool bAttacking;

	void Attack();

	UFUNCTION(BlueprintCallable)
		void attackEnd();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Anims)
		class UAnimMontage* CombatMontage;

	bool mainInAir;

	bool bLMBDown;

	void setMainInAir(bool value);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
		class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundCue* HitSound;

	UFUNCTION(BlueprintCallable)
		void deathEnd();

	void updateCombatTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
		TSubclassOf<AEnemy> EnemyFilter;

	void switchLevel(FName levelName);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool setPos);

	UFUNCTION(BlueprintCallable)
	void LoadGameNoSwitch();

	UFUNCTION(BlueprintCallable)
	void LoadGameHub();

	UFUNCTION(BlueprintCallable)
	void resetGame();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int cppcoin;

	UFUNCTION(BlueprintImplementableEvent)
	void cppcoinsave();

	UFUNCTION(BlueprintImplementableEvent)
	void cppcoinload();

	bool canmove(float value);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Save)
		bool bcomplevel1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Save)
		bool bcomplevel2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Save)
		bool bcomplevel3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Save)
		bool bcomplevel4;

	UFUNCTION(BlueprintNativeEvent)
	void BP_UponDeath();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int level1stars;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int level2stars;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int level3stars;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int level4stars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int killCount;

	UFUNCTION(BlueprintCallable)
	void returnToHub();

	bool bHasSavedGame;
};