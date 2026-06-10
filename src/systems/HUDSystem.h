#pragma once

#include <string>

#include "game/WorldState.h"

class HUDSystem {
 public:
  std::string BuildHud(const WorldState& world, const std::string& prompt) const;
};
