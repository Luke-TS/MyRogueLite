#pragma once

#include "game/states.hpp"
#include "game/defs.hpp"
#include "systems.hpp"

struct SkillInstance; // forward declared from core/ecs.hpp
struct HitEvent;      // forward declared from core/ecs.hpp

inline void buildSkill(SkillInstance& inst) {
    // start from base
    inst.builtEffects = inst.def->effects;

    // effect defaults
    for(auto& effect : inst.builtEffects) {
        switch(effect.type) {
            case Defs::EffectType::DealDamage:
                effect.value0 = inst.def->baseDamage;
                break;
            default:
                break;
        }
    }

    // apply supports
    /*
    for (auto* sup : inst.supports) {
        applySupport(*sup, inst.builtEffects);
    }
    */
}

inline void buildPlayerSkills(GameContext& ctx, Entity player) {
    auto& ecs = ctx.ecs;
    auto& progress = ctx.progress;

    for (auto& s : progress.loadout) {
        SkillInstance inst;
        inst.def = &Defs::skills[s.skillIdx];

        /*
        for (int sup : s.supports) {
            inst.supports.push_back(&Defs::supports[sup]);
        }
        */

        buildSkill(inst);

        ecs.skills[player].skills.push_back(inst);
    }
}


inline void effectDealDamage(GameContext& ctx, const HitEvent& hit, const Defs::Effect& effect) {
    assert(effect.type == Defs::EffectType::DealDamage); 

    ctx.ecs.healths[hit.target].value -= effect.value0;
};

inline void effectSpawnProjectile(GameContext& ctx, const HitEvent& hit, const Defs::Effect& effect) {
    assert(effect.type == Defs::EffectType::SpawnProjectile); 

    //ctx.ecs.healths[hit.target].value -= effect.value0;
};

