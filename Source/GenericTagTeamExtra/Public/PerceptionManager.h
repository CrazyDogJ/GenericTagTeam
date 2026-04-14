// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "PerceptionManager.generated.h"

class UPerceptionReceiver;
class UGenericTagTeamComponent;
class UAIPerceptionComponent;

USTRUCT(BlueprintType)
struct FPerceptionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector LastLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsPercept = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector LastVelocity = FVector::ZeroVector;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class GENERICTAGTEAMEXTRA_API UPerceptionManager : public UActorComponent
{
	GENERATED_BODY()

	typedef TMap<TObjectKey<AActor>, FActorPerceptionInfo> TActorPerceptionContainer;
	typedef TActorPerceptionContainer FActorPerceptionContainer;
	
public:
	UPerceptionManager();

	UAIPerceptionComponent* GetOwnerPerceptionComponent() const;
	UGenericTagTeamComponent* GetPawnTagTeamComponent() const;
	AAIController* GetOwnerAiController() const;
	APawn* GetPawn() const;
	static UPerceptionReceiver* TryGetPerceptionReceiver(const AActor* OtherActor);

	bool HasTag(const AActor* OtherActor) const;
	
	UFUNCTION(BlueprintNativeEvent)
	float DeltaAdd(const float& DeltaTime, const AActor* Other);

	UFUNCTION(BlueprintNativeEvent)
	float DeltaSub(const float& DeltaTime);
	
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
	TMap<AActor*, float> PerceptionAlpha;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Perception|State")
	TMap<AActor*, FPerceptionInfo> TrackingActors;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ForgetActor(AActor* Actor);
	
protected:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void UpdateReceiver(const TPair<AActor*, float> InPair) const;

	void ClearReceiver(APawn* Pawn);
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
