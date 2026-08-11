#pragma once
#include <array>
#include <cstdint>

constexpr short PORT = 9000;
constexpr int WORLD_WIDTH = 400;
constexpr int WORLD_HEIGHT = 400;
constexpr int MAX_PLAYERS = 18;
constexpr int MAX_NAME_LEN = 20;
constexpr int MAX_ROOM_AI = 6;
constexpr int MAX_ROOM_PLAYERS = 2; // 원래 6명

enum struct PACKET_STAGE : std::uint8_t {
    DEFAULT,
	TITLE,
	MATCH,
	BATTLE,
	RESULT,
	NONE
};

enum struct PACKET_TYPE : std::uint8_t {
	// 클라이언트에서 서버에게 3초마다 패킷 전송, 서버는 받은 패킷에 응답 처리, 10초 이상 응답 없을시 연결 끊김 처리
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
	SC_ATTACK, //서버에서 클라이언트로 공격 알림
	SC_SKILL, //서버에서 클라이언트로 스킬 사용 알림
	SC_SKILL_HIT, //서버에서 클라이언트로 스킬 히트 알림

	// result
	SC_GAME_RESULT,

	None,
};

//enum DIRECTION {UP,DOWN,LEFT,RIGHT};

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

template <PACKET_TYPE Type, typename Derived>
struct TZPacket
{
	std::uint16_t size;
	PACKET_TYPE type;

	constexpr TZPacket()
		: size(sizeof(Derived)),
		type(Type)
	{}
};

struct TZPacketHeader
	: TZPacket<PACKET_TYPE::None, TZPacketHeader>{};

// default - 연결 상태 확인
struct CS_ConnectionCheck : TZPacket<PACKET_TYPE::CS_CONNECTION_CHECK, CS_ConnectionCheck>{};
// 클라이언트가 서버에게 연결 확인 요청

struct SC_ConnectionCheck : TZPacket<PACKET_TYPE::SC_CONNECTION_CHECK, SC_ConnectionCheck> {};
// 클라이언트가 서버에게 연결 확인 요청

// title - 로그인 요청
struct CS_Login : TZPacket<PACKET_TYPE::CS_LOGIN, CS_Login> {
    char username[MAX_NAME_LEN]{};
	char password[MAX_NAME_LEN]{};
};

// title - 클라이언트 매칭 준비 요청
struct CS_Ready : TZPacket<PACKET_TYPE::CS_READY, CS_Ready> {};

// title - 로그인 성공/실패 결과
struct SC_LoginResult : TZPacket<PACKET_TYPE::SC_LOGIN_RESULT, SC_LoginResult> {
    bool success{};
    char message[50]{};
};

// title - 로그인 후 내 플레이어 기본 정보 (편한대로 써도 됨)
struct SC_AvatarInfo : TZPacket<PACKET_TYPE::SC_AVATAR_INFO, SC_AvatarInfo> {
    int playerId{};
    short x{};
    short y{};
};

//==============================================================================================

// match - 방 입장 시 방 정보와 플레이어 목록 전달
struct SC_RoomEnter : TZPacket<PACKET_TYPE::SC_ROOM_ENTER, SC_RoomEnter> {
    int room_id{};
    int player_count{};
	// 플레이어 수에 따라 플레이어 닉네임을 보내줌
	char usernames[MAX_ROOM_PLAYERS][MAX_NAME_LEN]{};
    int player_ids[MAX_ROOM_PLAYERS]{};
    int teams[MAX_ROOM_PLAYERS]{};
	//TeamType teams[MAX_ROOM_PLAYERS]{};
};

// match - 캐릭터 선택 요청
struct CS_SelectCharacter : TZPacket<PACKET_TYPE::CS_SELECT_CHARACTER, CS_SelectCharacter> {
    CharacterType character{ CharacterType::CHAR_NONE };
};

// match - 특정 플레이어의 캐릭터 선택 결과 알림
struct SC_CharacterSelected : TZPacket<PACKET_TYPE::SC_CHARACTER_SELECTED, SC_CharacterSelected> {
    int player_id{};
    CharacterType character{ CharacterType::CHAR_NONE };
};

// match - 게임 시작 준비 상태 변경 요청
struct CS_GameReady : TZPacket<PACKET_TYPE::CS_GAME_READY, CS_GameReady> {
    bool ready{};
};

