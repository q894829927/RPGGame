// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"
#include "AbilitySystem/AuraAbilitySystemComponentBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerState.h"
#include "Player/AuraPlayerController.h"
#include "UI/HUD/AuraHUD.h"
AAuraCharacter::AAuraCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AAuraCharacter::InitAbilityActorInfo()
{
	// Initialize the AbilitySystemComponent and AttributeSet when the character is possessed by a controller (server only)
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	if (!AuraPlayerState)
	{
		return;
	}

	UAbilitySystemComponent* AuraAbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	if (!AuraAbilitySystemComponent)
	{
		return;
	}

	AuraAbilitySystemComponent->InitAbilityActorInfo(AuraPlayerState, this);
	/*
	* InitAbilityActorInfo(OwnerActor, AvatarActor)
	* OwnerActor = PlayerState：逻辑所有者（拥有 ASC）
	* AvatarActor = this（Character）：物理表现（执行动作）
	*/
	AbilitySystemComponent = AuraAbilitySystemComponent;
	AttributeSet = AuraPlayerState->GetAttributeSet();

	AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController());

	if (AuraPlayerController) 
	{
		AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD());
		if (AuraHUD)
		{	
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AuraAbilitySystemComponent, AttributeSet);
		}
	}
}
// Called when the character is possessed by a controller (server only)
void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	//Init ability actor info for the server
	InitAbilityActorInfo();

}

// Called on clients when the PlayerState is replicated
void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	//Init ability actor info for the client
	InitAbilityActorInfo();

}
