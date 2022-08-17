// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Main.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy.h"
#include "Components/CapsuleComponent.h"



AExplosive::AExplosive(){
	Damage = 15.f;
}

void AExplosive::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (OtherActor) {
		AMain* main = Cast<AMain>(OtherActor);
		AEnemy* enemy = Cast<AEnemy>(OtherActor);

		if (main || enemy) {

			UCapsuleComponent* caps = Cast<UCapsuleComponent>(OtherComp);
			if (caps) {

				if (OverlapParticles) {
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);
				}
				if (sound) {
					UGameplayStatics::PlaySound2D(this, sound);
				}

				UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageTypeClass);

				//	main->PickupLocs.Add(GetActorLocation());

				Destroy();
			}
		}
	}
}

void AExplosive::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}