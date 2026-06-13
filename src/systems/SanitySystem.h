#pragma once

#include "game/WorldState.h"

class SanitySystem {
 public:
  void ChangeSanity(WorldState& world, int amount);
  void UpdateSanity(WorldState& world, float dt);
 
private:
  float drain_accumulator_ = 0.0f;
};
