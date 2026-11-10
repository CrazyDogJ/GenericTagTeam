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
	// 修复：蓝图可能传入空目标或已销毁的 Actor。
	if (!IsValid(Other)) return nullptr;
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
	
	// 修复：编辑器或世界退出期间可能没有 GameInstance。
	const auto World = Other->GetWorld();
	const auto Subsystem = UGameInstance::GetSubsystem<UGenericTagTeamSubsystem>(World ? World->GetGameInstance() : nullptr);
	const auto Comp = GetOtherComponent(Other);
	if (Subsystem && Comp)
	{
		OutAttitude = Subsystem->GetTeamAttitudeByTag(TeamTag, Comp->TeamTag);
		return true;
	}

	OutAttitude = UGenericTagTeamSubsystem::GetDefaultTeamAttitude();
	return false;
}
