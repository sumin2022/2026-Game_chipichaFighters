// Fill out your copyright notice in the Description page of Project Settings.

#include "TestZoneConnectSubsystem.h"

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

void UTestZoneConnectSubsystem::ReceiveLoop() {
  if (not Socket)
    return;
  int32 BytesRead = -1;

  // RecvEnd 만큼의 데이터를 제외하고 RecvBuffer 에 데이터를 읽어옴
  std::uint8_t *pRecvBuffer = RecvBuffer.data() + RecvEnd;
  int RemainBytes = RecvBuffer.size() - RecvEnd;

  // 소켓이 닫히거나 복구 불가능한 오류가 발생할 경우 false 반환, 넌블럭킹
  bool bNotExceptional = Socket->Recv(pRecvBuffer, RemainBytes, BytesRead,
                                      ESocketReceiveFlags::None);
  if (not bNotExceptional) {
    UE_LOG(LogTemp, Error, TEXT("closed socket or unrecuverable error"));
    return;
  }

  // 온 게 없음.
  if (0 == BytesRead) {
    return;
  } else {
    RecvEnd += BytesRead;
  }

  // 패킷 분석
  while (RecvEnd - RecvStart >= sizeof(TZPacketHeader)) {
    TZPacketHeader *Header =
        reinterpret_cast<TZPacketHeader *>(RecvBuffer.data() + RecvStart);
    if (Header->size > RecvEnd - RecvStart) {
      break;
    }

    switch (Header->type) {
      using enum PACKET_TYPE;
    case SC_LOGIN_RESULT: {
      SC_LoginResult *pRecv = reinterpret_cast<SC_LoginResult *>(Header);
      UE_LOG(LogTemp, Warning, TEXT("Login Result: %d"), pRecv->success);
      OnLoginResult.Broadcast(
          pRecv->success,
          FString::Printf(TEXT("Login Result: %d"), pRecv->success));
      RecvStart += Header->size;
      break;
    }
    case SC_AVATAR_INFO: {
      SC_AvatarInfo *pRecv = reinterpret_cast<SC_AvatarInfo *>(Header);
      OnAvatarInfo.Broadcast(pRecv->playerId, pRecv->x, pRecv->y);
      RecvStart += Header->size;
      break;
    }
    case SC_ADD_PLAYER: {
      SC_AddPlayer *pRecv = reinterpret_cast<SC_AddPlayer *>(Header);
      OnAddPlayer.Broadcast(pRecv->playerId,
                            FString(UTF8_TO_TCHAR(pRecv->username)), pRecv->x,
                            pRecv->y);
      RecvStart += Header->size;
      break;
    }
    case SC_REMOVE_PLAYER: {
      SC_RemovePlayer *pRecv = reinterpret_cast<SC_RemovePlayer *>(Header);
      OnRemovePlayer.Broadcast(pRecv->playerid);
      RecvStart += Header->size;
      break;
    }
    case SC_MOVE_PLAYER: {
      SC_MovePlayer *pRecv = reinterpret_cast<SC_MovePlayer *>(Header);
      OnMovePlayer.Broadcast(pRecv->playerId, pRecv->x, pRecv->y);
      RecvStart += Header->size;
      break;
    }
    case SC_NETPLAYERSTATE: {
      // SC_NETPLAYERSTATE 패킷 구조체가 없으므로 일단 무시
      RecvStart += Header->size;
      break;
    }
    case SC_ROOM_SNAPSHOT: {
      SC_RoomSnapshot *pRecv = reinterpret_cast<SC_RoomSnapshot *>(Header);
      TArray<FNetPlayerState> Players;
      for (int i = 0; i < pRecv->count; ++i) {
        FNetPlayerState State;
        State.PlayerId = pRecv->players[i].player_id;
        State.X = pRecv->players[i].x;
        State.Y = pRecv->players[i].y;
        State.faceX = pRecv->players[i].faceX;
        State.faceY = pRecv->players[i].faceY;
        State.HP = pRecv->players[i].hp;
        State.MaxHP = pRecv->players[i].max_hp;
        State.Alive = pRecv->players[i].alive;
        State.Character =
            static_cast<ECharacterType>(pRecv->players[i].character);
        State.KillCount = pRecv->players[i].kill_count;
        State.DeathCount = pRecv->players[i].death_count;
        State.CurrentTargetId = pRecv->players[i].current_target_id;
        Players.Add(State);
      }
      OnRoomSnapshot.Broadcast(pRecv->count, pRecv->red_score,
                               pRecv->blue_score, pRecv->time_left, Players);
      RecvStart += Header->size;
      break;
    }
    case SC_CURRENT_STATE: {
      SC_CurrentState *pRecv = reinterpret_cast<SC_CurrentState *>(Header);
      OnCurrentState.Broadcast();
      RecvStart += Header->size;
      break;
    }
    case SC_DEATH: {
      SC_Death *pRecv = reinterpret_cast<SC_Death *>(Header);
      OnDeath.Broadcast(pRecv->dead_id, pRecv->killer_id);
      RecvStart += Header->size;
      break;
    }
    case SC_RESPAWN: {
      SC_Respawn *pRecv = reinterpret_cast<SC_Respawn *>(Header);
      OnRespawn.Broadcast(pRecv->player_id, pRecv->x, pRecv->y, pRecv->hp);
      RecvStart += Header->size;
      break;
    }
    case SC_GAME_RESULT: {
      SC_GameResult *pRecv = reinterpret_cast<SC_GameResult *>(Header);
      TArray<int32> PlayerIds, Kills, Deaths;
      for (int i = 0; i < pRecv->player_count; ++i) {
        PlayerIds.Add(pRecv->player_ids[i]);
        Kills.Add(pRecv->kills[i]);
        Deaths.Add(pRecv->deaths[i]);
      }
      OnGameResult.Broadcast(pRecv->red_score, pRecv->blue_score,
                             static_cast<ETeamType>(pRecv->winner_team),
                             pRecv->player_count, PlayerIds, Kills, Deaths);
      RecvStart += Header->size;
      break;
    }
    case SC_GAME_START: {
      SC_GameStart *pRecv = reinterpret_cast<SC_GameStart *>(Header);
      OnGameStart.Broadcast();
      RecvStart += Header->size;
      break;
    }
    case SC_ROOM_ENTER: {
      SC_RoomEnter *pRecv = reinterpret_cast<SC_RoomEnter *>(Header);
      TArray<int32> PlayerIds, Teams;
      for (int i = 0; i < pRecv->player_count; ++i) {
        PlayerIds.Add(pRecv->player_ids[i]);
        Teams.Add(pRecv->teams[i]);
      }
      OnRoomEnter.Broadcast(pRecv->room_id, pRecv->player_count, PlayerIds,
                            Teams);
      RecvStart += Header->size;
      break;
    }
    case SC_CHARACTER_SELECTED: {
      SC_CharacterSelected *pRecv =
          reinterpret_cast<SC_CharacterSelected *>(Header);
      OnCharacterSelect.Broadcast(
          pRecv->player_id, static_cast<ECharacterType>(pRecv->character));
      RecvStart += Header->size;
      break;
    }
    case SC_LOBBY_READY_STATE: {
      SC_LobbyReadyState *pRecv =
          reinterpret_cast<SC_LobbyReadyState *>(Header);
      OnLobbyReadyState.Broadcast(pRecv->player_id, pRecv->ready);
      RecvStart += Header->size;
      break;
    }
    default: {
      UE_LOG(LogTemp, Error, TEXT("Unknown packet type: %d %d"), Header->type,
             Header->size);
      RecvStart += Header->size;
      break;
    }
    }
  }

  if (RecvStart > 0) {
    std::memmove(RecvBuffer.data(), RecvBuffer.data() + RecvStart,
                 RecvEnd - RecvStart);
    RecvEnd -= RecvStart;
    RecvStart = 0;
  }
}

