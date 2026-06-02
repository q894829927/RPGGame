// Fill out your copyright notice in the Description page of Project Settings.



#include "Actor/AuraEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/SphereComponent.h"
// Sets default values
AAuraEffectActor::AAuraEffectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot")));
}


// Called when the game starts or when spawned
void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
}

/**
 * 将指定的 Gameplay Effect 应用到目标 Actor 身上
 * @param Target               接收效果的目标对象
 * @param GameplayEffectClass  要应用的效果类（在编辑器中定义的蓝图类）
 */
void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// 1. 获取目标的 AbilitySystemComponent 组件
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	// 安全检查：确保目标拥有 ASC 组件
	if (TargetASC == nullptr) return;
	check(GameplayEffectClass);
	// 2. 创建 EffectContextHandle，用于存储效果相关的上下文数据
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();

	// 3. 将当前 Actor (this) 设为效果的来源对象
	EffectContextHandle.AddSourceObject(this);

	// 4. 创建效果规格句柄 (SpecHandle)，传入效果类、等级(1.0)和上下文
	FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	if (EffectSpecHandle.IsValid())
	{
		// 获取 Spec 数据指针并解引用传入应用函数
		FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		
		// 检查效果是否为无限时长
		const bool bIsInfiniteEffect = EffectSpecHandle.Data->Def->DurationPolicy == EGameplayEffectDurationType::Infinite;
		if (bIsInfiniteEffect && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
		{
			ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
		}
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (InstanceEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstanceGameplayEffectClass);
	}
	if(DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if(InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if(InstanceEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstanceGameplayEffectClass);
	}
	if(DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if(InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
	if(InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!(IsValid(TargetASC))) return;

		TArray<FActiveGameplayEffectHandle> HandlesToRemove;

		for (const TPair<FActiveGameplayEffectHandle, TObjectPtr<UAbilitySystemComponent>>& HandlePair : ActiveEffectHandles)
		{
			if (HandlePair.Value == TargetASC)
			{
				TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
				HandlesToRemove.Add(HandlePair.Key);
			}
		}
		for (const FActiveGameplayEffectHandle& Handle : HandlesToRemove)
		{
			//ActiveEffectHandles.FindAndRemoveChecked(Handle);
			ActiveEffectHandles.Remove(Handle);
		}
	}
}
