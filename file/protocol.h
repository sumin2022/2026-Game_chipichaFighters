#include <array>
#pragma once
constexpr short PORT = 9000;
constexpr int WORLD_WIDTH = 400;
constexpr int WORLD_HEIGHT = 400;
constexpr int MAX_PLAYERS = 18;
constexpr int MAX_NAME_LEN = 20;
constexpr int MAX_ROOM_PLAYERS = 6;
constexpr int MAX_ROOM_AI = 6;

enum PACKET_TYPE {
	CS_LOGIN, CS_MOVE, SC_LOGIN_RESULT, SC_AVATAR_INFO,SC_ADD_PLAYER,
	 SC_REMOVE_PLAYER, SC_MOVE_PLAYER, CS_SKILL, SC_CURRENT_STATE, SC_DEATH, 
	 SC_RESPAWN, SC_GAME_RESULT,SC_GAME_START,CS_READY

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
	DIRECTION dir;
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

struct CS_Skill;
struct SC_CurrentState;
struct SC_Death;
struct SC_Respawn;
struct SC_GameResult;
struct SC_GameStart;
struct CS_Ready;


struct Room{
	int room_id = -1; // room ¹øÈ£ : 1, 2, 3 ...? 
	bool active = false;

	std::array<int, MAX_ROOM_PLAYERS> player_ids;
	std::array<int, MAX_ROOM_AI> ai_ids;

	int player_count = 0;
	int ai_count = 0;
};

#pragma pack(pop)