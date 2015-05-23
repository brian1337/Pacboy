// Fill out your copyright notice in the Description page of Project Settings.

#include "Pacboy.h"
#include "MainPlayerController.h"

#include "UnrealNetwork.h"

AMainPlayerController::AMainPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AMainPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerController, Kills);
	DOREPLIFETIME(AMainPlayerController, Deaths);
}
