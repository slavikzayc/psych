#pragma once

#include <string>

#include "game/WorldState.h"

namespace hud_system {

// Собирает нижнюю информационную панель для режима Exploration.
std::string BuildHud(const WorldState &world, const std::string &prompt);

} // namespace hud_system
