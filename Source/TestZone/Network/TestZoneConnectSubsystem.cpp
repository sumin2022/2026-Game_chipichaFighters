// Fill out your copyright notice in the Description page of Project Settings.

#include "Network/TestZoneConnectSubsystem.h"

void UTestZoneConnectSubsystem::Initialize(
    FSubsystemCollectionBase &Collection) {
  Super::Initialize(Collection);
}

void UTestZoneConnectSubsystem::Deinitialize() {
  Super::Deinitialize();
  if (Socket) {
    Socket->Close();
    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
    Socket = nullptr;
  }
}

EConnectResult UTestZoneConnectSubsystem::ConnectToServer(FString IPAddress,
                                                          int32 Port) {
                                                        
  
  TSharedRef<FInternetAddr> ServerAddress = 
      ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

  bool bIsValid;

  ServerAddress->SetIp(IPAddress.GetCharArray().GetData(), bIsValid);
  if (not bIsValid) {
    return EConnectResult::InvalidAddress;
  }

  ServerAddress->SetPort(Port);

  if (Socket) {
    Socket->Close();
    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
    Socket = nullptr;
  }

  Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)
               ->CreateSocket(NAME_Stream, TEXT("ConnectToServer"), false);

  if (!Socket) {
    return EConnectResult::Failure;
  }

  bool Connected = Socket->Connect(*ServerAddress);
  if (Connected) {
    return EConnectResult::Success;
  } else {
    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
    Socket = nullptr;
    return EConnectResult::Failure;
  }
}

void UTestZoneConnectSubsystem::SendLogin(FString const &Username,
                                          FString const &Password) {
  if (!Socket) {
    return;
  }

  CS_Login LoginPacket;
  LoginPacket.size = sizeof(LoginPacket);
  LoginPacket.type = PACKET_TYPE::CS_LOGIN;

  // Copy the username and password into the packet
  FCStringAnsi::Strncpy(LoginPacket.username, TCHAR_TO_ANSI(*Username),
                        sizeof(LoginPacket.username));
  // FCStringAnsi::Strncpy(LoginPacket.password, TCHAR_TO_ANSI(*Password),
  //                       sizeof(LoginPacket.password));

  int32 BytesSent = 0;
  bool bSuccess =
      Socket->Send((uint8 *)&LoginPacket, sizeof(LoginPacket), BytesSent);
  if (!bSuccess || BytesSent != sizeof(LoginPacket)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send login packet"));
  }
}