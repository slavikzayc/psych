#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/InputSystem.h"

namespace dialogue_combat_system {

// Обрабатывает ввод в диалоговом бою: выбор реплики, выход или открытие
// инвентаря.
void HandleInput(WorldState &world, const GameDatabase &db,
                 InputCommand command);

} // namespace dialogue_combat_system
