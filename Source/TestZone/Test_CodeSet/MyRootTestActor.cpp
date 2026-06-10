// Fill out your copyright notice in the Description page of Project Settings.

#include "Test_CodeSet/MyRootTestActor.h"

// Sets default values
AMyRootTestActor::AMyRootTestActor() {
  // Set this actor to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  USceneComponent* DefaultSceneRoot =
      CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
  RootComponent = DefaultSceneRoot;

  UStaticMeshComponent* StaticMeshComponent =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
  StaticMeshComponent->SetupAttachment(DefaultSceneRoot);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(
      TEXT("/Game/LevelPrototyping/Meshes/SM_Ramp.SM_Ramp"));

  if (StaticMeshAsset.Succeeded()) {
    StaticMeshComponent->SetStaticMesh(StaticMeshAsset.Object);
    UE_LOG(LogTemp, Log, TEXT("Successfully loaded static mesh asset!"));
  } else {
    UE_LOG(LogTemp, Warning, TEXT("Failed to load static mesh asset!"));
  }
}

// Called when the game starts or when spawned
void AMyRootTestActor::BeginPlay() { Super::BeginPlay(); }

// Called every frame
void AMyRootTestActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }
