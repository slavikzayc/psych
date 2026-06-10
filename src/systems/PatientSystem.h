#pragma once

#include "game/WorldState.h"

class PatientSystem {
 public:
  void Update(WorldState& world, float dt);

 private:
  float time_accumulator_ = 0.0f;
};
