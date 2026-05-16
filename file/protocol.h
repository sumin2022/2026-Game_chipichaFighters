#include <array>
#pragma once
constexpr short PORT = 9000;
constexpr int WORLD_WIDTH = 400;
constexpr int WORLD_HEIGHT = 400;
constexpr int MAX_PLAYERS = 18;
constexpr int MAX_NAME_LEN = 20;
constexpr int MAX_ROOM_AI = 6;

enum PACKET_TYPE : unsigned char {
	CS_LOGIN, CS_MOVE, CS_ATTACK, CS_SKILL, CS_READY, 

	CS_SELECT_CHARACTER,
	CS_GAME_READY,

	SC_LOGIN_RESULT, SC_AVATAR_INFO,SC_ADD_PLAYER,
	 SC_REMOVE_PLAYER, SC_MOVE_PLAYER,  SC_CURRENT_STATE, SC_DEATH, 
	 SC_RESPAWN, SC_GAME_RESULT,SC_GAME_START,

	 SC_ROOM_ENTER,
	 SC_CHARACTER_SELECTED,
	 SC_LOBBY_READY_STATE

};
enum DIRECTION {UP,DOWN,LEFT,RIGHT};

#pragma pack(push, 1)

struct CS_Login {
	unsigned char size;
	PACKET_TYPE type;
	char username[MAX_NAME_LEN];
};

struct CS_Move {
	unsigned char size;
	PACKET_TYPE type;
	float axisX;   // -1.0 ~ 1.0
	float axisY;   // -1.0 ~ 1.0
};

struct SC_LoginResult {
	unsigned char size;
	PACKET_TYPE type;
	bool success;
	char message[50];
};

struct SC_AddPlayer {
	unsigned char size;
	PACKET_TYPE   type;
	int playerId;
	char username[MAX_NAME_LEN];
	short x;
	short y;
};

struct SC_RemovePlayer {
	unsigned char size;
	PACKET_TYPE type;
	int playerid;
};

struct SC_MovePlayer {
	unsigned char size;
	PACKET_TYPE   type;
	int playerId;
	short x;
	short y;
};

struct SC_AvatarInfo {
	unsigned char size;
	PACKET_TYPE   type;
	int playerId;
	short x;
	short y;
};

struct CS_Attack {
	unsigned char size;
	PACKET_TYPE type;

	float aimX; // 공격 방향 x
	float aimY; // 공격 방향 y
};

struct CS_Skill {
	unsigned char size;
	PACKET_TYPE type;

	short skillId;
	float aimX;
	float aimY;
};

struct SC_CurrentState {
	unsigned char size;
	PACKET_TYPE type;
};

struct SC_Death {
	unsigned char size;
	PACKET_TYPE type;
};

struct SC_Respawn {
	unsigned char size;
	PACKET_TYPE type;
};

struct SC_GameResult {
	unsigned char size;
	PACKET_TYPE type;
};

struct SC_GameStart {
	unsigned char size;
	PACKET_TYPE type;
};

struct CS_Ready {
	unsigned char size;
	PACKET_TYPE type;
};

enum CharacterType : unsigned char {
	CHAR_NONE = 0,
	CHAR_DEALER,
	CHAR_ARCHER,
	CHAR_TANKER,
	CHAR_HEALER
};

struct CS_SelectCharacter {
	unsigned char size;
	PACKET_TYPE type;
	CharacterType character;
};

struct CS_GameReady {
	unsigned char size;
	PACKET_TYPE type;
	bool ready;
};

struct SC_RoomEnter {
	unsigned char size;
	PACKET_TYPE type;
	int room_id;
	int player_count;
};

struct SC_CharacterSelected {
	unsigned char size;
	PACKET_TYPE type;
	int player_id;
	CharacterType character;
};

struct SC_LobbyReadyState {
	unsigned char size;
	PACKET_TYPE type;
	int player_id;
	bool ready;
};

#pragma pack(pop)