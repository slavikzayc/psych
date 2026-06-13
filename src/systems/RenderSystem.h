#pragma once

#include <string>

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/HUDSystem.h"

namespace render_system {

std::string RenderExploration(const WorldState &world, const GameDatabase &db,
                              const std::string &prompt);

std::string RenderMainMenu(int selected);
std::string RenderStatAllocation(int authority, int medication, int points,
                                 int selected);
std::string RenderInventory(const WorldState &world, const GameDatabase &db,
                            int selected);
std::string RenderDialogue(const WorldState &world, const GameDatabase &db);
std::string RenderVictory(const WorldState &world);
std::string RenderDefeat(const WorldState &world);

}
