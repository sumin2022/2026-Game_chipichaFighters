// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MySimpleHUD.generated.h"

class SMySimpleWidget;

/**
 *
 */
UCLASS()
class TESTZONE_API AMySimpleHUD : public AHUD {
  GENERATED_BODY()

 protected:
  /** Pointer to the UI user widget */
 
  TSharedPtr<SMySimpleWidget> SWidget;

 public:
  void BeginPlay() override;

 protected:
  virtual void DrawHUD() override;
};
