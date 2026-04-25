#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "game/dungeon_sprites.hpp"

#include <toml.hpp>

namespace Defs {

// -----------------------------------------------
// ENEMY DEFINITIONS
// -----------------------------------------------

struct EnemyDef {
    std::string               name;
    float                     speed;
    float                     health;
    DungeonSprites::SpriteIdx sprite;
    float                     scale;
};

enum EnemyIdx {
    SKELETON,
    ZOMBIE,
    BIGBOI,
};

inline std::vector<EnemyDef> enemies;

// -----------------------------------------------
// SKILL DEFINITIONS
// -----------------------------------------------

enum class SkillType {
    Projectile,
    Melee,
};

struct ProjectileParams {
    float speed    = 400.f;
    float fireRate = 1.f;
    int   count    = 1;
    float spread   = 0.f;
};

struct MeleeParams {
    float range      = 150.f;
    float arcDegrees = 90.f;
};

enum class EffectType : int8_t {
    WallBounce,
    DealDamage,
    MultiShot,
    Count,
};

struct Effect {
    EffectType type;
    float      value0 = 0.f;
    float      value1 = 0.f;
    int        count  = 0;
};

struct SkillDef {
    std::string               name;
    SkillType                 type;
    float                     baseDamage;
    DungeonSprites::SpriteIdx sprite;
    float                     scale;

    ProjectileParams projectile;
    MeleeParams      melee;

    std::vector<Effect> onHitEffects;
};

enum SkillIdx {
    SKILL_BOW,
    SKILL_FIREBALL,
};

inline std::vector<SkillDef> skills;

// -----------------------------------------------
// PLAYER CHARACTER DEFINITIONS
// -----------------------------------------------

struct CharacterDef {
    std::string                  name;
    float                        speed;
    float                        maxHealth;
    DungeonSprites::SpriteIdx    sprite;
    float                        scale;
    Defs::SkillIdx               startingSkill;
};

enum CharacterIdx {
    ARCHER,
    MAGE,
};

inline std::vector<CharacterDef> characters;

// -----------------------------------------------
// LOADER
// -----------------------------------------------

namespace detail {

inline DungeonSprites::SpriteIdx parseSpriteIdx(const std::string& s) {
    static const std::unordered_map<std::string, DungeonSprites::SpriteIdx> map = {
        {"CHARACTER_1", DungeonSprites::CHARACTER_1},
        {"CHARACTER_2", DungeonSprites::CHARACTER_2},
        {"CHARACTER_3", DungeonSprites::CHARACTER_3},
        {"WEAPON",      DungeonSprites::WEAPON},
        {"AXE",         DungeonSprites::AXE},
        {"ARROW",       DungeonSprites::ARROW},
        {"BOW",         DungeonSprites::BOW},
        {"SKELETON",    DungeonSprites::SKELETON},
        {"ZOMBIE",      DungeonSprites::ZOMBIE},
        {"BIGBOI",      DungeonSprites::BIGBOI},
        {"FIRE",        DungeonSprites::FIRE},
        {"BONES",       DungeonSprites::BONES},
    };
    auto it = map.find(s);
    if (it == map.end()) throw std::runtime_error("Unknown sprite: " + s);
    return it->second;
}

inline EffectType parseEffectType(const std::string& s) {
    static const std::unordered_map<std::string, EffectType> map = {
        {"WallBounce", EffectType::WallBounce},
        {"DealDamage", EffectType::DealDamage},
        {"MultiShot",  EffectType::MultiShot},
    };
    auto it = map.find(s);
    if (it == map.end()) throw std::runtime_error("Unknown effect type: " + s);
    return it->second;
}

inline SkillType parseSkillType(const std::string& s) {
    if (s == "Projectile") return SkillType::Projectile;
    if (s == "Melee")      return SkillType::Melee;
    throw std::runtime_error("Unknown skill type: " + s);
}

} // namespace detail

inline void loadDefs() {
    const std::string path = std::string(ASSETS_DIR) + "/defs.toml";

    toml::table doc;
    try {
        doc = toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error(std::string("Failed to parse defs.toml: ") + e.what());
    }

    enemies.clear();
    if (auto* arr = doc["enemies"].as_array()) {
        for (auto& node : *arr) {
            auto& t = *node.as_table();
            EnemyDef def;
            def.name   = t["name"].value<std::string>().value();
            def.speed  = (float)t["speed"].value<double>().value();
            def.health = (float)t["health"].value<double>().value();
            def.sprite = detail::parseSpriteIdx(t["sprite"].value<std::string>().value());
            def.scale  = (float)t["scale"].value<double>().value();
            enemies.push_back(std::move(def));
        }
    }

    skills.clear();
    if (auto* arr = doc["skills"].as_array()) {
        for (auto& node : *arr) {
            auto& t = *node.as_table();
            SkillDef def;
            def.name       = t["name"].value<std::string>().value();
            def.type       = detail::parseSkillType(t["type"].value<std::string>().value());
            def.baseDamage = (float)t["baseDamage"].value<double>().value();
            def.sprite     = detail::parseSpriteIdx(t["sprite"].value<std::string>().value());
            def.scale      = (float)t["scale"].value<double>().value();

            if (auto* proj = t["projectile"].as_table()) {
                def.projectile.speed    = (float)(*proj)["speed"].value<double>().value_or(400.0);
                def.projectile.fireRate = (float)(*proj)["fireRate"].value<double>().value_or(1.0);
                def.projectile.count    = (int)(*proj)["count"].value<int64_t>().value_or(1);
                def.projectile.spread   = (float)(*proj)["spread"].value<double>().value_or(0.0);
            }
            if (auto* melee = t["melee"].as_table()) {
                def.melee.range      = (float)(*melee)["range"].value<double>().value_or(150.0);
                def.melee.arcDegrees = (float)(*melee)["arcDegrees"].value<double>().value_or(90.0);
            }

            if (auto* effects = t["onHitEffects"].as_array()) {
                for (auto& effectNode : *effects) {
                    auto& et = *effectNode.as_table();
                    Effect effect;
                    effect.type   = detail::parseEffectType(et["type"].value<std::string>().value());
                    effect.value0 = (float)et["value0"].value<double>().value_or(0.0);
                    effect.value1 = (float)et["value1"].value<double>().value_or(0.0);
                    effect.count  = (int)et["count"].value<int64_t>().value_or(0);
                    def.onHitEffects.push_back(effect);
                }
            }

            skills.push_back(std::move(def));
        }
    }

    characters.clear();
    if (auto* arr = doc["characters"].as_array()) {
        for (auto& node : *arr) {
            auto& t = *node.as_table();
            CharacterDef def;
            def.name      = t["name"].value<std::string>().value();
            def.speed     = (float)t["speed"].value<double>().value();
            def.maxHealth = (float)t["maxHealth"].value<double>().value();
            def.sprite    = detail::parseSpriteIdx(t["sprite"].value<std::string>().value());
            def.scale     = (float)t["scale"].value<double>().value();

            std::string skillName = t["startingSkill"].value<std::string>().value();
            def.startingSkill = SKILL_BOW; // fallback
            for (int i = 0; i < (int)skills.size(); i++) {
                if (skills[i].name == skillName) {
                    def.startingSkill = (SkillIdx)i;
                    break;
                }
            }

            characters.push_back(std::move(def));
        }
    }
}

} // namespace Defs
