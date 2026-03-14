// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GenericTagTeamLibrary.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig;

UCLASS()
class GENERICTAGTEAM_API UGenericTagTeamLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AI", meta = (DeterminesOutputType = "ConfigClass"))
	static UAISenseConfig* GetSenseConfig(const UAIPerceptionComponent* AiPerception, TSubclassOf<UAISenseConfig> ConfigClass);

	UFUNCTION(BlueprintCallable, Category = "AI")
	static void ForgetActor(UAIPerceptionComponent* AiPerception, AActor* ForgetActor);
};