// match - 매칭 방에서 다른 플레이어 준비 상태 알림 
struct SC_LobbyReadyState : TZPacket<PACKET_TYPE::SC_LOBBY_READY_STATE, SC_LobbyReadyState> {
    int player_id{};
    bool ready{};
};

// match - 모든 준비 완료 후 게임 시작 알림
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
    short skill_id{}; //캐릭터 별 스킬이 다르긴 하나 어차피 캐릭터 당 스킬 하나여서 상관없음.
};

//==============================================================================================
// *현재는 사용하지 않음, 나중에 필요하면 구현*
// battle - 게임 중 플레이어 추가 알림 (재접속/중도 참가용)
struct SC_AddPlayer : TZPacket<PACKET_TYPE::SC_ADD_PLAYER, SC_AddPlayer> {
    int playerId{};
    char username[MAX_NAME_LEN]{};
    short x{};
    short y{};
};

// battle - 게임 중 플레이어 제거 알림 (연결 종료/퇴장용) 
struct SC_RemovePlayer : TZPacket<PACKET_TYPE::SC_REMOVE_PLAYER, SC_RemovePlayer> {
    int playerid{};
};
//==============================================================================================

// battle - 특정 플레이어 이동 위치 알림
struct SC_MovePlayer : TZPacket<PACKET_TYPE::SC_MOVE_PLAYER, SC_MovePlayer> {
    int playerId{};
    short x{};
    short y{};
};

// battle - 스냅샷에 포함되는 플레이어 상태 정보
struct NetPlayerState {
    int player_id{};

    float x{};
    float y{};

    float faceX{};
    float faceY{};

    int hp{};
    int max_hp{};

    bool alive{};
    CharacterType character{ CharacterType::CHAR_NONE };

    int kill_count{};
    int death_count{};

    int current_target_id{ -1 };
};

// battle - 방 전체 상태 주기 동기화
struct SC_RoomSnapshot : TZPacket<PACKET_TYPE::SC_ROOM_SNAPSHOT, SC_RoomSnapshot> {
    int count{};
    int red_score{};
    int blue_score{};
    float time_left{};
    NetPlayerState players[MAX_ROOM_AI]{};
};

// battle - 현재 상태 확인용 패킷 (아직 내용 없음)
struct SC_CurrentState : TZPacket<PACKET_TYPE::SC_CURRENT_STATE, SC_CurrentState> {};

// battle - 플레이어 사망 알림
struct SC_Death : TZPacket<PACKET_TYPE::SC_DEATH, SC_Death> {
    int dead_id{};
    int killer_id{};
};

// battle - 플레이어 리스폰 알림
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

// battle - 기본 공격 발생 알림
struct SC_Attack : TZPacket<PACKET_TYPE::SC_ATTACK, SC_Attack> {
	int attacker_id{};	// 공격자 ID
	int target_id{};    // 공격 대상 ID
};

// battle - 스킬 사용 알림
struct SC_Skill : TZPacket<PACKET_TYPE::SC_SKILL, SC_Skill> {
	int caster_id{};								// 스킬 시전자 ID
};

// battle - 스킬 적중 알림
struct SC_SkillHit : TZPacket<PACKET_TYPE::SC_SKILL_HIT, SC_SkillHit> {
	int caster_id{};							// 스킬 시전자 ID
	int target_id{};							// 스킬 적중 대상 ID
};

//==============================================================================================

// result - 게임 종료 결과 전달
struct SC_GameResult : TZPacket<PACKET_TYPE::SC_GAME_RESULT, SC_GameResult> {
    int red_score{};
    int blue_score{};
    TeamType winner_team{ TeamType::TEAM_NONE };

    int player_count{};
    int player_ids[MAX_ROOM_PLAYERS]{};
    int kills[MAX_ROOM_PLAYERS]{};
    int deaths[MAX_ROOM_PLAYERS]{};
};

#pragma pack(pop)

inline PACKET_STAGE GetPacketStage(PACKET_TYPE type)
{
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
	case SC_ATTACK:
	case SC_SKILL:
	case SC_SKILL_HIT:
        return PACKET_STAGE::BATTLE;

    case SC_GAME_RESULT:
        return PACKET_STAGE::RESULT;

    default:
        return PACKET_STAGE::NONE;
    }
}
