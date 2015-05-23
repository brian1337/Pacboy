// Fill out your copyright notice in the Description page of Project Settings.

#include "Pacboy.h"
#include "Weapon.h"

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->WeaponMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, FName(TEXT("WeaponMesh")));

	// Note: The static mesh references on the WeaponMesh component
	// are set in the derived blueprint classes (to avoid direct content references in C++)
}

void AWeapon::Init(const AWeapon& Weapon)
{
	this->WeaponType = Weapon.WeaponType;
	this->ShootingType = Weapon.ShootingType;
	this->Damage = Weapon.Damage;
	this->AmmoCapacity = Weapon.AmmoCapacity;
	this->ClipCapacity = Weapon.ClipCapacity;
	this->RemainingAmmo = Weapon.RemainingAmmo;
	this->AmmoInClip = Weapon.AmmoInClip;
	this->GunMuzzleSocketName = Weapon.GunMuzzleSocketName;
	this->ShotsPerSecond = Weapon.ShotsPerSecond;
	this->ProjectileClass = Weapon.ProjectileClass;
	this->WeaponImpactFX = Weapon.WeaponImpactFX;
}

void AWeapon::Reload()
{
	if ((this->AmmoInClip < this->ClipCapacity) && (this->RemainingAmmo > 0))
	{
		int32 AmmoToReload = this->ClipCapacity - this->AmmoInClip;

		if (this->RemainingAmmo > AmmoToReload)
		{
			this->AmmoInClip += AmmoToReload;
			this->RemainingAmmo -= AmmoToReload;
		}
		else
		{
			this->AmmoInClip += this->RemainingAmmo;
			this->RemainingAmmo = 0;
		}
	}
}
