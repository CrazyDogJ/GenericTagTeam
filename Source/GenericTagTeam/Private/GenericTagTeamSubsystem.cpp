// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericTagTeamSubsystem.h"

#include "AIController.h"
#include "GenericTagTeamSettings.h"

ETeamAttitude::Type UGenericTagTeamSubsystem::GetTeamAttitudeByTag(FGameplayTag InTeamTag, FGameplayTag OtherTeamTag) const
{
	if (const auto Found = RuntimeTeamSettings.Find(InTeamTag))
	{
		if (const auto FoundAttitude = Found->TeamAttitude.Find(OtherTeamTag))
		{
			const ETeamAttitude::Type Result = *FoundAttitude;
			return Result;
		}
	}

	return GetDefaultTeamAttitude();
}

ETeamAttitude::Type UGenericTagTeamSubsystem::GetTeamAttitudeById(FGenericTeamId InId, FGenericTeamId OtherTeamId) const
{
	const auto Settings = GetDefault<UGenericTagTeamSettings>();
	return GetTeamAttitudeByTag(Settings->GetTeamTag(InId), Settings->GetTeamTag(OtherTeamId));
}

bool UGenericTagTeamSubsystem::SetTeamAttitude(FGameplayTag ThisTeam, FGameplayTag OtherTeam,
                                               TEnumAsByte<ETeamAttitude::Type> TeamAttitude)
{
	if (const auto FoundTeam = RuntimeTeamSettings.Find(ThisTeam))
	{
		FoundTeam->TeamAttitude.Add(OtherTeam, TeamAttitude);
		return true;
	}

	return false;
}

void UGenericTagTeamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const auto TagTeamSettings = GetDefault<UGenericTagTeamSettings>();
	RuntimeTeamSettings = TagTeamSettings->DefaultTeamSettings;
}

void UGenericTagTeamSubsystem::Deinitialize()
{
	Super::Deinitialize();

	RuntimeTeamSettings.Empty();
}
