#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

namespace collision_system {

// Возвращает true, если клетка (x, y) занята стеной, границей карты
// или сущностью с блокирующей коллизией.
bool IsBlocked(const WorldState &world, const GameDatabase &db, int x, int y,
               Entity ignored = 0);

} // namespace collision_system
