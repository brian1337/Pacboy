// Fill out your copyright notice in the Description page of Project Settings.

#include "Pacboy.h"
#include "CharacterBase.h"

#include "UnrealNetwork.h"

ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->GetCapsuleComponent()->InitCapsuleSize(55.f, 88.f);

	// Don't rotate the character when the controller is rotated (this will only affect the camera)
	this->bUseControllerRotationPitch = false;
	this->bUseControllerRotationRoll = false;
	this->bUseControllerRotationYaw = true;

	// Configure default character and movement
	this->JogSpeed = 600.f;
	this->SprintSpeed = 1200.f;
	this->AimSpeed = 300.f;
	this->GetCharacterMovement()->MaxWalkSpeed = this->JogSpeed;
	this->GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	this->GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ... at this rotation rate
	this->GetCharacterMovement()->JumpZVelocity = 450.f;
	this->GetCharacterMovement()->AirControl = 0.5f;

	// Set up gameplay features
	this->HealthCapacity = 100;
	this->EnergyCapacity = 100;

	this->DashForce = 1500.f;

	this->SprintEnergy = 1.5f;
	this->DashEnergy = 10.f;
	this->WallJumpEnergy = 15.f;

	this->EnergyRegen = 0.66f;

	this->MaxWallJumps = -1;

	this->FirstShot = true;

	// Note: The skeletal mesh and animation blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named BP_MainCharacter (to avoid direct content references in C++)
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	this->Health = this->HealthCapacity;
	this->Energy = this->EnergyCapacity;

	this->GetCharacterMovement()->MaxWalkSpeed = this->JogSpeed;

	if (this->RifleClass != NULL)
	{
		AWeapon* RifleWeapon = Cast<AWeapon>(this->RifleClass->GetDefaultObject());

		if (RifleWeapon != NULL)
		{
			this->Rifle = this->GetWorld()->SpawnActor<AWeapon>(this->RifleClass);
			this->Rifle->Init(*RifleWeapon);

			this->Rifle->WeaponMesh->AttachTo(this->GetMesh(), this->WeaponSocketName, EAttachLocation::SnapToTarget, true);

			this->EquippedWeapon = this->Rifle;
		}
	}

	if (this->RocketLauncherClass != NULL)
	{
		AWeapon* RocketLauncherWeapon = Cast<AWeapon>(this->RocketLauncherClass->GetDefaultObject());

		if (RocketLauncherWeapon != NULL)
		{
			this->RocketLauncher = this->GetWorld()->SpawnActor<AWeapon>(this->RocketLauncherClass);
			this->RocketLauncher->Init(*RocketLauncherWeapon);

			this->RocketLauncher->WeaponMesh->AttachTo(this->GetMesh(), this->WeaponSocketName, EAttachLocation::SnapToTarget, true);

			this->RocketLauncher->SetActorHiddenInGame(true);
		}
	}

	this->GetWorldTimerManager().SetTimer(this, &ACharacterBase::UpdateEnergy, 0.1f, true);
}

void ACharacterBase::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharacterBase, Health);
	DOREPLIFETIME(ACharacterBase, Energy);
	DOREPLIFETIME(ACharacterBase, bIsSprinting);
	DOREPLIFETIME(ACharacterBase, bIsReloading);
	DOREPLIFETIME(ACharacterBase, bIsAiming);
	DOREPLIFETIME(ACharacterBase, bIsFiring);
	DOREPLIFETIME(ACharacterBase, bIsDead);
	DOREPLIFETIME(ACharacterBase, ReloadAnimTimeRemaining);
	DOREPLIFETIME(ACharacterBase, CharPitch);
	DOREPLIFETIME(ACharacterBase, FirstShot);
	DOREPLIFETIME(ACharacterBase, DelayShot);
	DOREPLIFETIME(ACharacterBase, ShootingGateOpen);
	DOREPLIFETIME(ACharacterBase, FireFromClient);
}

void ACharacterBase::SwapToRifle()
{
	if (Role < ROLE_Authority)
	{
		this->SwapToRifle_Server();
	}
	else
	{
		this->SwapToRifle_Multicast();
	}
}

bool ACharacterBase::SwapToRifle_Server_Validate()
{
	return true;
}

void ACharacterBase::SwapToRifle_Server_Implementation()
{
	this->SwapToRifle_Multicast();
}

bool ACharacterBase::SwapToRifle_Multicast_Validate()
{
	return true;
}

