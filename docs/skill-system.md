# ECS Skill System Design

## Overview

This system implements a composable skill architecture inspired by games like Path of Exile.
Skills are defined as data and executed through ECS systems, allowing flexible combinations without hardcoding interactions.

---

## Core Principles

* Skills themselves are just data
* Supports modify the skill data
* Effects are instructions exectued by systems
* Systems remain generic and reusable

---

## Skill Representation

A skill consists of:

* Base properties (damage, cooldown, etc.)
* A list of effects
* A list of supports

Example:

```
Fireball:
- SpawnProjectile
- DealDamage(10)
```

---

## Supports

Supports modify a skill before execution.

They may:

* Modify existing effects
* Add new effects
* Adjust numerical values

Examples:

```
Multishot:
- SpawnProjectile → count += 2

Chain:
- Adds Chain(2) effect

DamageBoost:
- damage *= 1.5
```

Supports are applied at skill build time, producing a final effect list.

---

## Effects

Effects are atomic instructions interpreted by systems.

Examples:

* SpawnProjectile
* DealDamage
* ApplyStatus
* Chain

Effects are consumed by systems to produce behavior.

---

## Execution Pipeline

```
Input
 → CastSkillEvent
   → SkillBuild (apply supports)
     → EffectExecution
       → Spawn entities / emit events
         → ECS systems process results
```

---

## Entity Interaction

Some effects create entities that carry behavior forward.

Example: projectile entity

```
ProjectileComponent:
- damage
- onHitEffects
```

When a projectile hits a target, its stored effects are executed.

---

## Systems

The skill system relies on a small set of generic ECS systems:

* SkillBuildSystem
  Applies supports to produce final effect list

* EffectExecutionSystem
  Executes effects at cast time (e.g., spawning projectiles)

* ProjectileSystem
  Updates projectile movement

* CollisionSystem
  Detects overlaps and emits HitEvent

* OnHitEffectSystem
  Executes effects stored on entities

* DamageSystem
  Applies damage with cooldown handling

* StatusEffectSystem
  Handles ongoing effects such as poison or burn

---

## Example

Base skill:

```
Fireball:
- SpawnProjectile
- DealDamage(10)
```

Supports:

```
Multishot
Chain
```

Built skill:

```
- SpawnProjectile (count = 3)
- DealDamage(10)
- Chain(2)
```

Runtime behavior:

1. Three projectiles are spawned
2. Each projectile collides with an enemy
3. On hit:

   * Damage is applied
   * Chain effect triggers additional hits

---

## Design Notes

* No system should depend on specific skill combinations
* All combinations emerge from data composition
* Effects should remain small and focused
* Supports should only transform data, not execute logic

---

## Summary

This design separates:

* Data (skills, supports, effects)
* Execution (systems)
* State (components)

The result is a scalable system where new skills and supports can be added without modifying existing systems.
