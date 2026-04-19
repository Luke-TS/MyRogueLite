#pragma once

#include "game/states.hpp"
#include "game/defs.hpp"


struct SkillInstance; // forward declared from core/ecs.hpp

inline void buildSkill(SkillInstance& inst) {
    // start from base
    inst.builtEffects = inst.def->effects;

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
