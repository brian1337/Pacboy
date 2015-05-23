// Fill out your copyright notice in the Description page of Project Settings.

#include "Pacboy.h"
#include "MainCharacter.h"

AMainCharacter::AMainCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	this->CameraBoom = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, FName(TEXT("CameraBoom")));
	this->CameraBoom->AttachTo(this->RootComponent);
	this->CameraBoom->TargetArmLength = 200.f; // The camera follows at this distance behind the character
	this->CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the control rotation

	// Create an extension for the camera boom. This extension is used in OTS (Over the Shoulder) view
	this->CameraBoomExtension = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, FName(TEXT("CameraBoomExtension")));
	this->CameraBoomExtension->AttachTo(this->CameraBoom, USpringArmComponent::SocketName);
	this->CameraBoomExtension->TargetArmLength = 30.f; // The camera is this distance left of the target;
	this->CameraBoomExtension->bUsePawnControlRotation = false; // Already uses pawn control rotation, because it is attached to CameraBoom
	this->CameraBoomExtension->SetRelativeRotation(FRotator(-10.f, -90.f, 0.f)); // Rotate it -90 degrees so that the camera is over the right shoulder of the character

	// Create a follow camera
	this->FollowCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, FName(TEXT("FollowCamera")));
	this->FollowCamera->AttachTo(this->CameraBoomExtension, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	this->FollowCamera->bUsePawnControlRotation = false; // Already uses pawn control rotation, because it is attached to CameraBoom
	this->FollowCamera->SetWorldRotation(FRotator::ZeroRotator);

	this->CameraBoomLengthCache = this->CameraBoom->TargetArmLength;
	this->CameraBoomExtensionLengthCache = this->CameraBoomExtension->TargetArmLength;

	// Camera settings
	this->CameraBoomLengthWhileAiming = 80.f; // The camera is this distance behind the target
	this->CameraBoomExtensionLengthWhileAiming = 50.f; // The camera is this distance left of the target;
	this->CameraTransitionSmoothSpeed = 15.f; // The smooth speed at which the camera transitions between two points in space (A multiplier for DeltaTime)
	this->MouseXSensitivity = 1.f;
	this->MouseYSensitivity = 1.f;
}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->bIsAiming)
	{
		this->MoveCameraCloserToCharacter(this->CameraTransitionSmoothSpeed, DeltaTime);
	}
	else
	{
		this->MoveCameraFurtherFromCharacter(this->CameraTransitionSmoothSpeed, DeltaTime);
	}
}

void AMainCharacter::OnFire()
{
	if (this->EquippedWeapon != NULL &&
		this->bIsAiming && !this->bIsReloading &&
		this->EquippedWeapon->AmmoInClip > 0.f)
	{
		if (this->DelayShot)
		{
			this->DelayShot = false;
			this->GetWorldTimerManager().PauseTimer(this, &ACharacterBase::OnFire);

			if (!this->FirstShot)
			{
				return;
			}
		}

		if (this->FirstShot)
		{
			this->GetWorldTimerManager().SetTimer(this, &ACharacterBase::OnFire, 1.f / this->EquippedWeapon->ShotsPerSecond, true);
		}

		if (!this->ShootingGateOpen)
		{
			return;
		}

		UWorld* World = this->GetWorld();
		if (World != NULL)
		{
			// Find the spawn location of the shot
			FVector SpawnLocation = this->EquippedWeapon->WeaponMesh->GetSocketLocation(this->EquippedWeapon->GunMuzzleSocketName);

			// Find the spawn rotation of the shot
			const FRotator CameraRotation = this->FollowCamera->GetComponentRotation();
			const FVector CameraLocation = this->FollowCamera->GetComponentLocation();

			const FVector CameraForwardVector = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::X);

			const FVector RayStart = CameraLocation;
			const FVector RayEnd = RayStart + (CameraForwardVector * 10000.f);

			FCollisionQueryParams QueryParams(FName(TEXT("ShotTrace")), true, this);
			QueryParams.AddIgnoredActor(this);

			FHitResult HitResult;

			FVector ProjectileDirection;

			if (World->LineTraceSingle(HitResult, RayStart, RayEnd, ECollisionChannel::ECC_Camera, QueryParams))
			{
				ProjectileDirection = HitResult.Location - SpawnLocation; // If we hit something, we find more accurate shot direction
				//DrawDebugLine(World, SpawnLocation, HitResult.Location, FColor::Red, false, 5.f, (uint8)'\000', 1.f);
			}
			else
			{
				ProjectileDirection = RayEnd - SpawnLocation; // The default shot direction is from the SpawnLocation to the RayEnd
			}

			this->OnFireEvent_Multicast(SpawnLocation);

			// TODO: refactor
			if (this->EquippedWeapon->ShootingType == EWeaponShootingType::Instant)
			{
				FVector ImpactPoint = HitResult.ImpactPoint;
				UGameplayStatics::SpawnEmitterAtLocation(this->GetWorld(), this->EquippedWeapon->WeaponImpactFX, ImpactPoint);

				AActor* HitActor = HitResult.GetActor();
				if (HitActor != NULL)
				{
					IDamageableObject* DamageableObject = Cast<IDamageableObject>(HitActor);
					if (DamageableObject != NULL)
					{
						DamageableObject->TakeDamage(this->EquippedWeapon->Damage, HitResult, this->GetController());
					}
				}
			}
			else if (this->EquippedWeapon->ShootingType == EWeaponShootingType::Projectile)
			{
				const FRotator SpawnRotation = FRotationMatrix::MakeFromX(ProjectileDirection).Rotator();

				if (Role < ROLE_Authority)
				{
					this->OnFire_Server(SpawnLocation, SpawnRotation, this->GetController());
				}
				else
				{
					FActorSpawnParameters ProjSpawnParams;
					ProjSpawnParams.bNoCollisionFail = true;

					AProjectileBase* SpawnedProjectile = this->GetWorld()->SpawnActor<AProjectileBase>(this->EquippedWeapon->ProjectileClass, SpawnLocation, SpawnRotation, ProjSpawnParams);
					SpawnedProjectile->Shooter = this->GetController();
				}
			}

			this->EquippedWeapon->AmmoInClip--;

			if (Role >= ROLE_Authority && this->FireFromClient)
			{
				this->OnFire_Client();
			}
		}
	}
}

