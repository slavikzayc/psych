#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

namespace inventory_system {

void AddItem(WorldState &world, Entity owner, int item_id, int count);

bool RemoveItem(WorldState &world, Entity owner, int item_id, int count);

bool HasItem(const WorldState &world, Entity owner, int item_id);

void UseItem(WorldState &world, const GameDatabase &db, Entity owner,
             int item_id);

}
