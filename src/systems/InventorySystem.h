#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

class InventorySystem {
 public:
  void AddItem(WorldState& world, Entity owner, int item_id, int count);
  bool RemoveItem(WorldState& world, Entity owner, int item_id, int count);
  bool HasItem(const WorldState& world, Entity owner, int item_id) const;
  void UseItem(WorldState& world, const GameDatabase& db, Entity owner, int item_id);
};
