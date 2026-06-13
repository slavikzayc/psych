#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

namespace night_event_system {

// Обновляет таймер смены, фазу ночи и события из night_events.json.
void Update(WorldState &world, GameDatabase &db, float dt,
            float &time_accumulator);

} // namespace night_event_system
