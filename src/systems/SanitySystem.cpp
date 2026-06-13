#include "systems/SanitySystem.h"

#include <algorithm>

void sanity_system::ChangeSanity(WorldState &world, int amount) {
  // Рассудок хранится на игроке в SanityComponent.
  auto sanity_it = world.registry.sanities.find(world.player);
  if (sanity_it == world.registry.sanities.end()) {
    return;
  }

  // clamp не даёт рассудку выйти ниже 0 или выше max.
  SanityComponent &sanity = sanity_it->second;
  sanity.current = std::clamp(sanity.current + amount, 0, sanity.max);
  if (sanity.current <= 0) {
    // Нулевой рассудок — немедленное поражение.
    world.mode = GameMode::Defeat;
    world.message = "Рассудок не выдержал ночной смены.";
  }
}

void sanity_system::UpdateSanity(WorldState &world, float dt,
                                 float &drain_accumulator) {
  // Рассудок должен падать только во время активной игры, а не в меню.
  if (world.mode != GameMode::Exploration &&
      world.mode != GameMode::Inventory &&
      world.mode != GameMode::DialogueCombat) {
    return;
  }

  auto sanity_it = world.registry.sanities.find(world.player);
  if (sanity_it == world.registry.sanities.end()) {
    return;
  }

  // Накапливаем dt. Когда накопится нужный интервал, снимаем рассудок.
  drain_accumulator += dt;
  const float drain_interval = static_cast<float>(
      std::max(1, sanity_it->second.passive_drain_interval_seconds));
  while (drain_accumulator >= drain_interval) {
    // while нужен на случай, если один кадр был длинным и накопилось несколько
    // тиков.
    drain_accumulator -= drain_interval;
    sanity_system::ChangeSanity(world,
                                -sanity_it->second.passive_drain_per_minute);
    if (world.mode == GameMode::Defeat) {
      return;
    }
  }
}
