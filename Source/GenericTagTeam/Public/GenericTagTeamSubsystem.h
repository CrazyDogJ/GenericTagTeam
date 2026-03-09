// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTagTeamSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GenericTagTeamSubsystem.generated.h"

class AAIController;

UCLASS()
class GENERICTAGTEAM_API UGenericTagTeamSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FGameplayTag, FTeamAttitude> RuntimeTeamSettings;
	
	static ETeamAttitude::Type GetDefaultTeamAttitude() { return GetDefault<UGenericTagTeamSettings>()->DefaultTeamAttitude; }

	UFUNCTION(BlueprintPure)
	ETeamAttitude::Type GetTeamAttitudeByTag(FGameplayTag InTeamTag, FGameplayTag OtherTeamTag) const;

	UFUNCTION(BlueprintPure)
	ETeamAttitude::Type GetTeamAttitudeById(FGenericTeamId InId, FGenericTeamId OtherTeamId) const;
	
	// Set runtime team attitude.
	UFUNCTION(BlueprintCallable)
	bool SetTeamAttitude(FGameplayTag ThisTeam, FGameplayTag OtherTeam, TEnumAsByte<ETeamAttitude::Type> TeamAttitude);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
