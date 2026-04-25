#pragma once

#include "game/states.hpp"
#include "game/defs.hpp"

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

using EffectFn = void(*)(GameContext&, const HitEvent&, const Defs::Effect&);

constexpr size_t EFX(Defs::EffectType t) { return (size_t)t; }

static const EffectFn EffectTable[EFX(Defs::EffectType::Count)] = {
    [EFX(Defs::EffectType::DealDamage)]      = effectDealDamage,
    [EFX(Defs::EffectType::SpawnProjectile)] = effectSpawnProjectile,
    [EFX(Defs::EffectType::WallBounce)]      = nullptr,
};
