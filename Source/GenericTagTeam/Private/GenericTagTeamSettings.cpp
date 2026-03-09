// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericTagTeamSettings.h"

FGenericTeamId UGenericTagTeamSettings::GetGenericTeamId(const FGameplayTag& Tag) const
{
	TArray<FGameplayTag> Tags;
	DefaultTeamSettings.GenerateKeyArray(Tags);
	const auto Index = Tags.Find(Tag);
	if (Index >= 0)
	{
		return Index;
	}
	
	return FGenericTeamId();
}

FGameplayTag UGenericTagTeamSettings::GetTeamTag(const FGenericTeamId& Id) const
{
	TArray<FGameplayTag> Tags;
	DefaultTeamSettings.GenerateKeyArray(Tags);
	if (Tags.IsValidIndex(Id.GetId()))
	{
		return Tags[Id.GetId()];
	}

	return FGameplayTag();
}

FName UGenericTagTeamSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}
