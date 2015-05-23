// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PACBOY_API AMainPlayerController : public APlayerController
{
public:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Gameplay")
	int32 Kills;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Gameplay")
	int32 Deaths;

	AMainPlayerController(const FObjectInitializer& ObjectInitializer);

private:

	GENERATED_BODY()

};
