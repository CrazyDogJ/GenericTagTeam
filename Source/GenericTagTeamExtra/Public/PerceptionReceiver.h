// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "PerceptionReceiver.generated.h"

USTRUCT(BlueprintType)
struct FPerceptionEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FPerceptionEntry() {}
	FPerceptionEntry(APawn* InAiPawn, const float& InPerceptionAlpha)
		: AiPawn(InAiPawn), PerceptionAlpha(InPerceptionAlpha) {}
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	APawn* AiPawn = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PerceptionAlpha = 0.0f;
};

USTRUCT(BlueprintType)
struct FPerceptionArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FPerceptionEntry> PerceptionArray;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, NotReplicated)
	TMap<APawn*, float> PerceptionMap;

	float GetPerception(const APawn* AiPawn);
	void RemovePerception(APawn* AiPawn);
	void UpdatePerceptionAlpha(APawn* Pawn, const float& PerceptionAlpha);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPerceptionChangedEvent, const TArray<APawn*>& ChangedIndices)
	
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
	{
		TArray<APawn*> Pawns;
		for (const auto Itr : RemovedIndices)
		{
			PerceptionMap.Remove(PerceptionArray[Itr].AiPawn);
			Pawns.Add(PerceptionArray[Itr].AiPawn);
		}
		PerceptionRemoveEvent.Broadcast(Pawns);
	}
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
	{
		TArray<APawn*> Pawns;
		for (const auto Itr : AddedIndices)
		{
			PerceptionMap.Add(PerceptionArray[Itr].AiPawn, PerceptionArray[Itr].PerceptionAlpha);
			Pawns.Add(PerceptionArray[Itr].AiPawn);
		}
		PerceptionAddEvent.Broadcast(Pawns);
	}
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
	{
		TArray<APawn*> Pawns;
		for (const auto Itr : ChangedIndices)
		{
			PerceptionMap.Add(PerceptionArray[Itr].AiPawn, PerceptionArray[Itr].PerceptionAlpha);
			Pawns.Add(PerceptionArray[Itr].AiPawn);
		}
		PerceptionChangedEvent.Broadcast(Pawns);
	}

	FOnPerceptionChangedEvent PerceptionRemoveEvent;
	FOnPerceptionChangedEvent PerceptionAddEvent;
	FOnPerceptionChangedEvent PerceptionChangedEvent;
	
	// Replication
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FPerceptionEntry, FPerceptionArray>(PerceptionArray, DeltaParams, *this);
	}
};

template<> struct TStructOpsTypeTraits<FPerceptionArray> : public TStructOpsTypeTraitsBase2<FPerceptionArray>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class GENERICTAGTEAMEXTRA_API UPerceptionReceiver : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerceptionReceiver();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	FPerceptionArray PerceptionArray;

	UFUNCTION(BlueprintPure)
	float GetPerceptionAlpha(const APawn* AiPawn, bool& bSuccess);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemovePerception(APawn* AiPawn)
	{
		PerceptionArray.RemovePerception(AiPawn);
	}
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UpdatePerceptionAlpha(APawn* AiPawn, float PerceptionAlpha)
	{
		PerceptionArray.UpdatePerceptionAlpha(AiPawn, PerceptionAlpha);
	}

	UFUNCTION(BlueprintImplementableEvent)
	void OnPerceptionAdd(const TArray<APawn*>& ArrayIndices);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPerceptionRemove(const TArray<APawn*>& ArrayIndices);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
