// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "PerceptionManager.generated.h"

class UPerceptionReceiver;
class UGenericTagTeamComponent;
class UAIPerceptionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GENERICTAGTEAMEXTRA_API UPerceptionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerceptionManager();

	UAIPerceptionComponent* GetOwnerPerceptionComponent() const;
	UGenericTagTeamComponent* GetPawnTagTeamComponent() const;
	APawn* GetPawn() const;
	static UPerceptionReceiver* TryGetPerceptionReceiver(const AActor* OtherActor);

	UFUNCTION(BlueprintNativeEvent)
	float DeltaAdd(const float& DeltaTime, const AActor* Other);

	UFUNCTION(BlueprintNativeEvent)
	float DeltaSub(const float& DeltaTime);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AddMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SubMultiplier = 1.0f;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UAIPerceptionComponent* AiPerceptionComponent = nullptr;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AActor*> PerceptionActors;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TMap<AActor*, float> PerceptionAlpha;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AActor* TrackingActor = nullptr;
	
protected:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
