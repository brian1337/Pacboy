// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DamageableObject.h"
#include "GameFramework/Character.h"
#include "Weapon.h"
#include "MainPlayerController.h"
#include "CharacterBase.generated.h"

/**
*
*/
UCLASS()
class PACBOY_API ACharacterBase : public ACharacter, public IDamageableObject
{
public:

	/** The speed of the character while jogging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float JogSpeed;

	/** The speed of the character while sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float SprintSpeed;

	/** The speed of the character while aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float AimSpeed;

	/** The Current health that the character has left */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Character")
	float Health;

	/** The maximum health of the character that he can have */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character")
	float HealthCapacity;

	/** The current energy that the character has left */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Character")
	float Energy;

	/** The maximum energy that the character can have */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character")
	float EnergyCapacity;

	/** Indicates if the character is sprinting */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Character")
	bool bIsSprinting;

	/** Indicates if the character is aiming */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Character")
	bool bIsAiming;

	/** Indicates if the character is firing */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Character")
	bool bIsFiring;

	/** Indicates if the character is reloading */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Character")
	bool bIsReloading;

	/** Indicates if the character is dead */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Character")
	bool bIsDead;

	UPROPERTY(Replicated)
	bool bReloadClient;

	/** Character dash force */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float DashForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float SprintEnergy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float DashEnergy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float WallJumpEnergy;

	/** Energy regeneration (once every 0.1 secs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	float EnergyRegen;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character Movement")
	int32 JumpCount;

	/** Maximum wall jumps. Set to -1 for unlimited wall jumps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement")
	int32 MaxWallJumps;

	/** The class used for the character's rifle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeapon> RifleClass;

	/** The character's rifle */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	AWeapon* Rifle;

	/** The class used for the character's rocket launcher */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeapon> RocketLauncherClass;

	/** The character's rocket launcher */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	AWeapon* RocketLauncher;

	/** A reference to the current equipped weapon */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	AWeapon* EquippedWeapon;

	/** The socket name of the weapon slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponSocketName;

	/** The hit effect when character is hit */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* HitFX;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	UAnimMontage* ReloadAnim;

	UPROPERTY(Replicated)
	float ReloadAnimTimeRemaining;

	UPROPERTY(Replicated)
	bool FirstShot;

	UPROPERTY(Replicated)
	bool DelayShot;

	UPROPERTY(Replicated)
	bool ShootingGateOpen;

	UPROPERTY(Replicated)
	bool FireFromClient;

	/** Used for aim offset */
	UPROPERTY(Replicated)
	float CharPitch;

	UFUNCTION(Server, WithValidation, Reliable)
	void SetCharPitch_Server();

	UFUNCTION(Server, WithValidation, Reliable)
	void RestartReloadAnimTimeRemaining();

	ACharacterBase(const FObjectInitializer& ObjectInitializer);

	/**
	* The character takes damage
	* @param Damage - How much damage the character gets
	* @param Hit - Hit information
	* @param EventInstigator - The Controller responsible for the damage
	*/
	UFUNCTION(BlueprintCallable, Category = "Character Action")
	virtual void TakeDamage(float Damage, const FHitResult& Hit, AController* EventInstigator) override;

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void TakeDamage_Server(float Damage, const FHitResult& Hit, AController* EventInstigator);

	UFUNCTION(NetMulticast, Reliable)
	virtual void TakeDamageFX_Multicast(FVector ImpactPoint);

	UFUNCTION(Client, Reliable)
	virtual void TakeDamage_Client();

	/** Called when character receives any damage (from ingame events) */
	virtual void ReceiveAnyDamage(float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, WithValidation, Reliable, Category = "Weapon Action")
	void OnFireEvent_Multicast(FVector Location);

	UFUNCTION(Server, WithValidation, Reliable)
	void OnReloadStart_Server();

	UFUNCTION(NetMulticast, Reliable)
	void OnReloadStart_Multicast();

	virtual void Jump() override;

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void Jump_Server();

	struct DetectWallResult
	{
		bool HitWall;
		bool HitSide;
		bool RightSideHit;
	};
	virtual DetectWallResult DetectWall();

	virtual void Landed(const FHitResult& Hit) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	/** Used for moving forward and backward */
	virtual void MoveForward(float AxisValue);

	/** Used for moving right and left */
	virtual void MoveRight(float AxisValue);

	virtual void Move(float AxisValue, EAxis::Type Axis);

	virtual void LeftDash();

	virtual void RightDash();

	virtual void Dash(float Force);

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void Dash_Server(float Force);

	virtual void SprintStart();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void SprintStart_Server();

	virtual void SprintStop();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void SprintStop_Server();

	virtual void AimStart();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void AimStart_Server();

	virtual void AimStop();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void AimStop_Server();

	virtual void FireStart_Key();

	virtual void FireStart(bool FromClient);

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void FireStart_Server(bool FromClient);

	virtual void FireStop();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void FireStop_Server();

	virtual void OnFire();

	/** Spawns projectile */
	UFUNCTION(Server, WithValidation, Reliable)
	virtual void OnFire_Server(FVector SpawnLocation, FRotator SpawnRotation, AController* Shooter);

	UFUNCTION(Client, Reliable)
	virtual void OnFire_Client();

	UFUNCTION(Client, Reliable)
	virtual void Reload_Client();

	virtual void ReloadStart();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void ReloadStart_Server();

	virtual void ReloadStop();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void ReloadStop_Server();

	/** Reloads the clip of the character */
	UFUNCTION(BlueprintCallable, Category = "Character Action")
	virtual void Reload();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void Reload_Server();

	UFUNCTION(Client, Reliable)
	virtual void Reload_OnClient();

	UFUNCTION(Client, Reliable)
	virtual void Respawn_Player_Client();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void Respawn_Player_Server();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Despawn_Actor();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void FellOutOfWorld_Server(const class UDamageType* dmgType);

	UFUNCTION(Client, Reliable)
	virtual void FellOutOfWorld_StopEnergy();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Destroy_Body();

	virtual void SwapToRifle();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void SwapToRifle_Server();

	UFUNCTION(NetMulticast, WithValidation, Reliable)
	virtual void SwapToRifle_Multicast();

	virtual void SwapToRocketLauncher();

	UFUNCTION(Server, WithValidation, Reliable)
	virtual void SwapToRocketLauncher_Server();

	UFUNCTION(NetMulticast, WithValidation, Reliable)
	virtual void SwapToRocketLauncher_Multicast();

	/** Return false if insufficient energy */
	bool UseEnergy(float EnergyValue);

	void UpdateEnergy();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		FRotator GetAimOffsets() const;

protected:

	USkeletalMeshComponent* WeaponMesh;

private:

	GENERATED_BODY()

};