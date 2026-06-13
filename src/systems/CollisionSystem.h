#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

namespace collision_system {

bool IsBlocked(const WorldState &world, const GameDatabase &db, int x, int y,
               Entity ignored = 0);

}
