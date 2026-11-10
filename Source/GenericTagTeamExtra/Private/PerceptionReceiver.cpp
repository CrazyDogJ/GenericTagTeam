// Fill out your copyright notice in the Description page of Project Settings.


#include "PerceptionReceiver.h"

#include "Net/UnrealNetwork.h"

float FPerceptionArray::GetPerception(const APawn* AiPawn)
{
	if (const auto Found = PerceptionMap.Find(AiPawn))
	{
		return *Found;
	}

	return -1.0f;
}

void FPerceptionArray::RemovePerception(APawn* AiPawn)
{
	const auto Found = PerceptionArray.IndexOfByPredicate([AiPawn](const FPerceptionEntry& In)
	{
		return In.AiPawn == AiPawn;
	});

	PerceptionMap.Remove(AiPawn);
	
	if (Found >= 0)
	{
		// 修复：回调可能再次修改感知数组，先完成删除再通知。
		PerceptionArray.RemoveAt(Found);
		MarkArrayDirty();
		PerceptionRemoveEvent.Broadcast({AiPawn});
	}
}

void FPerceptionArray::UpdatePerceptionAlpha(APawn* Pawn, const float& PerceptionAlpha)
{
	// 修复：目标已销毁或感知衰减到零时，仅清理已有条目。
	if (!IsValid(Pawn) || Pawn->IsActorBeingDestroyed() || PerceptionAlpha <= 0.0f)
	{
		RemovePerception(Pawn);
		return;
	}
	const auto Found = PerceptionArray.IndexOfByPredicate([Pawn](const FPerceptionEntry& In)
	{
		return In.AiPawn == Pawn;
	});
	
	if (Found >= 0)
	{
		auto& Entry = PerceptionArray[Found];
		if (Entry.PerceptionAlpha != PerceptionAlpha)
		{
			Entry.PerceptionAlpha = PerceptionAlpha;
			PerceptionMap.Add(Pawn, PerceptionAlpha);
			MarkItemDirty(Entry);
			PerceptionChangedEvent.Broadcast({Pawn});
			return;
		}
		
		return;
	}

	const auto NewIndex = PerceptionArray.Add(FPerceptionEntry(Pawn, PerceptionAlpha));
	PerceptionMap.Add(Pawn, PerceptionAlpha);
	MarkItemDirty(PerceptionArray[NewIndex]);
	PerceptionAddEvent.Broadcast({Pawn});
}

UPerceptionReceiver::UPerceptionReceiver()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

float UPerceptionReceiver::GetPerceptionAlpha(const APawn* AiPawn, bool& bSuccess)
{
	if (const auto Found = PerceptionArray.PerceptionMap.Find(AiPawn))
	{
		bSuccess = true;
		return *Found;
	}

	bSuccess = false;
	return -1.0f;
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
