// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericTagTeamLibrary.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"

UAISenseConfig* UGenericTagTeamLibrary::GetSenseConfig(const UAIPerceptionComponent* AiPerception,
                                                       TSubclassOf<UAISenseConfig> ConfigClass)
{
	if (AiPerception)
	{
		for (auto Index = AiPerception->GetSensesConfigIterator(); Index; ++Index)
		{
			auto SenseConfig = *Index;
			if (SenseConfig->GetClass() == ConfigClass)
			{
				return SenseConfig;
			}
		}
	}

	return nullptr;
}
