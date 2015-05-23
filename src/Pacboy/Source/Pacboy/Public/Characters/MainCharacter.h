// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Characters/CharacterBase.h"
#include "MainCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PACBOY_API AMainCharacter : public ACharacterBase
{
public:

	/** Camera boom, positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	/** Extension for the camera boom. Used for OTS (Over the Shoulder) view */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoomExtension;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	/** The length of the camera boom while the character is aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraBoomLengthWhileAiming;

	/** The length of the camera boom extension while the character is aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float CameraBoomExtensionLengthWhileAiming;

	/** The smooth speed at which the camera transitions between 2 points in space (A multiplier for DeltaTime) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float CameraTransitionSmoothSpeed;

	/** The left and right mouse movement sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float MouseXSensitivity;

	/** The up and down mouse movement sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float MouseYSensitivity;

	AMainCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void OnFire() override;

	/** Toggles camera position (left/right) while aiming */
	virtual void ToggleCameraPosition();

protected:

	/**
	* Turns the character left or right
	* @param AxisValue - value in range [-1.0, 1.0], (-1 -> TurnLeft, 1 -> TurnRight)
	*/
	virtual void Turn(float AxisValue);

	/**
	* Turns the character up or down
	* @param AxisValue - value in range [-1.0, 1.0], (-1 -> LookUp, 1 -> LookDown)
	*/
	virtual void LookUp(float AxisValue);

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

private:

	/** The starting length of the camera boom */
	float CameraBoomLengthCache;

	/** The starting length of the camera boom extension */
	float CameraBoomExtensionLengthCache;

	/** Move the camera closer to the character */
	void MoveCameraCloserToCharacter(float TransitionSmoothSpeed, float DeltaTime);

	/** Move the camera away from the character */
	void MoveCameraFurtherFromCharacter(float TransitionSmoothSpeed, float DeltaTime);

	GENERATED_BODY()

};
