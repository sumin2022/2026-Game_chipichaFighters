// Copyright Epic Games, Inc. All Rights Reserved.

#include "StrategyPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "StrategyUnit.h"

AStrategyPawn::AStrategyPawn() {
  PrimaryActorTick.bCanEverTick = true;

  // create the root
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

  // create the camera
  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(RootComponent);

  // create the movement component
  FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(
      TEXT("Floating Pawn Movement"));

  // configure the camera
  Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
  Camera->OrthoWidth = 1500.0f;
  Camera->AutoPlaneShift = 1.0f;
  Camera->bUpdateOrthoPlanes = false;

  // configure the movement comp
  FloatingPawnMovement->bConstrainToPlane = true;
  FloatingPawnMovement->SetPlaneConstraintNormal(FVector::UpVector);
  FloatingPawnMovement->SetPlaneConstraintOrigin(FVector::UpVector * 1500.0f);

  num = 0;
}

void AStrategyPawn::SetZoomModifier(float Value) {
  // set the ortho width on the camera
  Camera->SetOrthoWidth(Value);
  UWorld* World = GetWorld();
  if (!IsValid(World)) return;
  if (!SpawnActor) {
    UE_LOG(LogBlueprint, Warning, TEXT("SpawnActor 유효하지 않음"))
    return;
  }

  FActorSpawnParameters SpawnParams{};

  FString huh{TEXT("뭐임_")};
  huh.Append(FString::FromInt(num));

  SpawnParams.Name = FName{huh};
  SpawnParams.Name.SetNumber(++num);
  AStrategyUnit* SpawnedActor = World->SpawnActor<AStrategyUnit>(
      SpawnActor, {1154.0, 1600.0, 450.0}, {}, SpawnParams);
  if (!IsValid(SpawnedActor)) {
    UE_LOG(LogBlueprint, Warning, TEXT("SpawnActor 스폰 실패"))
  }
}
