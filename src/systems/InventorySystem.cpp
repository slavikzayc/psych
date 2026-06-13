#include "systems/InventorySystem.h"

#include <algorithm>

#include "systems/SanitySystem.h"

void inventory_system::AddItem(WorldState &world, Entity owner, int item_id,
                               int count) {
  auto &inventory = world.registry.inventories[owner];

  for (InventorySlot &slot : inventory.slots) {
    if (slot.item_id == item_id) {
      slot.count += count;
      return;
    }
  }

  inventory.slots.push_back({item_id, count});
}

bool inventory_system::RemoveItem(WorldState &world, Entity owner, int item_id,
                                  int count) {
  auto &slots = world.registry.inventories.at(owner).slots;
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

bool inventory_system::HasItem(const WorldState &world, Entity owner,
                               int item_id) {
  const auto &slots = world.registry.inventories.at(owner).slots;
  return std::any_of(slots.begin(), slots.end(), [item_id](const auto &slot) {
    return slot.item_id == item_id && slot.count > 0;
  });
}

void inventory_system::UseItem(WorldState &world, const GameDatabase &db,
                               Entity owner, int item_id) {
  const ItemData &item = db.items.at(item_id);
  bool used = false;

  if (item.sanity_restore > 0) {
    sanity_system::ChangeSanity(world, item.sanity_restore);
    world.message = "Вы используете: " + item.name + ".";
    if (item.dialogue_bonus > 0 &&
        !(world.inventory_return_mode == GameMode::DialogueCombat)) {
      world.message += " Диалоговый эффект предмета потерян.";
    }
    used = true;
  }

  if (world.mode == GameMode::Inventory &&
      world.inventory_return_mode == GameMode::DialogueCombat &&
      item.dialogue_bonus > 0) {
    world.registry.dialogue_combats.at(world.active_patient)
        .pending_item_bonus += item.dialogue_bonus;
    world.message = item.name + " поможет в следующей реплике.";
    used = true;
  }

  if (!used) {
    world.message = item.description.empty()
                        ? "Этот предмет сейчас не помогает."
                        : item.description;
    return;
  }

  if (item.consumable) {
    inventory_system::RemoveItem(world, owner, item_id, 1);
  }
}
