#pragma once

#include <string>

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/HUDSystem.h"

class RenderSystem {
 public:
  std::string RenderExploration(const WorldState& world, const GameDatabase& db,
                                const std::string& prompt, const HUDSystem& hud) const;
  std::string RenderMainMenu(int selected) const;
  std::string RenderStatAllocation(int authority, int medication, int points, int selected) const;
  std::string RenderInventory(const WorldState& world, const GameDatabase& db, int selected) const;
  std::string RenderDialogue(const WorldState& world, const GameDatabase& db) const;
  std::string RenderVictory(const WorldState& world) const;
  std::string RenderDefeat(const WorldState& world) const;
};
