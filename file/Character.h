#pragma once
#include "protocol.h"

enum class AttackType {
    NONE,
	DEALER, //근딜러
	ARCHER, //아처
    TANKER, // 탱커
	HEALER  //힐러
};

enum class SkillType {
    NONE,
    DEALER_SKILL,
	ARCHER_SKILL,
    TANKER_SKILL,
	HEALER_SKILL
};

enum class PassiveType {
    NONE,
    DEALER_PASSIVE,
    ARCHER_PASSIVE,
    TANKER_PASSIVE,
    HEALER_PASSIVE
};

struct SkillStats {
	SkillType type = SkillType::NONE;          // 어떤 캐릭터 스킬인지 구분

	int damage = 0;                            // 스킬 기본 피해량
	int extra_damage = 0;                      // 추가 피해량

	float cooldown = 0.0f;                     // 스킬 쿨타임
	float stun_duration = 0.0f;                // 기절 시간
	float range = 0.0f;                        // 스킬 사거리
	float mana_cost = 0.0f;                    // 마나 사용량

	float penetration_damage = 0.0f;           // 원거리 관통 기본 피해량
	float damage_reduce_per_hit = 0.0f;        // 관통 시 적중할 때마다 피해 감소율

	int heal = 0;                              // 치유량
	float heal_area_range = 0.0f;              // 치유 장판 범위

	float dealer_area_range = 0.0f;            // 근접 딜러 주변 피해 범위
};

struct PassiveStats {
	PassiveType type = PassiveType::NONE;      // 어떤 캐릭터 패시브인지 구분

	float lifesteal_rate = 0.0f;               // 탱커 흡혈 비율
	float reflect_damage = 0.0f;               // 근접 딜러 피해 반사량

	float attack_speed_buff_duration = 0.0f;   // 원거리 딜러 공속 증가 지속시간
	float attack_cooldown_reduce = 0.0f;       // 원거리 딜러 공속 증가량

	float self_heal_rate = 0.0f;               // 힐러: 아군 치유량 대비 자가 치유 비율
};

struct CharacterStats {
	CharacterType type = CharacterType::CHAR_NONE;            // 캐릭터 종류

	int max_hp = 100;                          // 최대 체력
	int max_mp = 0;                            // 최대 마나

	int attack_damage = 10;                    // 기본 공격력
	float attack_cooldown = 1.0f;              // 기본 공격속도, 몇 초마다 한 번 공격 가능한지
	float attack_range = 150.0f;               // 기본 공격 사거리
	float move_speed = 300.0f;                 // 초당 이동거리

	AttackType attack_type = AttackType::NONE; // 기본 공격 타입

	SkillStats skill;                          // 캐릭터 스킬 정보
	PassiveStats passive;                      // 캐릭터 패시브 정보
};

class CharacterManager {
public:
    static const CharacterStats& GetStats(CharacterType type);
};