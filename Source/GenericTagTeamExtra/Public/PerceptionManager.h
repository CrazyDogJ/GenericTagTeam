// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "PerceptionManager.generated.h"

class UPerceptionReceiver;
class UGenericTagTeamComponent;
class UAIPerceptionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class GENERICTAGTEAMEXTRA_API UPerceptionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerceptionManager();

	UAIPerceptionComponent* GetOwnerPerceptionComponent() const;
	UGenericTagTeamComponent* GetPawnTagTeamComponent() const;
	APawn* GetPawn() const;
	static UPerceptionReceiver* TryGetPerceptionReceiver(const AActor* OtherActor);

	bool HasTag(const AActor* OtherActor) const;
	
	UFUNCTION(BlueprintNativeEvent)
	float DeltaAdd(const float& DeltaTime, const AActor* Other);

	UFUNCTION(BlueprintNativeEvent)
	float DeltaSub(const float& DeltaTime);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Perception|Settings")
	uint8 bUseForgotten : 1;
	
	/** If actor does not contain tag team component, we still percept that actor with specific tags. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Perception|Settings")
	TArray<FName> AdditionalTags;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Perception|Settings")
	float AddMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Perception|Settings")
	float SubMultiplier = 1.0f;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Perception|State")
	UAIPerceptionComponent* AiPerceptionComponent = nullptr;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Perception|State")
	TArray<AActor*> PerceptionActors;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Perception|State")
	TMap<AActor*, float> PerceptionAlpha;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Perception|State")
	TArray<AActor*> TrackingActors;
	
protected:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	UFUNCTION()
	void OnTargetPerceptionForgotten(AActor* Actor);
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
