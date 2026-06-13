#pragma once

#include "game/WorldState.h"

namespace patient_system {

// Отсчитывает cooldown стабилизированных пациентов и возвращает их к разговору.
void Update(WorldState &world, float dt, float &time_accumulator);

} // namespace patient_system