void ACharacterBase::SwapToRifle_Multicast_Implementation()
{
	this->EquippedWeapon->SetActorHiddenInGame(true);
	this->EquippedWeapon = this->Rifle;
	this->EquippedWeapon->SetActorHiddenInGame(false);
	this->FirstShot = true;
}

void ACharacterBase::SwapToRocketLauncher()
{
	if (Role < ROLE_Authority)
	{
		this->SwapToRocketLauncher_Server();
	}
	else
	{
		this->SwapToRocketLauncher_Multicast();
	}
}

bool ACharacterBase::SwapToRocketLauncher_Server_Validate()
{
	return true;
}

void ACharacterBase::SwapToRocketLauncher_Server_Implementation()
{
	this->SwapToRocketLauncher_Multicast();
}

bool ACharacterBase::SwapToRocketLauncher_Multicast_Validate()
{
	return true;
}

void ACharacterBase::SwapToRocketLauncher_Multicast_Implementation()
{
	this->EquippedWeapon->SetActorHiddenInGame(true);
	this->EquippedWeapon = this->RocketLauncher;
	this->EquippedWeapon->SetActorHiddenInGame(false);
	this->FirstShot = true;
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsReloading)
	{
		this->ReloadAnimTimeRemaining -= DeltaTime;

		if (ReloadAnimTimeRemaining <= 0.f)
		{
			this->RestartReloadAnimTimeRemaining();
			this->StopAnimMontage(this->ReloadAnim);
			this->Reload();
		}
	}
}

bool ACharacterBase::RestartReloadAnimTimeRemaining_Validate()
{
	return true;
}

void ACharacterBase::RestartReloadAnimTimeRemaining_Implementation()
{
	this->ReloadAnimTimeRemaining = 0.f;
}

void ACharacterBase::Jump()
{
	if (this->bIsDead)
	{
		return;
	}

	if ((this->MaxWallJumps != -1) && (this->JumpCount > this->MaxWallJumps))
	{
		return;
	}

	if (Role < ROLE_Authority)
	{
		this->Jump_Server();
	}

	if (!this->GetCharacterMovement()->IsFalling())
	{
		Super::Jump();
		this->JumpCount++;
	}
	else
	{
		DetectWallResult DetectWallResult = this->DetectWall();
		//GEngine->AddOnScreenDebugMessage(0, 10.f, FColor::Cyan, "banana");

		if (!DetectWallResult.HitWall)
		{
			//GEngine->AddOnScreenDebugMessage(0, 10.f, FColor::Cyan, "fail-banana");
			return;
		}

		if (!this->UseEnergy(this->WallJumpEnergy))
		{
			return;
		}

		const FVector UpLaunchVelocity = this->GetActorUpVector() * 1050;
		FVector FullLaunchVelocity;

		if (DetectWallResult.HitSide)
		{
			if (DetectWallResult.RightSideHit)
			{
				FullLaunchVelocity = (this->GetActorRightVector() * -1000) + UpLaunchVelocity;
			}
			else
			{
				FullLaunchVelocity = (this->GetActorRightVector() * 1000) + UpLaunchVelocity;
			}
		}
		else
		{
			FullLaunchVelocity = (this->GetActorForwardVector() * -1000) + UpLaunchVelocity;
		}
		this->LaunchCharacter(FullLaunchVelocity, true, true);

		this->JumpCount++;
	}
}

bool ACharacterBase::Jump_Server_Validate()
{
	return true;
}

void ACharacterBase::Jump_Server_Implementation()
{
	this->Jump();
}

void ACharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	this->JumpCount = 0;

	if (this->bIsDead)
	{
		this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

ACharacterBase::DetectWallResult ACharacterBase::DetectWall()
{
	DetectWallResult Result{ false, false, false };

	FName TraceTag = FName(TEXT("WallTrace"));
	//GetWorld()->DebugDrawTraceTag = TraceTag;

	const FVector TraceStart = this->GetActorLocation();
	const FCollisionQueryParams QueryParams = FCollisionQueryParams(TraceTag, true, this);
	const FCollisionObjectQueryParams ObjectQueryParams = FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic));

	FHitResult SphereHitResult(ForceInit);
	const FVector SphereTraceEnd = (this->GetActorUpVector() * 100) + TraceStart;

	bool WallFound = this->GetWorld()->SweepSingle(SphereHitResult, TraceStart, SphereTraceEnd, FQuat(), FCollisionShape::MakeSphere(60.f), QueryParams, ObjectQueryParams);

	if (!WallFound)
	{
		return Result;
	}
	Result.HitWall = true;

	// right side
	FHitResult RightSideHitResult(ForceInit);
	const FVector RightSideTraceEnd = (this->GetActorRightVector() * 100) + TraceStart;

	bool RightSideHit = this->GetWorld()->LineTraceSingle(RightSideHitResult, TraceStart, RightSideTraceEnd, QueryParams, ObjectQueryParams);

	if (RightSideHit)
	{
		Result.HitSide = true;
		Result.RightSideHit = true;

		return Result;
	}

	// left side
	FHitResult LeftSideHitResult(ForceInit);
	const FVector LeftSideTraceEnd = (this->GetActorRightVector() * -100) + TraceStart;

	bool LeftSideHit = this->GetWorld()->LineTraceSingle(LeftSideHitResult, TraceStart, LeftSideTraceEnd, QueryParams, ObjectQueryParams);

	if (LeftSideHit)
	{
		Result.HitSide = true;
		Result.RightSideHit = false;

		return Result;
	}

	return Result;
}

