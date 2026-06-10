#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/InputSystem.h"

class DialogueCombatSystem {
 public:
  void HandleInput(WorldState& world, const GameDatabase& db, InputCommand command);
};
