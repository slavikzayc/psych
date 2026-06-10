#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/CollisionSystem.h"
#include "systems/InputSystem.h"

class MovementSystem {
 public:
  void Update(WorldState& world, const GameDatabase& db, const CollisionSystem& collision,
              InputCommand command);
};