void ACharacterBase::MoveForward(float AxisValue)
{
	this->Move(AxisValue, EAxis::X);
}

void ACharacterBase::MoveRight(float AxisValue)
{
	this->Move(AxisValue, EAxis::Y);
}

void ACharacterBase::Move(float AxisValue, EAxis::Type Axis)
{
	if (this->bIsDead)
	{
		return;
	}

	if (this->GetController() != NULL && AxisValue != 0.f)
	{
		// Find out which way is forward
		const FRotator Rotation = this->GetController()->GetControlRotation();
		const FRotator YawRotation = FRotator(0.f, Rotation.Yaw, 0.f);

		// Get forward or right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);

		this->AddMovementInput(Direction, AxisValue);
	}
}

void ACharacterBase::LeftDash()
{
	this->Dash(-this->DashForce);
}

void ACharacterBase::RightDash()
{
	this->Dash(this->DashForce);
}

void ACharacterBase::Dash(float Force)
{
	if (this->bIsDead)
	{
		return;
	}

	if (!this->UseEnergy(10.f))
	{
		return;
	}

	if (Role < ROLE_Authority)
	{
		this->Dash_Server(Force);
	}

	// Find out which way is forward
	const FRotator Rotation = this->GetController()->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector LaunchVelocity = YawRotation.RotateVector(FVector(0.f, Force, 50.f));

	this->LaunchCharacter(LaunchVelocity, true, true);
}

bool ACharacterBase::Dash_Server_Validate(float Force)
{
	return true;
}

void ACharacterBase::Dash_Server_Implementation(float Force)
{
	this->Dash(Force);
}

void ACharacterBase::SprintStart()
{
	if (this->bIsDead)
	{
		return;
	}

	if (Role < ROLE_Authority)
	{
		this->SprintStart_Server();
	}

	this->bIsSprinting = true;

	if (!this->bIsAiming && !this->bIsReloading)
	{
		this->GetCharacterMovement()->MaxWalkSpeed = this->SprintSpeed;
	}
}

bool ACharacterBase::SprintStart_Server_Validate()
{
	return true;
}

void ACharacterBase::SprintStart_Server_Implementation()
{
	this->SprintStart();
}

void ACharacterBase::SprintStop()
{
	if (Role < ROLE_Authority)
	{
		this->SprintStop_Server();
	}

	this->bIsSprinting = false;

	if (!this->bIsAiming)
	{
		this->GetCharacterMovement()->MaxWalkSpeed = this->JogSpeed;
	}
}

bool ACharacterBase::SprintStop_Server_Validate()
{
	return true;
}

void ACharacterBase::SprintStop_Server_Implementation()
{
	this->SprintStop();
}

void ACharacterBase::AimStart()
{
	if (this->bIsDead)
	{
		return;
	}

	if (Role < ROLE_Authority)
	{
		this->AimStart_Server();
	}

	this->bIsAiming = true;

	this->bUseControllerRotationYaw = true; // While the character is aiming he must use the controller's yaw rotation
	this->GetCharacterMovement()->bOrientRotationToMovement = false; // and he must not orient his rotation according to movement
	this->GetCharacterMovement()->MaxWalkSpeed = this->AimSpeed;
}

bool ACharacterBase::AimStart_Server_Validate()
{
	return true;
}

void ACharacterBase::AimStart_Server_Implementation()
{
	this->AimStart();
}

