#pragma once

#include "game/states.hpp"
#include "game/defs.hpp"

inline void buildSkill(SkillInstance& inst) {
    // copy the on-hit effect chain from the definition
    inst.builtEffects = inst.def->onHitEffects;

    // fill in computed values (e.g. damage from baseDamage)
    for(auto& effect : inst.builtEffects) {
        switch(effect.type) {
            case Defs::EffectType::DealDamage:
                effect.value0 = inst.def->baseDamage;
                break;
            case Defs::EffectType::MultiShot:
                inst.def->projectile.count  = 3;
                inst.def->projectile.spread = 10.f;
                break;
            default:
                break;
        }
    }

    // TODO: apply support gem modifiers here
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

using EffectFn = void(*)(GameContext&, const HitEvent&, const Defs::Effect&);

constexpr size_t EFX(Defs::EffectType t) { return (size_t)t; }

// Dispatch table indexed by EffectType.
// nullptr = handled elsewhere (e.g. WallBounce is handled by systemOnWallHitEffects).
static const EffectFn EffectTable[EFX(Defs::EffectType::Count)] = {
    [EFX(Defs::EffectType::WallBounce)] = nullptr,
    [EFX(Defs::EffectType::DealDamage)] = effectDealDamage,
};
