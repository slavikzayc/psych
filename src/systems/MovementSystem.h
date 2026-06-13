#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/CollisionSystem.h"
#include "systems/InputSystem.h"

namespace movement_system {

void Update(WorldState &world, const GameDatabase &db, InputCommand command);

}
