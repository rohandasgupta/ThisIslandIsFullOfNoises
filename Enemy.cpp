// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/AnimInstance.h"
#include "MainPlayerController.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Aggrosphere = CreateDefaultSubobject<USphereComponent>(TEXT("Aggrosphere"));
	Aggrosphere->SetupAttachment(GetRootComponent());
	Aggrosphere->InitSphereRadius(600.f);
	
	combatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Combat Sphere"));
	combatSphere->SetupAttachment(GetRootComponent());
	combatSphere->InitSphereRadius(75.f);

	bOverlappingCombatSphere = false;

	health = 75.f;
	maxHealth = 100.f;
	damage = 5.f;

	EnemyCombatCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Enemy Combat Collision"));
	EnemyCombatCollision->SetupAttachment(GetMesh(), FName("EnemyAttach"));

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	bhasvalidtarget = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	AIController = Cast<AAIController>(GetController());

	Aggrosphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AggrosphereOnOverlapBegin);
	Aggrosphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AggrosphereOnOverlapEnd);
	combatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::combatSphereOnOverlapBegin);
	combatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::combatSphereOnOverlapEnd);


	EnemyCombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	EnemyCombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);
	EnemyCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyCombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	EnemyCombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EnemyCombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (tickCheck > 120) {
		if (!bOverlappingCombatSphere && !bAttacking) {
			if (combatTarget){
			moveToTarget(combatTarget);
			}
			tickCheck = 0;
		}
		if (bOverlappingCombatSphere)
		{
			if (combatTarget) {
				FRotator lookatrot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), combatTarget->GetActorLocation());
				FRotator yawrot = FRotator(0.f, lookatrot.Yaw, 0.f);
				FRotator interprot = FMath::RInterpTo(GetActorRotation(), yawrot, DeltaTime, 5.f);
				SetActorRotation(interprot);
			}
		}
	}
	else
	{
		tickCheck++;
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::AggrosphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult){
	if (OtherActor && alive()) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);
			moveToTarget(Main);
			combatTarget = Main;
		}
	}
}

void AEnemy::AggrosphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {

			bhasvalidtarget = false;
			if (Main->CombatTarget == this) {
				Main->setCombatTarget(nullptr);
			}

			Main->setHasCombatTarget(false);
			
			Main->updateCombatTarget();
			

			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController) {
				AIController->StopMovement();
			}
			combatTarget = nullptr;
		}
	}
}

void AEnemy::combatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor && alive()) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {
			bhasvalidtarget = true;
			Main->setCombatTarget(this);
			Main->setHasCombatTarget(true);
			Main->updateCombatTarget();

			combatTarget = Main;
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
			bOverlappingCombatSphere = true;
			attack();
		}
	}
}

void AEnemy::combatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor && OtherComp) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {
			bOverlappingCombatSphere = false;
			if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking) {
				moveToTarget(Main);

				if (Main->CombatTarget == this) {
					Main->setCombatTarget(nullptr);
					Main->bHasCombatTarget = false;
					Main->updateCombatTarget();
				}
				if (Main->mainPlayerController) {
					USkeletalMeshComponent* mainmesh = Cast<USkeletalMeshComponent>(OtherComp);
					if (mainmesh)
					{
						Main->mainPlayerController->removeEnemyHealthBar();
					}
				}
			}
		}
	}
}

void AEnemy::moveToTarget(AMain* target) {
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController) {
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(target);
		MoveRequest.SetAcceptanceRadius(0.0f);

		
		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);
		/*
		TArray<FNavPathPoint> pathPoints = NavPath->GetPathPoints();

		
		for (auto Point : pathPoints) {
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Green, 10.f, 0.5f);
		}
		*/
	}
}

void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			if (Main->HitParticles)
			{

				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (TipSocket) {
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), false);
				}

			}
			if (Main->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}

			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(Main, damage, AIController, this, DamageTypeClass);
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

void AEnemy::activateCollision()
{
	EnemyCombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if (enemySwingSound)
	{
		UGameplayStatics::PlaySound2D(this, enemySwingSound);
	}
}

void AEnemy::deactivateCollision()
{
	EnemyCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::attack()
{	
	if (alive() && bhasvalidtarget) {
		if (AIController)
		{
			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
		if (!bAttacking)
		{
			bAttacking = true;
			UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
			if (animInstance)
			{
				animInstance->Montage_Play(CombatMontage, 1.35f);
				animInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
			}
		}
	}
}

void AEnemy::attackEnd()
{
	bAttacking = false;
	if (bOverlappingCombatSphere)
	{
		attack();
	}
	else
	{
		;
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (health - DamageAmount <= 0.f)
	{
		health -= DamageAmount;
		die(DamageCauser);
	}
	else
	{
		health -= DamageAmount;
	}
	return DamageAmount;
}

void AEnemy::die(AActor* causer)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance)
	{
		animInstance->Montage_Play(CombatMontage, 1.35f);
		animInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

	EnemyCombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Aggrosphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	combatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AMain* main = Cast<AMain>(causer);
	if (main)
	{
		main->updateCombatTarget();
		main->killCount = main->killCount + 1;
	}
}

void AEnemy::deathEnd()
{
	GetMesh()->bPauseAnims = true;
	//GetMesh()->bNoSkeletonUpdate = true;

}

bool AEnemy::alive()
{
	return (GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead);
}

void AEnemy::disappear()
{
	Destroy();
}