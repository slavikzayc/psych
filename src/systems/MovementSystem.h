#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/CollisionSystem.h"
#include "systems/InputSystem.h"

namespace movement_system {

// Выполняет перемещение игрока на одну клетку по команде направления.
void Update(WorldState &world, const GameDatabase &db, InputCommand command);

} // namespace movement_system
