// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericTagTeamComponent.h"

#include "GenericTagTeamSubsystem.h"
#include "GameFramework/PlayerState.h"

UGenericTagTeamComponent::UGenericTagTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UGenericTagTeamComponent* UGenericTagTeamComponent::GetOtherComponent(const AActor* Other) const
{
	if (const auto Direct = Other->GetComponentByClass(StaticClass()))
	{
		UGenericTagTeamComponent* Result = Cast<UGenericTagTeamComponent>(Direct);
		return Result;
	}

	if (const auto Pawn = Cast<APawn>(Other))
	{
		if (const auto Con = Pawn->GetController())
		{
			if (const auto Direct = Con->GetComponentByClass(StaticClass()))
			{
				UGenericTagTeamComponent* Result = Cast<UGenericTagTeamComponent>(Direct);
				return Result;
			}

			if (const auto PS = Con->GetPlayerState<APlayerState>())
			{
				if (const auto Direct = PS->GetComponentByClass(StaticClass()))
				{
					UGenericTagTeamComponent* Result = Cast<UGenericTagTeamComponent>(Direct);
					return Result;
				}
			}
		}
	}

	return nullptr;
}

bool UGenericTagTeamComponent::GetOtherAttitude(const AActor* Other, TEnumAsByte<ETeamAttitude::Type>& OutAttitude) const
{
	if (!Other)
	{
		OutAttitude = UGenericTagTeamSubsystem::GetDefaultTeamAttitude();
		return false;
	}
	
	const auto Subsystem = Other->GetWorld()->GetGameInstance()->GetSubsystem<UGenericTagTeamSubsystem>();
	if (const auto Comp = GetOtherComponent(Other))
	{
		OutAttitude = Subsystem->GetTeamAttitudeByTag(TeamTag, Comp->TeamTag);
		return true;
	}

	OutAttitude = UGenericTagTeamSubsystem::GetDefaultTeamAttitude();
	return false;
}
