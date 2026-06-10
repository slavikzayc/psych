#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/InventorySystem.h"

class InteractionSystem {
 public:
  void Interact(WorldState& world, const GameDatabase& db, InventorySystem& inventory);
  std::string GetPrompt(const WorldState& world) const;

 private:
  Entity FindInteractable(const WorldState& world) const;
};
