
#include <array>
// commit id: 8069d4057ee8bd410ecb69666c4da4349b95222d
// 수정함.

constexpr short PORT = 9000;
constexpr int WORLD_WIDTH = 400;
constexpr int WORLD_HEIGHT = 400;
constexpr int MAX_PLAYERS = 18;
constexpr int MAX_NAME_LEN = 20;
constexpr int MAX_ROOM_AI = 6;
constexpr int MAX_ROOM_PLAYERS = 6; // 원래 6명

enum struct PACKET_TYPE : std::uint8_t {
  CS_LOGIN,
  CS_MOVE,
  CS_ATTACK,
  CS_SKILL,
  CS_READY,

  CS_SELECT_CHARACTER,
  CS_GAME_READY,
  CS_FACE_DIR,

  SC_LOGIN_RESULT,
  SC_AVATAR_INFO,
  SC_ADD_PLAYER,
  SC_REMOVE_PLAYER,
  SC_MOVE_PLAYER,
  SC_NETPLAYERSTATE,
  SC_DEATH,
  SC_RESPAWN,
  SC_GAME_RESULT,
  SC_GAME_START,
  SC_ROOM_SNAPSHOT,

  SC_ROOM_ENTER,
  SC_CHARACTER_SELECTED,
  SC_LOBBY_READY_STATE,

  SC_CURRENT_STATE,
  None,
};

enum struct CharacterType : std::uint8_t {
  CHAR_NONE = 0,
  CHAR_DEALER,
  CHAR_ARCHER,
  CHAR_TANKER,
  CHAR_HEALER
};

enum struct TeamType : std::int32_t {
  TEAM_NONE = 0,
  TEAM_RED = 1,
  TEAM_BLUE = 2
};

#pragma pack(push, 1)

template <PACKET_TYPE Type, typename Derived> struct TZPacket {
  std::uint8_t size = sizeof(Derived);
  PACKET_TYPE type = Type;
};

struct TZPacketHeader : TZPacket<PACKET_TYPE::None, TZPacketHeader> {};

struct CS_Login : TZPacket<PACKET_TYPE::CS_LOGIN, CS_Login> {
  char username[MAX_NAME_LEN];
};

struct CS_Move : TZPacket<PACKET_TYPE::CS_MOVE, CS_Move> {
  float axisX; // -1.0 ~ 1.0
  float axisY; // -1.0 ~ 1.0
};

struct CS_FaceDir : TZPacket<PACKET_TYPE::CS_FACE_DIR, CS_FaceDir> {
  float faceX;
  float faceY;
};

struct SC_LoginResult : TZPacket<PACKET_TYPE::SC_LOGIN_RESULT, SC_LoginResult> {
  bool success;
  char message[50];
};

struct SC_AddPlayer : TZPacket<PACKET_TYPE::SC_ADD_PLAYER, SC_AddPlayer> {
  int playerId;
  char username[MAX_NAME_LEN];
  short x;
  short y;
};

struct SC_RemovePlayer
    : TZPacket<PACKET_TYPE::SC_REMOVE_PLAYER, SC_RemovePlayer> {
  int playerid;
};

struct SC_MovePlayer : TZPacket<PACKET_TYPE::SC_MOVE_PLAYER, SC_MovePlayer> {
  int playerId;
  short x;
  short y;
};

struct SC_AvatarInfo : TZPacket<PACKET_TYPE::SC_AVATAR_INFO, SC_AvatarInfo> {
  int playerId;
  short x;
  short y;
};

// 공격 방향은 플레이어의 faceX, faceY로 대체하기로 함
struct CS_Attack : TZPacket<PACKET_TYPE::CS_ATTACK, CS_Attack> {
  // float aimX; // 공격 방향 x
  // float aimY; // 공격 방향 y
};

struct CS_Skill : TZPacket<PACKET_TYPE::CS_SKILL, CS_Skill> {
  short skillId;
  // float aimX;
  // float aimY;
};
//-------------------------------

struct NetPlayerState {
  int player_id;

  float x;
  float y;

  float faceX;
  float faceY;

  int hp;
  int max_hp;

  bool alive;
  CharacterType character;

  int kill_count; // 킬뎃 실시간 적용?
  int death_count;

  int current_target_id = -1; // 클라 표시용
};

struct SC_RoomSnapshot
    : TZPacket<PACKET_TYPE::SC_ROOM_SNAPSHOT, SC_RoomSnapshot> { // 방전체 용
  int count;
  int red_score;
  int blue_score;
  float time_left;
  NetPlayerState players[MAX_ROOM_AI];
};

struct SC_CurrentState
    : TZPacket<PACKET_TYPE::SC_CURRENT_STATE, SC_CurrentState> { // 개인용
};

struct SC_Death : TZPacket<PACKET_TYPE::SC_DEATH, SC_Death> {
  int dead_id;
  int killer_id;
};

struct SC_Respawn : TZPacket<PACKET_TYPE::SC_RESPAWN, SC_Respawn> {
  int player_id;
  float x;
  float y;
  int hp;
};

struct SC_GameResult : TZPacket<PACKET_TYPE::SC_GAME_RESULT, SC_GameResult> {
  int red_score;
  int blue_score;
  TeamType winner_team;

  int player_count; // 테스트 용 1~6명 변경시 테스팅에 사용
  int player_ids[MAX_ROOM_PLAYERS];
  int kills[MAX_ROOM_PLAYERS];
  int deaths[MAX_ROOM_PLAYERS];
};

struct SC_GameStart : TZPacket<PACKET_TYPE::SC_GAME_START, SC_GameStart> {};

struct CS_Ready : TZPacket<PACKET_TYPE::CS_READY, CS_Ready> {};

struct CS_SelectCharacter
    : TZPacket<PACKET_TYPE::CS_SELECT_CHARACTER, CS_SelectCharacter> {
  CharacterType character;
};

struct CS_GameReady : TZPacket<PACKET_TYPE::CS_GAME_READY, CS_GameReady> {
  bool ready;
};

struct SC_RoomEnter : TZPacket<PACKET_TYPE::SC_ROOM_ENTER, SC_RoomEnter> {
  int room_id;
  int player_count;
  int player_ids[MAX_ROOM_PLAYERS];
  int teams[MAX_ROOM_PLAYERS];
};

struct SC_CharacterSelected
    : TZPacket<PACKET_TYPE::SC_CHARACTER_SELECTED, SC_CharacterSelected> {
  int player_id;
  CharacterType character;
};

struct SC_LobbyReadyState
    : TZPacket<PACKET_TYPE::SC_LOBBY_READY_STATE, SC_LobbyReadyState> {
  int player_id;
  bool ready;
};

#pragma pack(pop)