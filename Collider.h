// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Collider.generated.h"

UCLASS()
class TUTRERUN_API ACollider : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollider();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	class UStaticMeshComponent* meshComponent;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	class USphereComponent* sphereComponent;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UColliderMovementComponent* OurMovementComponent;
	
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	FORCEINLINE UStaticMeshComponent* GetMeshComponent() { return meshComponent; }
	FORCEINLINE void setMeshComponent(UStaticMeshComponent* Mesh) { meshComponent = Mesh; }

	FORCEINLINE USphereComponent* GetSphereComponent() { return sphereComponent; }
	FORCEINLINE void setSphereComponent(USphereComponent* Mesh) { sphereComponent = Mesh; }

	FORCEINLINE UCameraComponent* GetCameraComponent() { return Camera; }
	FORCEINLINE void setCameraComponent(UCameraComponent* CameraIn) { Camera = CameraIn; }

	FORCEINLINE USpringArmComponent* GetSpringArmComponent() { return SpringArm; }
	FORCEINLINE void setSpringArmComponent(USpringArmComponent* SpringArmIn) { SpringArm = SpringArmIn; }

private:

	void MoveForward(float input);
	void MoveRight(float input);

	void pitchCamera(float axisValue);
	void yawCamera(float axisValue);

	FVector2D CameraInput;
};
