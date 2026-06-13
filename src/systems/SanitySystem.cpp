#include "systems/SanitySystem.h"

#include <algorithm>

void sanity_system::ChangeSanity(WorldState &world, int amount) {
  SanityComponent &sanity = world.registry.sanities.at(world.player);
  sanity.current = std::clamp(sanity.current + amount, 0, sanity.max);
  if (sanity.current <= 0) {
    world.mode = GameMode::Defeat;
    world.message = "Рассудок не выдержал ночной смены.";
  }
}

void sanity_system::UpdateSanity(WorldState &world, float dt,
                                 float &drain_accumulator) {
  if (world.mode != GameMode::Exploration &&
      world.mode != GameMode::Inventory &&
      world.mode != GameMode::DialogueCombat) {
    return;
  }

  SanityComponent &sanity = world.registry.sanities.at(world.player);

  drain_accumulator += dt;
  const float drain_interval =
      static_cast<float>(std::max(1, sanity.passive_drain_interval_seconds));
  while (drain_accumulator >= drain_interval) {
    drain_accumulator -= drain_interval;
    sanity_system::ChangeSanity(world, -sanity.passive_drain_per_minute);
    if (world.mode == GameMode::Defeat) {
      return;
    }
  }
}
