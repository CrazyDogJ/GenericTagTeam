// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GenericTeamAgentInterface.h"
#include "GenericTagTeamComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GENERICTAGTEAM_API UGenericTagTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGenericTagTeamComponent();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag TeamTag;

	UFUNCTION(BlueprintCallable)
	UGenericTagTeamComponent* GetOtherComponent(const AActor* Other) const;

	UFUNCTION(BlueprintCallable)
	bool GetOtherAttitude(const AActor* Other, TEnumAsByte<ETeamAttitude::Type>& OutAttitude) const;
};
