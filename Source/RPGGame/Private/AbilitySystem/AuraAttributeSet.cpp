// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystemBlueprintLibrary.h"
UAuraAttributeSet::UAuraAttributeSet()
{
	InitHealth(50.f);
	InitMaxHealth(100.f);
	InitMana(0.f);
	InitMaxMana(50.f);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Replicate to everyone, but only send updates when the value changes. 
	// This is a bit more expensive than the default, 
	// but it means we won't have to worry about clients getting out of sync with the server.
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

}



void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// 这个函数在属性值被修改之前被调用，可以在这里对属性值进行限制或调整。
	// 例如，确保生命值不会超过最大生命值，或者魔法值不会为负数。
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
		UE_LOG(LogTemp, Warning, TEXT("Health: %f"), NewValue);
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
		SetHealth(FMath::Min(GetHealth(), NewValue));
		UE_LOG(LogTemp, Warning, TEXT("MaxHealth: %f"), NewValue);
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
		UE_LOG(LogTemp, Warning, TEXT("Mana: %f"), NewValue);
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
		SetMana(FMath::Min(GetMana(), NewValue));
		UE_LOG(LogTemp, Warning, TEXT("MaxMana: %f"), NewValue);
	}
	else if (Attribute == GetResilienceAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
		UE_LOG(LogTemp, Warning, TEXT("Resilience: %f"), NewValue);
	}
	else if (Attribute == GetVigorAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
		UE_LOG(LogTemp, Warning, TEXT("Vigor: %f"), NewValue);
	}
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& OutEffectProperties) const
{
	OutEffectProperties.EffectContextHandle = Data.EffectSpec.GetContext();
	OutEffectProperties.SourceASC = OutEffectProperties.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(OutEffectProperties.SourceASC) && OutEffectProperties.SourceASC->AbilityActorInfo.IsValid() && OutEffectProperties.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		OutEffectProperties.SourceAvatarActor = OutEffectProperties.SourceASC->AbilityActorInfo->AvatarActor.Get();
		OutEffectProperties.SourceController = OutEffectProperties.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (OutEffectProperties.SourceController == nullptr && OutEffectProperties.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(OutEffectProperties.SourceAvatarActor))
			{
				OutEffectProperties.SourceController = Pawn->GetController();
			}

		}
		if (OutEffectProperties.SourceController)
		{
			OutEffectProperties.SourceCharacter = Cast<ACharacter>(OutEffectProperties.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{

		OutEffectProperties.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		//
		OutEffectProperties.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		//
		OutEffectProperties.TargetCharacter = Cast<ACharacter>(OutEffectProperties.TargetAvatarActor);
		//
		OutEffectProperties.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OutEffectProperties.TargetAvatarActor);
		
	}
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	// 这个函数在 Gameplay Effect 执行后被调用，可以在这里处理属性变化后的逻辑
	// 例如触发事件、播放特效等。
	FEffectProperties EffectProperties;
	SetEffectProperties(Data, EffectProperties);

	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// 处理 Health 属性变化后的逻辑
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Health changed! New Value: %f"), GetHealth()));
		SetHealth(FMath::Min(GetHealth(), GetMaxHealth()));
	}
	if(Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		// 处理 Mana 属性变化后的逻辑
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Mana changed! New Value: %f"), GetMana()));
		SetMana(FMath::Min(GetMana(), GetMaxMana()));
	}
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}



void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