EConnectResult UTestZoneConnectSubsystem::ConnectToServer(FString IPAddress,
                                                          int32 Port) {

  ServerAddress =
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

  if (not Socket) {
    return EConnectResult::Failure;
  }
  
  Socket->SetNoDelay(true);
  bool const Connected = Socket->Connect(*ServerAddress);
  if (Connected) {
    // Socket->SetNonBlocking(true);
    RecvStart = 0;
    RecvEnd = 0;
    return EConnectResult::Success;
  } else {
    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
    Socket = nullptr;
    return EConnectResult::Failure;
  }
}

void UTestZoneConnectSubsystem::SendLogin(FString const &Username,
                                          FString const &Password) {
  if (not Socket) {
    return;
  }

  CS_Login LoginPacket;

  // Copy the username and password into the packet
  FMemory::Memzero(LoginPacket.username, sizeof(LoginPacket.username));
  auto res =
      FCStringAnsi::Strncpy(LoginPacket.username, TCHAR_TO_ANSI(*Username),
                            sizeof(LoginPacket.username));
  LoginPacket.size = 1 + sizeof(TZPacketHeader) + strlen(LoginPacket.username);
  // FCStringAnsi::Strncpy(LoginPacket.password, TCHAR_TO_ANSI(*Password),
  //                       sizeof(LoginPacket.password));

  int32 BytesSent = 0;
  bool const bSuccess =
      Socket->Send((uint8 *)&LoginPacket, LoginPacket.size, BytesSent);
  if (not bSuccess || BytesSent != LoginPacket.size) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send login packet %d %d %d"),
           BytesSent, sizeof(LoginPacket), LoginPacket.size);
    UE_LOG(LogTemp, Error, TEXT("login packet %d %d %hs"), LoginPacket.size,
           LoginPacket.type, LoginPacket.username);
  }

  UE_LOG(LogTemp, Error, TEXT("send login packet %d %d %d %hs"),
         (res - LoginPacket.username), LoginPacket.size, LoginPacket.type,
         LoginPacket.username);
}

