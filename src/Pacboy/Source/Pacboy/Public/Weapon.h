// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectileBase.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
namespace EWeaponType
{
	enum Type
	{
		Melee,
		Pistol,
		Rifle,
		Sniper,
		RocketLauncher
	};
}

UENUM(BlueprintType)
namespace EWeaponShootingType
{
	enum Type
	{
		Instant,
		Projectile
	};
}

/**
*
*/
UCLASS()
class PACBOY_API AWeapon : public AActor
{
public:

	/** Initializes the weapon by copying the properties of another weapon */
	void Init(const AWeapon& Weapon);

	/** The type of the weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TEnumAsByte<EWeaponType::Type> WeaponType;

	/** The shooting type of the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TEnumAsByte<EWeaponShootingType::Type> ShootingType;

	/** Weapon damage. Used when shooting type is instant */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage;

	/** The ammo capacity of the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 AmmoCapacity;

	/** The ammo clip capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 ClipCapacity;

	/** The remaining ammo of the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 RemainingAmmo;

	/** The amount of ammo left in the clip of the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 AmmoInClip;

	/** The socket name of the gun's muzzle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName GunMuzzleSocketName;

	/** How many shots can this weapon fire per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	int32 ShotsPerSecond;

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AProjectileBase> ProjectileClass;

	/** The weapon mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	/** The weapon impact effect. Used for instant type weapons */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* WeaponImpactFX;

	/** The weapon shot effect */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* WeaponShotFX;

	/** The weapon shot sound effect */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	USoundBase* WeaponShotSFX;

	AWeapon(const FObjectInitializer& ObjectInitializer);

	/** Reloads the weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

private:

	GENERATED_BODY()

};
