// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "PacboyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PACBOY_API APacboyGameMode : public AGameMode
{

public:

	virtual void ChangeName(AController* Other, const FString& S, bool bNameChange) override;

private:

	GENERATED_BODY()

};
