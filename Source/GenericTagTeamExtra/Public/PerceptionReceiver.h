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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, NotReplicated)
	TMap<APawn*, int32> IndexMap;

	void UpdatePerceptionAlpha(APawn* Pawn, const float& PerceptionAlpha);

	// TODO : Implements add or remove delegates
	
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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GENERICTAGTEAMEXTRA_API UPerceptionReceiver : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerceptionReceiver();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	FPerceptionArray PerceptionArray;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UpdatePerceptionAlpha(APawn* AiPawn, float PerceptionAlpha)
	{
		PerceptionArray.UpdatePerceptionAlpha(AiPawn, PerceptionAlpha);
	}
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
