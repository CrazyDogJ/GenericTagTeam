// Fill out your copyright notice in the Description page of Project Settings.


#include "PerceptionManager.h"

#include "GenericTagTeamComponent.h"
#include "PerceptionReceiver.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Damage.h"

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
		if (Source && Source->ComponentHasTag(Itr))
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

void UPerceptionManager::ForgetActor(AActor* Actor)
{
	if (AiPerceptionComponent)
	{
		AiPerceptionComponent->ForgetActor(Actor);
	}
}

void UPerceptionManager::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	TEnumAsByte<ETeamAttitude::Type> Attitude;
	const auto TagTeamComp = GetPawnTagTeamComponent();
	const auto TagTeamCondition = TagTeamComp && TagTeamComp->GetOtherAttitude(Actor, Attitude) && Attitude == ETeamAttitude::Hostile;
	if (TagTeamCondition || HasTag(Actor))
	{
		// Add if not exist.
		if (Stimulus.WasSuccessfullySensed())
		{
			if (PerceptionAlpha.Find(Actor) == nullptr)
			{
				// If damaged, we immediately find character.
				if (Stimulus.Type == UAISense::GetSenseID(UAISense_Damage::StaticClass()))
				{
					PerceptionAlpha.Add(Actor, 1.0f);
					UpdateReceiver(TPair<AActor*, float>(Actor, 1.0f));
				}
				// Normal state.
				else
				{
					PerceptionAlpha.Add(Actor, 0.0f);
				}
			}
		}
	}
}

void UPerceptionManager::UpdateReceiver(const TPair<AActor*, float> InPair) const
{
	const auto Receiver = TryGetPerceptionReceiver(InPair.Key);
	if (Receiver && GetPawn())
	{
		Receiver->UpdatePerceptionAlpha(GetPawn(), FMath::Clamp(InPair.Value, 0.0f, 1.0f));
	}
}

void UPerceptionManager::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<AActor*> PerceptActors;
	TArray<AActor*> RememberActors;
	if (AiPerceptionComponent)
	{
		for (FActorPerceptionContainer::TConstIterator DataIt = AiPerceptionComponent->GetPerceptualDataConstIterator(); DataIt; ++DataIt)
		{
			if (AActor* Actor = DataIt->Key.ResolveObjectPtr())
			{
				if (DataIt->Value.HasAnyCurrentStimulus())
				{
					if (const auto Found = PerceptionAlpha.Find(Actor))
					{
						PerceptionAlpha.Add(Actor, FMath::Clamp(*Found + DeltaAdd(DeltaTime, Actor), 0.0f, 1.0f));
						PerceptActors.Add(Actor);
					}
				}
				else
				{
					RememberActors.Add(Actor);
				}
			}
		}
	}

	TArray<AActor*> NeedToRemove;
	for (auto& AlphaPair : PerceptionAlpha)
	{
		if (PerceptActors.Find(AlphaPair.Key) == INDEX_NONE && (AlphaPair.Value != 1.0f || RememberActors.Find(AlphaPair.Key) == INDEX_NONE))
		{
			AlphaPair.Value = FMath::Clamp(AlphaPair.Value - DeltaSub(DeltaTime), 0.0f, 1.0f);
		}

		// Manage array.
		if (AlphaPair.Value == 0.0f)
		{
			NeedToRemove.Add(AlphaPair.Key);
		}
		else if (AlphaPair.Value == 1.0f)
		{
			// Update tracking info.
			const auto Info = AiPerceptionComponent->GetActorInfo(*AlphaPair.Key);
			// Update last velocity.
			const auto bPercept = Info->HasAnyCurrentStimulus();
			if (const auto Found = TrackingActors.Find(AlphaPair.Key))
			{
				Found->LastLocation = Info->GetLastStimulusLocation();
				Found->bIsPercept = bPercept;
				Found->LastVelocity = bPercept ? AlphaPair.Key->GetVelocity() : Found->LastVelocity;
			}
			else
			{
				TrackingActors.Add(AlphaPair.Key, FPerceptionInfo(Info->GetLastStimulusLocation(), bPercept,
					bPercept ? AlphaPair.Key->GetVelocity() : FVector::Zero()));
			}
		}
		else
		{
			// Remove tracking info.
			TrackingActors.Remove(AlphaPair.Key);
		}
		
		// Update receiver.
		UpdateReceiver(AlphaPair);
	}

	for (const auto Itr : NeedToRemove)
	{
		ForgetActor(Itr);
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
