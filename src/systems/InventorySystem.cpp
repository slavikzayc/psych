#include "systems/InventorySystem.h"

#include <algorithm>

#include "systems/SanitySystem.h"

void InventorySystem::AddItem(WorldState& world, Entity owner, int item_id, int count) {
  auto& inventory = world.registry.inventories[owner];
  for (InventorySlot& slot : inventory.slots) {
    if (slot.item_id == item_id) {
      slot.count += count;
      return;
    }
  }
  inventory.slots.push_back({item_id, count});
}

bool InventorySystem::RemoveItem(WorldState& world, Entity owner, int item_id, int count) {
  auto inv_it = world.registry.inventories.find(owner);
  if (inv_it == world.registry.inventories.end()) {
    return false;
  }

  auto& slots = inv_it->second.slots;
  for (auto it = slots.begin(); it != slots.end(); ++it) {
    if (it->item_id == item_id && it->count >= count) {
      it->count -= count;
      if (it->count == 0) {
        slots.erase(it);
      }
      return true;
    }
  }
  return false;
}

bool InventorySystem::HasItem(const WorldState& world, Entity owner, int item_id) const {
  const auto inv_it = world.registry.inventories.find(owner);
  if (inv_it == world.registry.inventories.end()) {
    return false;
  }

  return std::any_of(
      inv_it->second.slots.begin(), inv_it->second.slots.end(),
      [item_id](const InventorySlot& slot) { return slot.item_id == item_id && slot.count > 0; });
}

void InventorySystem::UseItem(WorldState& world, const GameDatabase& db, Entity owner,
                              int item_id) {
  const auto item_it = db.items.find(item_id);
  if (item_it == db.items.end()) {
    world.message = "Неизвестный предмет.";
    return;
  }

  const ItemData& item = item_it->second;
  bool used = false;

  if (item.sanity_restore > 0) {
    SanitySystem sanity;
    sanity.ChangeSanity(world, item.sanity_restore);
    world.message = "Вы используете: " + item.name + ".";
    if (item.dialogue_bonus > 0 &&
        !(world.inventory_return_mode == GameMode::DialogueCombat)) {
        world.message += " Диалоговый эффект предмета потерян.";
    }
    world.message = world.message;
    used = true;
  }

  if (world.mode == GameMode::Inventory &&
      world.inventory_return_mode == GameMode::DialogueCombat && item.dialogue_bonus > 0) {
    auto combat_it = world.registry.dialogue_combats.find(world.active_patient);
    if (combat_it != world.registry.dialogue_combats.end()) {
      combat_it->second.pending_item_bonus += item.dialogue_bonus;
      world.message = item.name + " поможет в следующей реплике.";
      used = true;
    }
  }

  if (!used) {
    world.message = item.description.empty()
                        ? "Этот предмет сейчас не помогает."
                        : item.description;
    return;
  }

  if (item.consumable) {
    RemoveItem(world, owner, item_id, 1);
  }


}