void ACharacterBase::AimStop()
{
	if (Role < ROLE_Authority)
	{
		this->AimStop_Server();
	}

	this->bIsFiring = false;
	this->bIsAiming = false;

	//this->bUseControllerRotationYaw = false;
	this->GetCharacterMovement()->bOrientRotationToMovement = true;

	if (this->bIsSprinting)
	{
		this->GetCharacterMovement()->MaxWalkSpeed = this->SprintSpeed;
	}
	else
	{
		this->GetCharacterMovement()->MaxWalkSpeed = this->JogSpeed;
	}
}

bool ACharacterBase::AimStop_Server_Validate()
{
	return true;
}

void ACharacterBase::AimStop_Server_Implementation()
{
	this->AimStop();
}

void ACharacterBase::FireStart_Key()
{
	this->FireStart(false);
}

void ACharacterBase::FireStart(bool FromClient)
{
	if (Role < ROLE_Authority)
	{
		this->FireStart_Server(true);
		return;
	}

	this->FireFromClient = FromClient;

	this->ShootingGateOpen = true;

	if (!this->FirstShot)
	{
		if (this->DelayShot)
		{
			this->DelayShot = false;
			return;
		}
	}

	if (!this->bIsReloading && this->bIsAiming &&
		this->EquippedWeapon->AmmoInClip > 0.f)
	{
		this->bIsFiring = true;
		this->OnFire();

		this->FirstShot = false;

		this->GetWorldTimerManager().SetTimer(this, &ACharacterBase::OnFire, 1.f / this->EquippedWeapon->ShotsPerSecond, true);
	}
}

bool ACharacterBase::FireStart_Server_Validate(bool FromClient)
{
	return true;
}

void ACharacterBase::FireStart_Server_Implementation(bool FromClient)
{
	this->FireStart(FromClient);
}

bool ACharacterBase::OnFireEvent_Multicast_Validate(FVector Location)
{
	return true;
}

void ACharacterBase::OnFireEvent_Multicast_Implementation(FVector Location)
{
	UGameplayStatics::SpawnEmitterAttached(this->EquippedWeapon->WeaponShotFX, this->EquippedWeapon->WeaponMesh, this->EquippedWeapon->GunMuzzleSocketName);
	UGameplayStatics::PlaySoundAtLocation(this->GetWorld(), this->EquippedWeapon->WeaponShotSFX, Location);
}

void ACharacterBase::FireStop()
{
	if (Role < ROLE_Authority)
	{
		this->FireStop_Server();
	}

	this->bIsFiring = false;
	this->DelayShot = true;
	this->ShootingGateOpen = false;
}

bool ACharacterBase::FireStop_Server_Validate()
{
	return true;
}

void ACharacterBase::FireStop_Server_Implementation()
{
	this->FireStop();
}

void ACharacterBase::OnFire()
{
}

bool ACharacterBase::OnFire_Server_Validate(FVector SpawnLocation, FRotator SpawnRotation, AController* Shooter)
{
	return true;
}

void ACharacterBase::OnFire_Server_Implementation(FVector SpawnLocation, FRotator SpawnRotation, AController* Shooter)
{
	FActorSpawnParameters ProjSpawnParams;
	ProjSpawnParams.bNoCollisionFail = true;

	AProjectileBase* SpawnedProjectile = this->GetWorld()->SpawnActor<AProjectileBase>(this->EquippedWeapon->ProjectileClass, SpawnLocation, SpawnRotation, ProjSpawnParams);
	SpawnedProjectile->Shooter = Shooter;
}

void ACharacterBase::OnFire_Client_Implementation()
{
	this->EquippedWeapon->AmmoInClip--;
}

void ACharacterBase::ReloadStart()
{
	if (this->bIsDead)
	{
		return;
	}

	if (Role < ROLE_Authority)
	{
		this->ReloadStart_Server();
		return;
	}

	if ((this->EquippedWeapon != NULL) &&
		(this->EquippedWeapon->AmmoInClip < this->EquippedWeapon->ClipCapacity) &&
		(this->EquippedWeapon->RemainingAmmo > 0) &&
		!this->bIsReloading)
	{
		this->bIsFiring = false;
		this->bIsReloading = true;

		if (this->ReloadAnimTimeRemaining <= 0.f)
		{
			this->OnReloadStart_Multicast();
		}

		if (!this->bIsAiming)
		{
			this->GetCharacterMovement()->MaxWalkSpeed = this->JogSpeed;
		}
	}
}

bool ACharacterBase::ReloadStart_Server_Validate()
{
	return true;
}

