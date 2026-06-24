// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TestZoneCharacter.h"
#include "BattlePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
/**
 *
 */
UCLASS(Blueprintable)
class TESTZONE_API ABattlePlayerController : public APlayerController {
  GENERATED_BODY()

 public:
  UPROPERTY(EditAnywhere, Category = "Charater")
  TObjectPtr<ATestZoneCharacter> ControlledPawn;

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputMappingContext* CurrentMappingContext;

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction* InputMove;

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction* InputAttack;

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction* InputSkill;

  UPROPERTY(EditAnywhere, Category = "Input")
  UInputAction* InputReady;

  virtual void OnPossess(APawn *InPawn) override;

 protected:
};
