// Fill out your copyright notice in the Description page of Project Settings.

#include "BattlePlayerController.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"

void ABattlePlayerController::OnPossess(APawn *InPawn) {
  Super::OnPossess(InPawn);

  if (not IsLocalPlayerController())
    return;

  UEnhancedInputLocalPlayerSubsystem *Subsystem =
      ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
          GetLocalPlayer());

  if (not IsValid(Subsystem)) {
    UE_LOG(LogTemp, Error, TEXT("Subsystem is not valid"));
    return;
  } else {
    UE_LOG(LogTemp, Display, TEXT("Subsystem is valid"));
  }

  auto *Chara = Cast<ATestZoneCharacter>(InPawn);

  if (not IsValid(Chara)) {
    UE_LOG(LogTemp, Error, TEXT("Chara is not valid"));
    return;
  } else {
    UE_LOG(LogTemp, Display, TEXT("Chara is valid"));
  }

  ControlledPawn = Chara;
}