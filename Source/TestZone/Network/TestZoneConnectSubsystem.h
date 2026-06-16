// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "TestZoneConnectSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class TESTZONE_API UTestZoneConnectSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
          ConnectToServer(FIPv4Address IPAddress, int32 Port);

	protected:
          FSocket* Socket;
};
