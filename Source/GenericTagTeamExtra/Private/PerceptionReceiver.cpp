// Fill out your copyright notice in the Description page of Project Settings.


#include "PerceptionReceiver.h"

#include "Net/UnrealNetwork.h"

void FPerceptionArray::UpdatePerceptionAlpha(APawn* Pawn, const float& PerceptionAlpha)
{
	if (const auto Found = IndexMap.Find(Pawn))
	{
		auto& Entry = PerceptionArray[*Found];
		if (Entry.PerceptionAlpha != PerceptionAlpha)
		{
			if (PerceptionAlpha <= 0.0f)
			{
				PerceptionArray.RemoveAt(*Found);
				IndexMap.Remove(Pawn);
				MarkArrayDirty();
				return;
			}
			
			Entry.PerceptionAlpha = PerceptionAlpha;
			MarkItemDirty(Entry);
			return;
		}
		
		return;
	}

	const auto NewIndex = PerceptionArray.Add(FPerceptionEntry(Pawn, PerceptionAlpha));
	MarkItemDirty(PerceptionArray[NewIndex]);
	IndexMap.Add(Pawn, NewIndex);
}

UPerceptionReceiver::UPerceptionReceiver()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPerceptionReceiver::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PerceptionArray)
}
