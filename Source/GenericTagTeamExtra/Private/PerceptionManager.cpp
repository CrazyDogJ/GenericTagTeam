// Fill out your copyright notice in the Description page of Project Settings.


#include "PerceptionManager.h"

#include "GenericTagTeamComponent.h"
#include "PerceptionReceiver.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISenseConfig_Sight.h"

UPerceptionManager::UPerceptionManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

UAIPerceptionComponent* UPerceptionManager::GetOwnerPerceptionComponent() const
{
	if (GetOwner())
	{
		const auto Comp = GetOwner()->GetComponentByClass(UAIPerceptionComponent::StaticClass());
		return Cast<UAIPerceptionComponent>(Comp);
	}

	return nullptr;
}

UGenericTagTeamComponent* UPerceptionManager::GetPawnTagTeamComponent() const
{
	if (const auto Pawn = GetPawn())
	{
		const auto Comp = Pawn->GetComponentByClass(UGenericTagTeamComponent::StaticClass());
		return Cast<UGenericTagTeamComponent>(Comp);
	}

	return nullptr;
}

APawn* UPerceptionManager::GetPawn() const
{
	if (const auto Controller = Cast<AController>(GetOwner()))
	{
		return Controller->GetPawn();
	}

	return nullptr;
}

UPerceptionReceiver* UPerceptionManager::TryGetPerceptionReceiver(const AActor* OtherActor)
{
	if (!OtherActor)
	{
		return nullptr;
	}

	const auto OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn)
	{
		return nullptr;
	}
	
	const auto Controller = OtherPawn->GetController();
	if (Controller->IsPlayerController())
	{
		return Cast<UPerceptionReceiver>(Controller->GetComponentByClass(UPerceptionReceiver::StaticClass()));
	}

	return nullptr;
}

bool UPerceptionManager::HasTag(const AActor* OtherActor) const
{
	const auto Source = OtherActor->GetComponentByClass<UAIPerceptionStimuliSourceComponent>();
	for (const auto Itr : AdditionalTags)
	{
		if (Source->ComponentHasTag(Itr))
		{
			return true;
		}
	}

	return false;
}

float UPerceptionManager::DeltaSub_Implementation(const float& DeltaTime)
{
	return DeltaTime * SubMultiplier;
}

float UPerceptionManager::DeltaAdd_Implementation(const float& DeltaTime, const AActor* Other)
{
	const auto Pawn = GetPawn();
	if (AiPerceptionComponent && Pawn && Other)
	{
		if (const auto Sight = AiPerceptionComponent->GetSenseConfig<UAISenseConfig_Sight, UAISenseConfig_Sight>())
		{
			const auto Distance = FVector::Distance(Pawn->GetActorLocation(), Other->GetActorLocation());
			return (1 - Distance / Sight->SightRadius) * AddMultiplier * DeltaTime;
		}
	}

	return DeltaTime;
}

void UPerceptionManager::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	TEnumAsByte<ETeamAttitude::Type> Attitude;
	const auto TagTeamComp = GetPawnTagTeamComponent();
	const auto TagTeamCondition = TagTeamComp && TagTeamComp->GetOtherAttitude(Actor, Attitude) && Attitude == ETeamAttitude::Hostile;
	if (TagTeamCondition || HasTag(Actor))
	{
		// Add
		if (Stimulus.WasSuccessfullySensed())
		{
			PerceptionActors.Add(Actor);
			PerceptionAlpha.Add(Actor, 0.0f);
			return;
		}
			
		// Remove
		PerceptionActors.Remove(Actor);
		if (TrackingActor == Actor)
		{
			TrackingActor = nullptr;
		}
	}
}

void UPerceptionManager::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<AActor*> NeedToRemove;
	
	for (auto& Itr : PerceptionAlpha)
	{
		if (PerceptionActors.Find(Itr.Key) >= 0)
		{
			Itr.Value = FMath::Clamp(Itr.Value + DeltaAdd(DeltaTime, Itr.Key), 0.0f, 1.0f);
		}
		else
		{
			Itr.Value = FMath::Clamp(Itr.Value - DeltaSub(DeltaTime), 0.0f, 1.0f);
		}

		// Update receiver.
		const auto Receiver = TryGetPerceptionReceiver(Itr.Key);
		if (Receiver && GetPawn())
		{
			Receiver->UpdatePerceptionAlpha(GetPawn(), FMath::Clamp(Itr.Value, 0.0f, 1.0f));
		}
		
		if (Itr.Value == 0.0f)
		{
			NeedToRemove.Add(Itr.Key);
		}

		if (Itr.Value == 1.0f)
		{
			if (!TrackingActor)
			{
				TrackingActor = Itr.Key;
			}
		}
	}

	for (const auto Itr : NeedToRemove)
	{
		PerceptionAlpha.Remove(Itr);
	}
}

void UPerceptionManager::BeginPlay()
{
	Super::BeginPlay();

	AiPerceptionComponent = GetOwnerPerceptionComponent();
	
	if (AiPerceptionComponent)
	{
		AiPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetPerceptionUpdated);
	}
}

void UPerceptionManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (AiPerceptionComponent)
	{
		AiPerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
	}
}
