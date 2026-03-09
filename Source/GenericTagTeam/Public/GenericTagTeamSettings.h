// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "Engine/DeveloperSettings.h"
#include "GenericTagTeamSettings.generated.h"

USTRUCT(BlueprintType)
struct FTeamAttitude
{
	GENERATED_BODY()

	// To others attitude.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FGameplayTag, TEnumAsByte<ETeamAttitude::Type>> TeamAttitude;
};

UCLASS(Config = "GenericTagTeam", defaultconfig, meta = (DisplayName = "Generic Tag Team"))
class GENERICTAGTEAM_API UGenericTagTeamSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// Key is team tag, value is to others attitudes.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Config)
	TMap<FGameplayTag, FTeamAttitude> DefaultTeamSettings;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Config)
	TEnumAsByte<ETeamAttitude::Type> DefaultTeamAttitude = ETeamAttitude::Neutral;

	FGenericTeamId GetGenericTeamId(const FGameplayTag& Tag) const;
	FGameplayTag GetTeamTag(const FGenericTeamId& Id) const;
	
	virtual FName GetCategoryName() const override;
};
