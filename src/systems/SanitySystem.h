#pragma once

#include "game/WorldState.h"

namespace sanity_system {

// Меняет рассудок игрока на amount и проверяет условие поражения.
void ChangeSanity(WorldState &world, int amount);

// Пассивно снижает рассудок по таймеру.
void UpdateSanity(WorldState &world, float dt, float &drain_accumulator);

} // namespace sanity_system
