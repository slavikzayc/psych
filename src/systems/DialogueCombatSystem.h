#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/InputSystem.h"

namespace dialogue_combat_system {

void HandleInput(WorldState &world, const GameDatabase &db,
                 InputCommand command);

}
