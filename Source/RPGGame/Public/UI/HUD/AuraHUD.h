// Fill out your copyright notice in the Description page of Project Settings.
// 作为 MVVM 架构中的“总导演”和依赖注入容器（Dependency Injection Container）。
//            主要职责包括：
//            1. 实例化上层 View（UAuraUserWidget）和中层 ViewModel（UWidgetController）。
//            2. 统筹并打包底层核心数据（PlayerState, ASC, AttributeSet），将其注入到 Controller 中。
//            3. 完成 View 与 Controller 的绑定牵线。
//            4. 并在整个生命周期内持有这些组件的强引用（TObjectPtr），防止被垃圾回收（GC）销毁。
//
// @note      在调用任何 Init() 函数前，必须确保在蓝图子类中正确配置了对应的 Class 图纸。
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"
class UAuraUserWidget;
class UOverlayWidgetController;
class APlayerController;
class UAbilitySystemComponent;
class UAttributeSet;
struct FWidgetControllerParams;
/**
 * 
 */
UCLASS()
class RPGGAME_API AAuraHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& InParams);

	void InitOverlay(APlayerController* InPlayerController, APlayerState* InPlayerState, UAbilitySystemComponent* InAbilitySystemComponent, UAttributeSet* InAttributeSet);
protected:
	

private:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;
	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
};
