#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

class NightEventSystem {
 public:
  void Update(WorldState& world, GameDatabase& db, float dt);
 private:
  float time_accumulator_ = 0.0f;
};
