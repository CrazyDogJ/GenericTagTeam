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
			// 修复：感知配置数组允许空条目。
			if (SenseConfig && SenseConfig->GetClass() == ConfigClass)
			{
				return SenseConfig;
			}
		}
	}

	return nullptr;
}

void UGenericTagTeamLibrary::ForgetActor(UAIPerceptionComponent* AiPerception, AActor* ForgetActor)
{
	if (AiPerception)
	{
		AiPerception->ForgetActor(ForgetActor);
	}
}
