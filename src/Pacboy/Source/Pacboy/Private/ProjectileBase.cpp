// Fill out your copyright notice in the Description page of Project Settings.

#include "Pacboy.h"
#include "ProjectileBase.h"
#include "DamageableObject.h"

AProjectileBase::AProjectileBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Use a sphere as a simple collision representation
	this->CollisionComponent = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, FName(TEXT("CollisionComponent")));
	this->CollisionComponent->InitSphereRadius(2.5f);
	this->CollisionComponent->bCanEverAffectNavigation = false;
	this->CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	//this->CollisionComponent->BodyInstance.SetCollisionProfileName("Projectile"); // Collision profiles are defined in DefaultEngine.ini

	this->RootComponent = this->CollisionComponent;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	this->ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, FName(TEXT("ProjectileMovementComponent")));
	this->ProjectileMovement->UpdatedComponent = this->CollisionComponent;
	this->ProjectileMovement->InitialSpeed = 7000.f;
	this->ProjectileMovement->MaxSpeed = 7000.f;
	this->ProjectileMovement->ProjectileGravityScale = 0.f;
	this->ProjectileMovement->bRotationFollowsVelocity = true;
	this->ProjectileMovement->bShouldBounce = false;

	// Destroy after 3 seconds by default
	this->InitialLifeSpan = 3.f;

	this->Damage = 0.f;
	this->ImpulseForce = 100.f;

	this->ProjectileMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, FName(TEXT("ProjectileMesh")));
	this->ProjectileMesh->AttachTo(this->RootComponent);

	// Note: The static mesh references on the ProjectileMesh component
	// are set in the derived blueprint classes (to avoid direct content references in C++)
}

void AProjectileBase::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != NULL) && (OtherActor != this))
	{
		AProjectileBase* OtherProj = Cast<AProjectileBase>(OtherActor);

		if (OtherProj != NULL)
		{
			return;
		}

		if ((OtherComp != NULL) && (OtherComp->IsSimulatingPhysics()))
		{
			OtherComp->AddImpulseAtLocation(this->GetVelocity() * this->ImpulseForce, this->GetActorLocation());
		}

		IDamageableObject* DamageableObject = Cast<IDamageableObject>(OtherActor);
		if (DamageableObject != NULL)
		{
			APawn* Pawn = Cast<APawn>(OtherActor);
			if (Pawn != NULL)
			{
				if (Pawn->GetController() == this->Shooter)
				{
					return;
				}
			}

			DamageableObject->TakeDamage(this->Damage, Hit, this->Shooter);
		}
	}

	this->OnImpact(OtherActor, OtherComp);
	this->Destroy();
}