void UTestZoneConnectSubsystem::SendMove(float axisX, float axisY) {
  if (not Socket) {
    return;
  }
  CS_Move MovePacket;
  MovePacket.axisX = axisX;
  MovePacket.axisY = axisY;
  int32 BytesSent = 0;
  bool const bSuccess =
      Socket->Send((uint8 *)&MovePacket, sizeof(MovePacket), BytesSent);
  if (not bSuccess || BytesSent != sizeof(MovePacket)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send move packet"));
  }
}
void UTestZoneConnectSubsystem::SendFaceDirection(float axisX, float axisY) {
  if (not Socket) {
    return;
  }
  CS_FaceDir FaceDirectionPacket;
  FaceDirectionPacket.faceX = axisX;
  FaceDirectionPacket.faceY = axisY;
  int32 BytesSent = 0;
  bool const bSuccess = Socket->Send((uint8 *)&FaceDirectionPacket,
                                     sizeof(FaceDirectionPacket), BytesSent);
  if (not bSuccess || BytesSent != sizeof(FaceDirectionPacket)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send face direction packet"));
  }
}
void UTestZoneConnectSubsystem::SendAttack() {
  if (not Socket) {
    return;
  }
  CS_Attack AttackPacket;
  int32 BytesSent = 0;
  bool const bSuccess =
      Socket->Send((uint8 *)&AttackPacket, sizeof(AttackPacket), BytesSent);
  if (not bSuccess || BytesSent != sizeof(AttackPacket)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send attack packet"));
  }
}
void UTestZoneConnectSubsystem::SendSkill(int32 skillId) {
  if (not Socket) {
    return;
  }
  CS_Skill SkillPacket;
  SkillPacket.skillId = skillId;
  int32 BytesSent = 0;
  bool const bSuccess =
      Socket->Send((uint8 *)&SkillPacket, sizeof(SkillPacket), BytesSent);
  if (not bSuccess || BytesSent != sizeof(SkillPacket)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send skill packet"));
  }
}
void UTestZoneConnectSubsystem::SendReady() {
  if (not Socket) {
    return;
  }
  CS_Ready ReadyPacket;
  int32 BytesSent = 0;
  bool const bSuccess =
      Socket->Send((uint8 *)&ReadyPacket, sizeof(ReadyPacket), BytesSent);
  if (not bSuccess || BytesSent != sizeof(ReadyPacket)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send ready packet"));
  }
}
void UTestZoneConnectSubsystem::SendSelectCharacter(
    ECharacterType characterId) {
  if (not Socket) {
    return;
  }
  CS_SelectCharacter SelectCharacterPacket;
  int const temp = static_cast<int>(characterId);
  SelectCharacterPacket.character = static_cast<CharacterType>(temp);
  int32 BytesSent = 0;
  bool const bSuccess = Socket->Send((uint8 *)&SelectCharacterPacket,
                                     sizeof(SelectCharacterPacket), BytesSent);
  if (not bSuccess || BytesSent != sizeof(SelectCharacterPacket)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send select character packet"));
  }
}
void UTestZoneConnectSubsystem::SendGameReady(bool ready) {
  if (not Socket) {
    return;
  }
  CS_GameReady GameReadyPacket;
  GameReadyPacket.ready = ready;
  int32 BytesSent = 0;
  bool const bSuccess = Socket->Send((uint8 *)&GameReadyPacket,
                                     sizeof(GameReadyPacket), BytesSent);
  if (not bSuccess || BytesSent != sizeof(GameReadyPacket)) {
    UE_LOG(LogTemp, Error, TEXT("Failed to send game ready packet"));
  }
}
