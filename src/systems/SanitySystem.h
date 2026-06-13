#pragma once

#include "game/WorldState.h"

namespace sanity_system {

void ChangeSanity(WorldState &world, int amount);

void UpdateSanity(WorldState &world, float dt, float &drain_accumulator);

}
