#pragma once

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "systems/InventorySystem.h"

namespace interaction_system {

// Выполняет действие F: подбор предмета, дверь, диалог, контейнер или документ.
void Interact(WorldState &world, const GameDatabase &db);

// Возвращает текст подсказки для ближайшего интерактивного объекта.
std::string GetPrompt(const WorldState &world);

} // namespace interaction_system
