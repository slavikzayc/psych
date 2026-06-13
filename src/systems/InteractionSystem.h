#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/InventorySystem.h"

namespace interaction_system {

void Interact(WorldState &world, const GameDatabase &db);

std::string GetPrompt(const WorldState &world);

}
