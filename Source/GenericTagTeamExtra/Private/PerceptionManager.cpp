// Fill out your copyright notice in the Description page of Project Settings.


#include "PerceptionManager.h"

#include "AIController.h"
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

AAIController* UPerceptionManager::GetOwnerAiController() const
{
	return Cast<AAIController>(GetOwner());
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
	if (Controller && Controller->IsPlayerController())
	{
		return Cast<UPerceptionReceiver>(Controller->GetComponentByClass(UPerceptionReceiver::StaticClass()));
	}

	return nullptr;
}

bool UPerceptionManager::HasTag(const AActor* OtherActor) const
{
	// 修复：感知更新可能与目标销毁发生在同一帧。
	if (!IsValid(OtherActor)) return false;
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
	if (IsValid(AiPerceptionComponent) && IsValid(Actor))
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
	else
	{
		ForgetActor(Actor);
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

void UPerceptionManager::ClearReceiver(APawn* Pawn)
{
	const auto Targets = PerceptionAlpha;
	for (const auto Itr : Targets)
	{
		if (const auto Receiver = TryGetPerceptionReceiver(Itr.Key))
		{
			Receiver->RemovePerception(Pawn);
		}
		ForgetActor(Itr.Key);
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
					if (PerceptionAlpha.Contains(Actor)) PerceptActors.Add(Actor);
				}
				else
				{
					RememberActors.Add(Actor);
				}
			}
		}
	}

	// 修复：DeltaAdd、DeltaSub 和接收端通知可执行蓝图，不跨回调保存容器迭代器或值引用。
	for (auto* Actor : PerceptActors)
	{
		if (!IsValid(Actor)) continue;
		const float Delta = DeltaAdd(DeltaTime, Actor);
		if (auto* Alpha = PerceptionAlpha.Find(Actor)) *Alpha = FMath::Clamp(*Alpha + Delta, 0.0f, 1.0f);
	}

	TArray<AActor*> NeedToRemove;
	const auto Targets = PerceptionAlpha;
	for (auto AlphaPair : Targets)
	{
		// 修复：感知系统可能先于跟踪表遗忘目标。
		if (!IsValid(AlphaPair.Key))
		{
			NeedToRemove.Add(AlphaPair.Key);
			continue;
		}
		if (PerceptActors.Find(AlphaPair.Key) == INDEX_NONE && (AlphaPair.Value != 1.0f || RememberActors.Find(AlphaPair.Key) == INDEX_NONE))
		{
			const float Delta = DeltaSub(DeltaTime);
			auto* Current = PerceptionAlpha.Find(AlphaPair.Key);
			if (!Current || !IsValid(AlphaPair.Key)) continue;
			AlphaPair.Value = *Current = FMath::Clamp(*Current - Delta, 0.0f, 1.0f);
		}

		// Manage array.
		if (AlphaPair.Value == 0.0f)
		{
			NeedToRemove.Add(AlphaPair.Key);
		}
		else if (AlphaPair.Value == 1.0f)
		{
			// Update tracking info.
			const auto Info = IsValid(AiPerceptionComponent) ? AiPerceptionComponent->GetActorInfo(*AlphaPair.Key) : nullptr;
			if (!Info)
			{
				NeedToRemove.Add(AlphaPair.Key);
				UpdateReceiver({AlphaPair.Key, 0.0f});
				continue;
			}
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
		TrackingActors.Remove(Itr);
	}
}

void UPerceptionManager::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (OldPawn && OldPawn != NewPawn)
	{
		ClearReceiver(OldPawn);
	}
}

void UPerceptionManager::BeginPlay()
{
	Super::BeginPlay();

	if (const auto Controller = GetOwnerAiController())
	{
		Controller->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	}
	
	AiPerceptionComponent = GetOwnerPerceptionComponent();
	
	if (AiPerceptionComponent)
	{
		AiPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetPerceptionUpdated);
	}
}

void UPerceptionManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (const auto Controller = GetOwnerAiController())
	{
		Controller->OnPossessedPawnChanged.RemoveAll(this);
	}
	
	if (AiPerceptionComponent)
	{
		AiPerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
	}

	// 修复：感知管理器被单独移除时，也清理接收端仍引用的 AI。
	if (auto* Pawn = GetPawn()) ClearReceiver(Pawn);
	PerceptionAlpha.Empty();
	TrackingActors.Empty();
}
