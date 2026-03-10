// Fill out your copyright notice in the Description page of Project Settings.


#include "PerceptionReceiver.h"

#include "Net/UnrealNetwork.h"

void FPerceptionArray::UpdatePerceptionAlpha(APawn* Pawn, const float& PerceptionAlpha)
{
	const auto Found = PerceptionArray.IndexOfByPredicate([Pawn](const FPerceptionEntry& In)
	{
		return In.AiPawn == Pawn;
	});
	
	if (Found >= 0)
	{
		auto& Entry = PerceptionArray[Found];
		if (Entry.PerceptionAlpha != PerceptionAlpha)
		{
			if (PerceptionAlpha <= 0.0f)
			{
				PerceptionRemoveEvent.Broadcast({Found});
				PerceptionArray.RemoveAt(Found);
				MarkArrayDirty();
				return;
			}
			
			Entry.PerceptionAlpha = PerceptionAlpha;
			PerceptionChangedEvent.Broadcast({Found});
			MarkItemDirty(Entry);
			return;
		}
		
		return;
	}

	const auto NewIndex = PerceptionArray.Add(FPerceptionEntry(Pawn, PerceptionAlpha));
	PerceptionAddEvent.Broadcast({NewIndex});
	MarkItemDirty(PerceptionArray[NewIndex]);
}

UPerceptionReceiver::UPerceptionReceiver()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPerceptionReceiver::BeginPlay()
{
	Super::BeginPlay();
	
	PerceptionArray.PerceptionAddEvent.AddUObject(this, &ThisClass::OnPerceptionAdd);
	PerceptionArray.PerceptionRemoveEvent.AddUObject(this, &ThisClass::OnPerceptionRemove);
}

void UPerceptionReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	PerceptionArray.PerceptionAddEvent.RemoveAll(this);
	PerceptionArray.PerceptionRemoveEvent.RemoveAll(this);
}

void UPerceptionReceiver::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PerceptionArray)
}
