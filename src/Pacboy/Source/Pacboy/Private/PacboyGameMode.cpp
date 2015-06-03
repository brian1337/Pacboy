// Fill out your copyright notice in the Description page of Project Settings.

#include "Pacboy.h"
#include "PacboyGameMode.h"
#include "MainPlayerController.h"

void APacboyGameMode::ChangeName(AController* Other, const FString& S, bool bNameChange)
{
	if (S.IsNumeric() || S.Len() > 10)
	{
		return;
	}

	Super::ChangeName(Other, S, true);
}