// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
	LastActor = nullptr;
	ThisActor = nullptr;

}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
/*
	FInputModeGameAndUI：这是一种混合模式，允许输入同时响应游戏逻辑（如旋转视角）和 UI 交互。
	SetLockMouseToViewportBehavior：设置鼠标是否锁定在游戏窗口内。DoNotLock 表示鼠标可以移出游戏窗口。
	SetHideCursorDuringCapture：当玩家点击屏幕（Capture）时，是否隐藏光标。设置为 false 保证光标始终可见。
	SetInputMode：将上述所有配置应用到当前的 PlayerController。
*/
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	}

}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if(APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult HitResult;
	// 创建一个结构体来存储光标指向的物体信息

	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult);
	// 从摄像机通过光标位置发出一条射线
	// ECC_Visibility = 检查可见性碰撞通道上的物体
	// false = 不忽略阻挡碰撞（返回第一个阻挡的物体）
	// HitResult = 存储碰撞信息

	LastActor = ThisActor;
	// 将当前的敌人移到"上一个"位置（前一帧的敌人）

	ThisActor = HitResult.bBlockingHit ? Cast<IEnemyInterface>(HitResult.GetActor()) : nullptr;
	// 将击中的敌人转换为 IEnemyInterface 接口类型
	// 如果成功，存储在 ThisActor 中
	// 如果敌人没有实现 IEnemyInterface 接口，ThisActor 就为 nullptr

	if (LastActor != ThisActor)
	{
		if (LastActor)
		{
			LastActor->UnHighlightActor();
		}

		if (ThisActor)
		{
			ThisActor->HighlightActor();
		}
	}
}
