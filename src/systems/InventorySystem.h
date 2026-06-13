#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"

namespace inventory_system {

// Добавляет предмет в инвентарь владельца. Если слот уже есть, увеличивает
// count.
void AddItem(WorldState &world, Entity owner, int item_id, int count);

// Удаляет count предметов. Возвращает false, если предмета не хватает.
bool RemoveItem(WorldState &world, Entity owner, int item_id, int count);

// Проверяет наличие хотя бы одного предмета с item_id.
bool HasItem(const WorldState &world, Entity owner, int item_id);

// Использует предмет: восстанавливает рассудок или добавляет бонус к диалогу.
void UseItem(WorldState &world, const GameDatabase &db, Entity owner,
             int item_id);

} // namespace inventory_system
