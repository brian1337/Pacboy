// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DamageableObject.generated.h"

/**
*
*/
UINTERFACE()
class PACBOY_API UDamageableObject : public UInterface
{
private:

	GENERATED_UINTERFACE_BODY()

};

class PACBOY_API IDamageableObject
{
public:

	/**
	* The IDamageableObject takes damage
	* @param Damage - How much damage to apply
	* @param Hit - Hit information
	* @param EventInstigator - The Controller responsible for the damage
	*/
	virtual void TakeDamage(float Damage, const FHitResult& Hit, AController* EventInstigator) = 0;

private:

	GENERATED_IINTERFACE_BODY()

};
