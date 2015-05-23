// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

/**
 * 
 */
UCLASS()
class PACBOY_API AProjectileBase : public AActor
{
public:

	/** The damage that the projectile deals on impact */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float Damage;

	/** The impulse force of the projectile on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float ImpulseForce;

	/** Projectile collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComponent;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;

	/** The controller of the player that spawned the projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	AController* Shooter;

	/** The projectile mesh */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;

	AProjectileBase(const FObjectInitializer& ObjectInitializer);

	/** Called when the projectile hits something (to apply effects) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnImpact(AActor* OtherActor, UPrimitiveComponent* OtherComp);

protected:

	/** Called when the projectile hits something (to apply damage) */
	UFUNCTION()
	virtual void OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:

	GENERATED_BODY()

};
