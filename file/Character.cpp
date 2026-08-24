#include "Character.h"

const CharacterStats& CharacterManager::GetStats(CharacterType type)
{
	static CharacterStats default_stats{};

	static CharacterStats tanker{
		CharacterType::CHAR_TANKER,                 // type
		610,                         // max_hp
		0,                           // max_mp
		80,                          // attack_damage
		1.8f,                        // attack_cooldown
		150.0f,                      // attack_range
		310.0f,                      // move_speed
		AttackType::TANKER,          // attack_type

		SkillStats{
			SkillType::TANKER_SKILL, // type
			80,                     // damage
			55,                     // extra_damage
			11.0f,                  // cooldown
			1.2f,                   // stun_duration
			525.0f,                 // range
			0.0f,                   // mana_cost
			0.0f,                   // penetration_damage
			0.0f,                   // damage_reduce_per_hit
			0,                      // heal
			0.0f,                   // heal_area_range
			0.0f                    // dealer_area_range
		},

		PassiveStats{
			PassiveType::TANKER_PASSIVE, // type
			0.20f,      // 피해량의 20% 흡혈
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f
		}
	};

	static CharacterStats dealer{
		CharacterType::CHAR_DEALER,                 // type
		582,                         // max_hp
		250,                         // max_mp
		55,                          // attack_damage
		1.4f,                        // attack_cooldown
		150.0f,                      // attack_range
		325.0f,                      // move_speed
		AttackType::DEALER,          // attack_type

		SkillStats{
			SkillType::DEALER_SKILL, // type
			55,                     // damage
			0,                      // extra_damage
			5.0f,                   // cooldown
			0.0f,                   // stun_duration
			0.0f,                   // range
			45.0f,                  // mana_cost
			0.0f,                   // penetration_damage
			0.0f,                   // damage_reduce_per_hit
			0,                      // heal
			0.0f,                   // heal_area_range
			260.0f                  // dealer_area_range
		},

		PassiveStats{
			PassiveType::DEALER_PASSIVE, // type
			0.0f,       // lifesteal_rate
			0.30f,      // 받은 기본 공격 피해의 30% 반사
			0.0f,       // attack_speed_buff_duration
			0.0f,       // attack_cooldown_reduce
			0.0f,       // passive_cooldown
			0.0f        // self_heal_rate
		}
	};

	static CharacterStats archer{
		CharacterType::CHAR_ARCHER,                 // type
		520,                         // max_hp
		290,                         // max_mp
		55,                          // attack_damage
		1.5f,                        // attack_cooldown
		600.0f,                      // attack_range
		285.0f,                      // move_speed
		AttackType::ARCHER,          // attack_type

		SkillStats{
			SkillType::ARCHER_SKILL, // type
			320,                    // damage
			0,                      // extra_damage
			10.0f,                  // cooldown
			0.0f,                   // stun_duration
			2600.0f,                // range
			105.0f,                 // mana_cost
			320.0f,                 // penetration_damage
			0.15f,                  // damage_reduce_per_hit
			0,                      // heal
			0.0f,                   // heal_area_range
			0.0f                    // dealer_area_range
		},

		PassiveStats{
			PassiveType::ARCHER_PASSIVE, // type
			0.0f,                       // lifesteal_rate
			0.0f,                       // reflect_damage
			3.0f,						// 공속 증가 3초
			0.5f,						// 공격 쿨타임 50% 감소
			6.0f,						// 패시브 쿨타임 6초
			0.0f                        // self_heal_rate
		}
	};

	static CharacterStats healer{
		CharacterType::CHAR_HEALER,                 // type
		560,                         // max_hp
		390,                         // max_mp
		0,                           // attack_damage
		1.7f,                        // attack_cooldown
		625.0f,                      // attack_range
		295.0f,                      // move_speed
		AttackType::HEALER,          // attack_type

		SkillStats{
			SkillType::HEALER_SKILL, // type
			90,                     // damage
			0,                      // extra_damage
			13.0f,                  // cooldown
			0.0f,                   // stun_duration
			300.0f,                 // range
			0.0f,                   // mana_cost
			0.0f,                   // penetration_damage
			0.0f,                   // damage_reduce_per_hit
			195,                    // heal
			260.0f,                 // heal_area_range
			0.0f                    // dealer_area_range
		},

		PassiveStats{
			PassiveType::HEALER_PASSIVE, // type
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0.40f       // 아군 실제 치유량의 40% 자가 치유
		}
	};

	switch (type) {
	case CharacterType::CHAR_DEALER:
		return dealer;
	case CharacterType::CHAR_ARCHER:
		return archer;
	case CharacterType::CHAR_TANKER:
		return tanker;
	case CharacterType::CHAR_HEALER:
		return healer;
	default:
		return default_stats;
	}
}
