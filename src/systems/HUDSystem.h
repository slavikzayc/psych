#pragma once

#include <string>

#include "game/WorldState.h"

namespace hud_system {

std::string BuildHud(const WorldState &world, const std::string &prompt);

}
