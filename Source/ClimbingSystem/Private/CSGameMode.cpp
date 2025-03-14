// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSGameMode.h"
#include "..\Public\CSCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACSGameMode::ACSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