void AMainCharacter::Turn(float AxisValue)
{
	this->AddControllerYawInput(AxisValue * MouseXSensitivity);
}

void AMainCharacter::LookUp(float AxisValue)
{
	this->AddControllerPitchInput(AxisValue * MouseYSensitivity);
}

void AMainCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	if (InputComponent != NULL)
	{
		InputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
		InputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

		InputComponent->BindAxis("Turn", this, &AMainCharacter::Turn);
		InputComponent->BindAxis("LookUp", this, &AMainCharacter::LookUp);

		InputComponent->BindAction("ToggleCamera", IE_Pressed, this, &AMainCharacter::ToggleCameraPosition);

		InputComponent->BindAction("LeftDash", IE_Pressed, this, &AMainCharacter::LeftDash);
		InputComponent->BindAction("RightDash", IE_Pressed, this, &AMainCharacter::RightDash);

		InputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::SprintStart);
		InputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::SprintStop);

		InputComponent->BindAction("Aim", IE_Pressed, this, &AMainCharacter::AimStart);
		InputComponent->BindAction("Aim", IE_Released, this, &AMainCharacter::AimStop);

		InputComponent->BindAction("Fire", IE_Pressed, this, &AMainCharacter::FireStart_Key);
		InputComponent->BindAction("Fire", IE_Released, this, &AMainCharacter::FireStop);

		InputComponent->BindAction("Reload", IE_Pressed, this, &AMainCharacter::ReloadStart);

		InputComponent->BindAction("SwapToRifle", IE_Pressed, this, &AMainCharacter::SwapToRifle);
		InputComponent->BindAction("SwapToRocketLauncher", IE_Pressed, this, &AMainCharacter::SwapToRocketLauncher);

		InputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);
	}
}

void AMainCharacter::ToggleCameraPosition()
{
}

void AMainCharacter::MoveCameraCloserToCharacter(float TransitionSmoothSpeed, float DeltaTime)
{
	if (FMath::Abs(this->CameraBoom->TargetArmLength - this->CameraBoomLengthWhileAiming) > 1.f)
	{
		// The camera must get closer to the character
		this->CameraBoom->TargetArmLength =
			FMath::Lerp(this->CameraBoom->TargetArmLength, this->CameraBoomLengthWhileAiming, TransitionSmoothSpeed * DeltaTime);
	}

	if (FMath::Abs(this->CameraBoomExtension->TargetArmLength - this->CameraBoomExtensionLengthWhileAiming) > 1.f)
	{
		// The camera must be at this distance left of the target
		this->CameraBoomExtension->TargetArmLength =
			FMath::Lerp(this->CameraBoomExtension->TargetArmLength, this->CameraBoomExtensionLengthWhileAiming, TransitionSmoothSpeed * DeltaTime);
	}
}

void AMainCharacter::MoveCameraFurtherFromCharacter(float TransitionSmoothSpeed, float DeltaTime)
{
	if (FMath::Abs(this->CameraBoom->TargetArmLength - this->CameraBoomLengthCache) > 1.f)
	{
		// The camera must get further away from the character
		this->CameraBoom->TargetArmLength =
			FMath::Lerp(this->CameraBoom->TargetArmLength, this->CameraBoomLengthCache, TransitionSmoothSpeed * DeltaTime);
	}

	if (FMath::Abs(this->CameraBoomExtension->TargetArmLength - this->CameraBoomExtensionLengthCache) > 1.f)
	{
		// The camera must get closer to the right shoulder of the character
		this->CameraBoomExtension->TargetArmLength =
			FMath::Lerp(this->CameraBoomExtension->TargetArmLength, this->CameraBoomExtensionLengthCache, TransitionSmoothSpeed * DeltaTime);
	}
}
