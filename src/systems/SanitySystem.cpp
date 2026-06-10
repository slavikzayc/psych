#include "systems/SanitySystem.h"

#include <algorithm>

void SanitySystem::ChangeSanity(WorldState& world, int amount) {
  auto sanity_it = world.registry.sanities.find(world.player);
  if (sanity_it == world.registry.sanities.end()) {
    return;
  }

  SanityComponent& sanity = sanity_it->second;
  sanity.current = std::clamp(sanity.current + amount, 0, sanity.max);
  if (sanity.current <= 0) {
    world.mode = GameMode::Defeat;
    world.message = "Рассудок не выдержал ночной смены.";
  }
}

void SanitySystem::UpdateSanity(WorldState& world, float dt) {
  if (world.mode != GameMode::Exploration && world.mode != GameMode::Inventory &&
      world.mode != GameMode::DialogueCombat) {
    return;
  }

  auto sanity_it = world.registry.sanities.find(world.player);
  if (sanity_it == world.registry.sanities.end()) {
    return;
  }

  drain_accumulator_ += dt;
  const float drain_interval =
      static_cast<float>(std::max(1, sanity_it->second.passive_drain_interval_seconds));
  while (drain_accumulator_ >= drain_interval) {
    drain_accumulator_ -= drain_interval;
    ChangeSanity(world, -sanity_it->second.passive_drain_per_minute);
    if (world.mode == GameMode::Defeat) {
      return;
    }
  }
}
