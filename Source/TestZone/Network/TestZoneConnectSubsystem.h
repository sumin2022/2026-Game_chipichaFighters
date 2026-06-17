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
enum struct EConnectResult : uint8 { Success, Failure, Timeout, InvalidAddress };

UENUM(BlueprintType)
enum struct ECharacterType : uint8 { None, Dealer, Archer, Tanker, Healer };

UENUM(BlueprintType)
enum struct ETeamType : uint8 { None, Red, Blue };

USTRUCT(BlueprintType)
struct FNetPlayerState {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly)
  int32 PlayerId;

  UPROPERTY(BlueprintReadOnly)
  float X;

  UPROPERTY(BlueprintReadOnly)
  float Y;

  UPROPERTY(BlueprintReadOnly)
  float faceX;

  UPROPERTY(BlueprintReadOnly)
  float faceY;

  UPROPERTY(BlueprintReadOnly)
  int32 HP;

  UPROPERTY(BlueprintReadOnly)
  int32 MaxHP;

  UPROPERTY(BlueprintReadOnly)
  bool Alive;

  UPROPERTY(BlueprintReadOnly)
  ECharacterType Character;

  UPROPERTY(BlueprintReadOnly)
  int32 KillCount;

  UPROPERTY(BlueprintReadOnly)
  int32 DeathCount;

  UPROPERTY(BlueprintReadOnly)
  int32 CurrentTargetId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginResult, //
                                             bool, bSuccess, FString const &,
                                             Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAddPlayer, //
                                              int32, PlayerId, FString const &,
                                              username, int32, x, int32, y);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemovePlayer, //
                                            int32, PlayerId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMovePlayer, //
                                               int32, PlayerId, int32, axisX,
                                               int32, axisY);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAvatarInfo, //
                                               int32, PlayerId, int32, axisX,
                                               int32, axisY);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnRoomSnapshot, //
                                              int32, count, int32, red_score,
                                              int32, blue_score, float,
                                              time_left,
                                              TArray<FNetPlayerState> const &,
                                              players);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurrentState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath, //
                                             int32, dead_id, int32, killer_id);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRespawn, //
                                              int32, player_id, float, x, float,
                                              y, int32, hp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(
    FOnGameResult, //
    int32, red_score, int32, blue_score, ETeamType, winner_team, int32,
    player_count, TArray<int32> const &, player_ids, TArray<int32> const &,
    player_kills, TArray<int32> const &, player_deaths);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStart);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRoomEnter, //
                                              int32, room_id, int32,
                                              player_count,
                                              TArray<int32> const &, player_ids,
                                              TArray<int32> const &, teams);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterSelected, //
                                             int32, player_id, ECharacterType,
                                             character_id);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbyReadyState, //
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
  void SendMove(float axisX, float axisY){}
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendFaceDirection(float axisX, float axisY){}
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendAttack(){}
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendSkill(int32 skillId){}
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendReady(){}

  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendSelectCharacter(ECharacterType characterId){}
  UFUNCTION(BlueprintCallable, Category = "TestZone|Network")
  void SendGameReady(bool ready){}

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnLoginResult OnLoginResult;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnAddPlayer OnAddPlayer;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnRemovePlayer OnRemovePlayer;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnCurrentState OnCurrentState;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnDeath OnDeath;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnRespawn OnRespawn;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnCharacterSelected OnCharacterSelect;

  UPROPERTY(BlueprintAssignable, Category = "TestZone|Network")
  FOnLobbyReadyState OnLobbyReadyState;

protected:
  FSocket *Socket;
};