void ACharacterBase::ReloadStart_Server_Implementation()
{
	this->ReloadStart();
}

bool ACharacterBase::OnReloadStart_Server_Validate()
{
	return true;
}

void ACharacterBase::OnReloadStart_Server_Implementation()
{
	if (this->ReloadAnimTimeRemaining <= 0.f)
	{
		this->OnReloadStart_Multicast();
	}
}

void ACharacterBase::OnReloadStart_Multicast_Implementation()
{
	this->ReloadAnimTimeRemaining = this->PlayAnimMontage(this->ReloadAnim);
}

void ACharacterBase::ReloadStop()
{
	this->bIsReloading = false;

	if (Role < ROLE_Authority)
	{
		this->ReloadStop_Server();
	}

	if (this->bIsAiming)
	{
		this->GetCharacterMovement()->MaxWalkSpeed = this->AimSpeed;
	}
	else if (this->bIsSprinting)
	{
		this->GetCharacterMovement()->MaxWalkSpeed = this->SprintSpeed;
	}
	else
	{
		this->GetCharacterMovement()->MaxWalkSpeed = this->JogSpeed;
	}
}

bool ACharacterBase::ReloadStop_Server_Validate()
{
	return true;
}

void ACharacterBase::ReloadStop_Server_Implementation()
{
	this->ReloadStop();
}

void ACharacterBase::Reload()
{
	if (this->EquippedWeapon != NULL)
	{
		this->EquippedWeapon->Reload();
		this->ReloadStop();
	}

	if (Role < ROLE_Authority)
	{
		this->Reload_Server();
	}
}

bool ACharacterBase::Reload_Server_Validate()
{
	return true;
}

void ACharacterBase::Reload_Server_Implementation()
{
	this->Reload();
}

void ACharacterBase::Respawn_Player_Client_Implementation()
{
	if (Role < ROLE_Authority)
	{
		this->Respawn_Player_Server();
	}

	AMainPlayerController* ThisController = Cast<AMainPlayerController>(this->GetController());

	this->DetachFromControllerPendingDestroy();

	if (ThisController != NULL)
	{
		//ThisController->Respawn();
		ThisController->ServerRestartPlayer();
	}
}

bool ACharacterBase::Respawn_Player_Server_Validate()
{
	return true;
}

void ACharacterBase::Respawn_Player_Server_Implementation()
{
	AMainPlayerController* ThisController = Cast<AMainPlayerController>(this->GetController());

	this->DetachFromControllerPendingDestroy();

	if (ThisController != NULL)
	{
		//ThisController->Respawn();
		ThisController->ServerRestartPlayer();
	}
}

void ACharacterBase::Despawn_Actor_Implementation()
{
	this->Destroy();
}

void ACharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (this->Rifle != NULL)
	{
		this->Rifle->Destroy();
	}

	if (this->RocketLauncher != NULL)
	{
		this->RocketLauncher->Destroy();
	}
}

void ACharacterBase::FellOutOfWorld(const class UDamageType& dmgType)
{
	if (Role < ROLE_Authority)
	{
		this->FellOutOfWorld_Server(&dmgType);
		return;
	}

	this->GetWorldTimerManager().PauseTimer(this, &ACharacterBase::UpdateEnergy);

	this->bIsDead = true;
	this->Health = 0;
	this->Energy = 0;

	this->FellOutOfWorld_StopEnergy();

	AMainPlayerController* ThisController = Cast<AMainPlayerController>(this->GetController());

	if (ThisController != NULL)
	{
		ThisController->Deaths++;
	}

	this->Destroy_Body();

	this->GetWorldTimerManager().PauseTimer(this, &ACharacterBase::OnFire);

	this->GetWorldTimerManager().SetTimer(this, &ACharacterBase::Respawn_Player_Client, 5.f, false);
	this->GetWorldTimerManager().SetTimer(this, &ACharacterBase::Despawn_Actor, 15.f, false);
}

bool ACharacterBase::FellOutOfWorld_Server_Validate(const class UDamageType* dmgType)
{
	return true;
}

void ACharacterBase::FellOutOfWorld_Server_Implementation(const class UDamageType* dmgType)
{
	this->FellOutOfWorld(*dmgType);
}

void ACharacterBase::FellOutOfWorld_StopEnergy_Implementation()
{
	this->GetWorldTimerManager().PauseTimer(this, &ACharacterBase::UpdateEnergy);
}

void ACharacterBase::Destroy_Body_Implementation()
{
	this->Rifle->Destroy();
	this->RocketLauncher->Destroy();
	this->GetCapsuleComponent()->DestroyComponent();
	this->GetMesh()->DestroyComponent();
}

