
#include <array>
// commit id: 0df2186e36ca3f9e8fd083b3dde7680b787811cc
// https://github.com/sumin2022/2026-Game_chipichaFighters/commit/0df2186e36ca3f9e8fd083b3dde7680b787811cc
// 수정함.

constexpr short PORT = 9000;
constexpr int WORLD_WIDTH = 400;
constexpr int WORLD_HEIGHT = 400;
constexpr int MAX_PLAYERS = 18;
constexpr int MAX_NAME_LEN = 20;
constexpr int MAX_ROOM_AI = 6;
constexpr int MAX_ROOM_PLAYERS = 6; // 원래 6명

enum struct PACKET_STAGE : std::uint8_t { TITLE, MATCH, BATTLE, RESULT, NONE };

enum struct PACKET_TYPE : std::uint8_t {
  // 클라이언트에서 서버에게 3초마다 패킷 전송, 서버는 받은 패킷에 응답 처리,
  // 10초 이상 응답 없을시 연결 끊김 처리
  // default
  CS_CONNECTION_CHECK, // 클라이언트에서 서버로 연결 확인 요청
  SC_CONNECTION_CHECK, // 서버에서 클라이언트로 연결 확인 응답

  // title
  CS_LOGIN,
  CS_READY,
  SC_LOGIN_RESULT,
  SC_AVATAR_INFO,

  // match
  SC_ROOM_ENTER,
  CS_SELECT_CHARACTER,
  SC_CHARACTER_SELECTED,
  CS_GAME_READY,
  SC_LOBBY_READY_STATE,
  SC_GAME_START,

  // battle
  CS_MOVE,
  CS_FACE_DIR,
  CS_ATTACK,
  CS_SKILL,
  SC_ADD_PLAYER,
  SC_REMOVE_PLAYER,
  SC_MOVE_PLAYER,
  SC_NETPLAYERSTATE,
  SC_ROOM_SNAPSHOT,
  SC_CURRENT_STATE,
  SC_DEATH,
  SC_RESPAWN,
  SC_ITEM_STATE,

  // result
  SC_GAME_RESULT,

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
  std::uint16_t size = sizeof(Derived);
  PACKET_TYPE type = Type;
};

template <PACKET_STAGE Stage, PACKET_TYPE Type, typename Derived>
struct ExamplePacket {
  std::uint16_t size{sizeof(Derived)};
  PACKET_STAGE stage{Stage};
  PACKET_TYPE type{Type};
};

struct TZPacketHeader : TZPacket<PACKET_TYPE::None, TZPacketHeader> {};

// default - 기본 연결 확인
struct CS_ConnectionCheck
    : TZPacket<PACKET_TYPE::CS_CONNECTION_CHECK, CS_ConnectionCheck> {};
// 클라이언트에서 서버로 연결 확인 요청

struct SC_ConnectionCheck
    : TZPacket<PACKET_TYPE::SC_CONNECTION_CHECK, SC_ConnectionCheck> {};
// 클라이언트에서 서버로 연결 확인 응답

// title - 로그인 요청
struct CS_Login : TZPacket<PACKET_TYPE::CS_LOGIN, CS_Login> {
  char username[MAX_NAME_LEN]{};
};

// title - 클라이언트 닉네임 준비 요청
struct CS_Ready : TZPacket<PACKET_TYPE::CS_READY, CS_Ready> {};

// title - 로그인 성공/실패 처리
struct SC_LoginResult : TZPacket<PACKET_TYPE::SC_LOGIN_RESULT, SC_LoginResult> {
  bool success{};
  char message[50]{};
};

// title - 로그인 후 플레이어 기본 정보 (현재는 전부 단순 전달)
struct SC_AvatarInfo : TZPacket<PACKET_TYPE::SC_AVATAR_INFO, SC_AvatarInfo> {
  int playerId{};
  short x{};
  short y{};
};

//==============================================================================================

// match - 한 게임 중 각 플레이어 상태 정보
struct SC_RoomEnter : TZPacket<PACKET_TYPE::SC_ROOM_ENTER, SC_RoomEnter> {
  int room_id{};
  int player_count{};
  int player_ids[MAX_ROOM_PLAYERS]{};
  int teams[MAX_ROOM_PLAYERS]{};
};

// match - 캐릭터 선택 요청
struct CS_SelectCharacter
    : TZPacket<PACKET_TYPE::CS_SELECT_CHARACTER, CS_SelectCharacter> {
  CharacterType character{CharacterType::CHAR_NONE};
};

// match - 특정 플레이어의 캐릭터 선택 알림
struct SC_CharacterSelected
    : TZPacket<PACKET_TYPE::SC_CHARACTER_SELECTED, SC_CharacterSelected> {
  int player_id{};
  CharacterType character{CharacterType::CHAR_NONE};
};

// match - 게임 준비 요청
struct CS_GameReady : TZPacket<PACKET_TYPE::CS_GAME_READY, CS_GameReady> {
  bool ready{};
};

// match - 매치 방에서 다른 플레이어의 준비 상태 알림
struct SC_LobbyReadyState
    : TZPacket<PACKET_TYPE::SC_LOBBY_READY_STATE, SC_LobbyReadyState> {
  int player_id{};
  bool ready{};
};

