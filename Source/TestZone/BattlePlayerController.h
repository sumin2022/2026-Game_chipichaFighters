// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BattlePlayerController.generated.h"
class UInputMappingContext;
class UInputAction;
/**
 * 
 */
UCLASS(Blueprintable)
class TESTZONE_API ABattlePlayerController : public APlayerController
{
	GENERATED_BODY()
 protected:

  UPROPERTY(EditAnywhere, Category = "Input")
  TObjectPtr<UInputMappingContext> SetMappingContext;

};
