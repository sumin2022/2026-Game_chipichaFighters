// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameProtocol.h"
#include "IPAddress.h"
#include "Networking.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TestZoneConnectSubsystem.generated.h"

UENUM(BlueprintType)
enum struct EConnectResult : uint8 {
  Success,
  Failure,
  Timeout,
  InvalidAddress
};

UENUM(BlueprintType)
enum struct ECharacterType : uint8 { None, Dealer, Archer, Tanker, Healer };

UENUM(BlueprintType)
enum struct ETeamType : uint8 { None, Red, Blue };

USTRUCT(BlueprintType)
struct FNetPlayerState {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly)
  int32 PlayerId = -1;

  UPROPERTY(BlueprintReadOnly)
  float X = 0.0f;

  UPROPERTY(BlueprintReadOnly)
  float Y = 0.0f;

  UPROPERTY(BlueprintReadOnly)
  float faceX = 0.0f;

  UPROPERTY(BlueprintReadOnly)
  float faceY = 0.0f;

  UPROPERTY(BlueprintReadOnly)
  int32 HP = -1;

  UPROPERTY(BlueprintReadOnly)
  int32 MaxHP = -1;

  UPROPERTY(BlueprintReadOnly)
  bool Alive = false;

  UPROPERTY(BlueprintReadOnly)
  ECharacterType Character = ECharacterType::None;

  UPROPERTY(BlueprintReadOnly)
  int32 KillCount = 0;

  UPROPERTY(BlueprintReadOnly)
  int32 DeathCount = 0;

  UPROPERTY(BlueprintReadOnly)
  int32 CurrentTargetId = -1;
};

USTRUCT(BlueprintType)
struct FEntryRoomPlayerInfo {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 PlayerId = -1;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bIsReady = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  ECharacterType Character = ECharacterType::None;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  ETeamType Team = ETeamType::None;
};

USTRUCT(BlueprintType)
struct FLoginInfo {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString IPAddress;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Port;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Username;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Password;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTZOnDisconnected,  //
                                             EConnectResult, EDisconnectResult,
                                             FString const &, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginResult,  //
                                             bool, bSuccess,  //
                                             FString const &, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAddPlayer,               //
                                              int32, PlayerId,            //
                                              FString const &, username,  //
                                              int32, x, int32, y);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemovePlayer,  //
                                            int32, PlayerId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMovePlayer,    //
                                               int32, PlayerId,  //
                                               int32, axisX, int32, axisY);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAvatarInfo,  //
                                               int32, PlayerId, int32, axisX,
                                               int32, axisY);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnRoomSnapshot,    //
                                              int32, count,       //
                                              int32, red_score,   //
                                              int32, blue_score,  //
                                              float, time_left,   //
                                              TArray<FNetPlayerState> const &,
                                              players);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurrentState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath,  //
                                             int32, dead_id, int32, killer_id);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRespawn,          //
                                              int32, player_id,    //
                                              float, x, float, y,  //
                                              int32, hp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(
    FOnGameResult,                                                //
    int32, red_score, int32, blue_score, ETeamType, winner_team,  //
    int32, player_count, TArray<int32> const &, player_ids,       //
    TArray<int32> const &, player_kills, TArray<int32> const &, player_deaths);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStart);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRoomEnter,    //
                                               int32, room_id,  //
                                               TArray<int32> const &,
                                               player_ids,  //
                                               TArray<ETeamType> const &,
                                               teams);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterSelected,  //
                                             int32, player_id,      //
                                             ECharacterType, character_id);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbyReadyState,  //
                                             int32, player_id, bool, ready);

/**
 *
 */
UCLASS(BlueprintType)
class TESTZONE_API UTestZoneConnectSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()

  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;

 public:
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  EConnectResult ConnectToServer(FString IPAddress, int32 Port);

  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendLogin(FString const &Username, FString const &Password);

  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendMove(float axisX, float axisY);
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendFaceDirection(float axisX, float axisY);
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendAttack();
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendSkill(int32 skillId);
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendReady();

  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendSelectCharacter(ECharacterType characterId);
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendGameReady(bool ready);

  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SetEntryRoomState(TArray<FEntryRoomPlayerInfo> const &param_state);

  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  TArray<FEntryRoomPlayerInfo> const &GetEntryRoomState();

  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SetEntryRoomId(int32 id) { RoomId = id; }


  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  int32 GetEntryRoomId() const {return RoomId; }

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FTZOnDisconnected TZOnDisconnected;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnLoginResult OnLoginResult;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnAddPlayer OnAddPlayer;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnRemovePlayer OnRemovePlayer;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnMovePlayer OnMovePlayer;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnAvatarInfo OnAvatarInfo;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnRoomSnapshot OnRoomSnapshot;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnCurrentState OnCurrentState;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnDeath OnDeath;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnRespawn OnRespawn;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnGameResult OnGameResult;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnGameStart OnGameStart;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnRoomEnter OnRoomEnter;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnCharacterSelected OnCharacterSelect;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnLobbyReadyState OnLobbyReadyState;

  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void ReceiveLoop();

 protected:
  TSharedPtr<FInternetAddr> ServerAddress;
  FSocket *Socket;
  std::array<uint8, 4096> RecvBuffer;
  size_t RecvStart;
  size_t RecvEnd;
  int32 RoomId;
  TArray<FEntryRoomPlayerInfo> EntryRoomState;
  FLoginInfo PreviousLoginInfo;

};