// match - 라운드 준비 및 시작 완료 알림
struct SC_GameStart : TZPacket<PACKET_TYPE::SC_GAME_START, SC_GameStart> {};

//==============================================================================================

// battle - 이동 입력값 전송
struct CS_Move : TZPacket<PACKET_TYPE::CS_MOVE, CS_Move> {
  float axisX{};
  float axisY{};
};

// battle - 바라보는 방향 전송
struct CS_FaceDir : TZPacket<PACKET_TYPE::CS_FACE_DIR, CS_FaceDir> {
  float faceX{};
  float faceY{};
};

// battle - 기본 공격 요청
struct CS_Attack : TZPacket<PACKET_TYPE::CS_ATTACK, CS_Attack> {};

// battle - 스킬 사용 요청
struct CS_Skill : TZPacket<PACKET_TYPE::CS_SKILL, CS_Skill> {
  short skillId{};
};

//==============================================================================================
// *필요한 패킷만 추가, 나중에 정리할 것*
// battle - 전투 중 플레이어 추가 알림 (스폰/리스폰 시)
struct SC_AddPlayer : TZPacket<PACKET_TYPE::SC_ADD_PLAYER, SC_AddPlayer> {
  int playerId{};
  char username[MAX_NAME_LEN]{};
  short x{};
  short y{};
};

// battle - 전투 중 플레이어 제거 알림 (사망 처리)
struct SC_RemovePlayer
    : TZPacket<PACKET_TYPE::SC_REMOVE_PLAYER, SC_RemovePlayer> {
  int playerid{};
};
//==============================================================================================

// battle - 특정 플레이어 이동 위치 알림
struct SC_MovePlayer : TZPacket<PACKET_TYPE::SC_MOVE_PLAYER, SC_MovePlayer> {
  int playerId{};
  short x{};
  short y{};
};

// battle - 게임에 포함되는 플레이어 상태 정보
struct NetPlayerState {
  int player_id{};

  float x{};
  float y{};

  float faceX{};
  float faceY{};

  int hp{};
  int max_hp{};

  bool alive{};
  CharacterType character{CharacterType::CHAR_NONE};

  int kill_count{};
  int death_count{};

  int current_target_id{-1};
};

// battle - 한 라운드의 전체 상태 스냅샷
struct SC_RoomSnapshot
    : TZPacket<PACKET_TYPE::SC_ROOM_SNAPSHOT, SC_RoomSnapshot> {
  int count{};
  int red_score{};
  int blue_score{};
  float time_left{};
  NetPlayerState players[MAX_ROOM_AI]{};
};

// battle - 현재 상태 확인용 패킷 (연결 유지용)
struct SC_CurrentState
    : TZPacket<PACKET_TYPE::SC_CURRENT_STATE, SC_CurrentState> {};

// battle - 플레이어 사망 알림
struct SC_Death : TZPacket<PACKET_TYPE::SC_DEATH, SC_Death> {
  int dead_id{};
  int killer_id{};
};

// battle - 플레이어 부활 알림
struct SC_Respawn : TZPacket<PACKET_TYPE::SC_RESPAWN, SC_Respawn> {
  int player_id{};
  float x{};
  float y{};
  int hp{};
};

// battle - 아이템 활성/비활성 상태 알림
struct SC_ItemState : TZPacket<PACKET_TYPE::SC_ITEM_STATE, SC_ItemState> {
  int item_id{};
  bool active{};
};

//==============================================================================================

// result - 게임 결과 정보
struct SC_GameResult : TZPacket<PACKET_TYPE::SC_GAME_RESULT, SC_GameResult> {
  int red_score{};
  int blue_score{};
  TeamType winner_team{TeamType::TEAM_NONE};

  int player_count{};
  int player_ids[MAX_ROOM_PLAYERS]{};
  int kills[MAX_ROOM_PLAYERS]{};
  int deaths[MAX_ROOM_PLAYERS]{};
};

#pragma pack(pop)

inline PACKET_STAGE GetPacketStage(PACKET_TYPE type) {
  using enum PACKET_TYPE;

  switch (type) {
  case CS_LOGIN:
  case CS_READY:
  case SC_LOGIN_RESULT:
  case SC_AVATAR_INFO:
    return PACKET_STAGE::TITLE;

  case SC_ROOM_ENTER:
  case CS_SELECT_CHARACTER:
  case SC_CHARACTER_SELECTED:
  case CS_GAME_READY:
  case SC_LOBBY_READY_STATE:
  case SC_GAME_START:
    return PACKET_STAGE::MATCH;

  case CS_MOVE:
  case CS_FACE_DIR:
  case CS_ATTACK:
  case CS_SKILL:
  case SC_ADD_PLAYER:
  case SC_REMOVE_PLAYER:
  case SC_MOVE_PLAYER:
  case SC_NETPLAYERSTATE:
  case SC_ROOM_SNAPSHOT:
  case SC_CURRENT_STATE:
  case SC_DEATH:
  case SC_RESPAWN:
  case SC_ITEM_STATE:
    return PACKET_STAGE::BATTLE;

  case SC_GAME_RESULT:
    return PACKET_STAGE::RESULT;

  default:
    return PACKET_STAGE::NONE;
  }
}
