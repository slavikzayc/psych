#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

namespace night_event_system {

void Update(WorldState &world, GameDatabase &db, float dt,
            float &time_accumulator);

}
