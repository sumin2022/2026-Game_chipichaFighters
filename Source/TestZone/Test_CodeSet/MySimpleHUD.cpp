// Fill out your copyright notice in the Description page of Project Settings.

#include "Test_CodeSet/MySimpleHUD.h"

#include "Engine/Engine.h"
#include "Test_CodeSet/MySimpleWidget.h"

void AMySimpleHUD::BeginPlay() {
  Super::BeginPlay();

  if (!GEngine) return;
  if (!GEngine->GameViewport) return;

  SWidget = SNew(SMySimpleWidget);
  if (!SWidget.IsValid()) {
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                                     TEXT("Failed to create SMySimpleWidget!"));
    return;
  } else {
    SWidget->SetVisibility(EVisibility::Visible);
  }
  GEngine->GameViewport->AddViewportWidgetContent(SWidget.ToSharedRef(), 1000);
}

void AMySimpleHUD::DrawHUD() {
  Super::DrawHUD();
  // Draw some debug text to show that the HUD is working
  DrawText(TEXT("Hello from MySimpleHUD!"), FColor::Green, 50, 50, nullptr,
           1.5f);
}