#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

class CollisionSystem {
 public:
  bool IsBlocked(const WorldState& world, const GameDatabase& db, int x, int y,
                 Entity ignored = 0) const;
};