bool ACharacterBase::UseEnergy(float EnergyValue)
{
	if (this->Energy < EnergyValue)
	{
		return false;
	}

	this->Energy -= EnergyValue;

	return true;
}

void ACharacterBase::UpdateEnergy()
{
	if (this->bIsSprinting && !this->bIsAiming && this->GetVelocity() != FVector(0, 0, 0))
	{
		this->Energy = FMath::Max(this->Energy - 1.f, 0.f);
	}
	else
	{
		this->Energy = FMath::Min(this->Energy + this->EnergyRegen, this->EnergyCapacity);
	}

	if (this->Energy <= 1.f && this->bIsSprinting)
	{
		this->SprintStop();
	}
}

void ACharacterBase::TakeDamage(float Damage, const FHitResult& Hit, AController* EventInstigator)
{
	if (Role < ROLE_Authority)
	{
		this->TakeDamage_Server(Damage, Hit, EventInstigator);
		return;
	}

	this->Health -= Damage;

	this->TakeDamageFX_Multicast(Hit.ImpactPoint);

	if (this->bIsDead)
	{
		return;
	}

	if (this->Health <= 0.f)
	{
		this->bIsSprinting = false;
		this->bIsAiming = false;
		this->bIsFiring = false;
		this->bIsReloading = false;
		this->bIsDead = true;

		this->GetWorldTimerManager().PauseTimer(this, &ACharacterBase::UpdateEnergy);

		this->Energy = 0;

		AMainPlayerController* ThisController = Cast<AMainPlayerController>(this->GetController());
		if (ThisController != NULL)
		{
			ThisController->Deaths++;
		}

		AMainPlayerController* ShooterController = Cast<AMainPlayerController>(EventInstigator);
		if (ShooterController != NULL && this->GetController() != EventInstigator)
		{
			ShooterController->Kills++;
		}

		if (!this->GetCharacterMovement()->IsFalling())
		{
			this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		this->GetWorldTimerManager().SetTimer(this, &ACharacterBase::Destroy_Body, 1.f, false);
		this->GetWorldTimerManager().SetTimer(this, &ACharacterBase::Respawn_Player_Client, 5.f, false);
		this->GetWorldTimerManager().SetTimer(this, &ACharacterBase::Despawn_Actor, 15.f, false);

		//this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		this->TakeDamage_Client();

		this->GetWorldTimerManager().PauseTimer(this, &ACharacterBase::OnFire);

		this->bUseControllerRotationYaw = false;
		this->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

bool ACharacterBase::TakeDamage_Server_Validate(float Damage, const FHitResult& Hit, AController* EventInstigator)
{
	return true;
}

void ACharacterBase::TakeDamage_Server_Implementation(float Damage, const FHitResult& Hit, AController* EventInstigator)
{
	this->TakeDamage(Damage, Hit, EventInstigator);
}

void ACharacterBase::TakeDamageFX_Multicast_Implementation(FVector ImpactPoint)
{
	UGameplayStatics::SpawnEmitterAtLocation(this->GetWorld(), this->HitFX, ImpactPoint);
}

void ACharacterBase::TakeDamage_Client_Implementation()
{
	this->bUseControllerRotationYaw = false;
	this->GetCharacterMovement()->bOrientRotationToMovement = false;
	this->GetWorldTimerManager().PauseTimer(this, &ACharacterBase::OnFire);
	this->GetWorldTimerManager().PauseTimer(this, &ACharacterBase::UpdateEnergy);
}

void ACharacterBase::ReceiveAnyDamage(float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	this->TakeDamage(Damage, FHitResult(), InstigatedBy);
}

FRotator ACharacterBase::GetAimOffsets() const
{
	if (this->bIsDead)
	{
		return FRotator(0.f, 0.f, 0.f);
	}

	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	//return FRotator(
	//	(AimRotLS.Pitch > 180) ? AimRotLS.Pitch - 300 : AimRotLS.Pitch - 60,
	//	(AimRotLS.Yaw > 180) ? AimRotLS.Yaw - 360 : AimRotLS.Yaw,
	//	(AimRotLS.Roll > 180) ? AimRotLS.Roll - 360 : AimRotLS.Roll);
	return AimRotLS;
}

bool ACharacterBase::SetCharPitch_Server_Validate()
{
	return true;
}

void ACharacterBase::SetCharPitch_Server_Implementation()
{
}
