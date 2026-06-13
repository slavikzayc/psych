#include "systems/InventorySystem.h"

#include <algorithm>

#include "systems/SanitySystem.h"

void inventory_system::AddItem(WorldState &world, Entity owner, int item_id,
                               int count) {
  // operator[] создаёт InventoryComponent автоматически, если его ещё нет.
  auto &inventory = world.registry.inventories[owner];

  // Сначала пытаемся найти существующий слот с таким item_id.
  for (InventorySlot &slot : inventory.slots) {
    if (slot.item_id == item_id) {
      slot.count += count;
      return;
    }
  }

  // Если слот не найден, добавляем новый.
  inventory.slots.push_back({item_id, count});
}

bool inventory_system::RemoveItem(WorldState &world, Entity owner, int item_id,
                                  int count) {
  // Если у сущности нет инвентаря, удалять нечего.
  auto inv_it = world.registry.inventories.find(owner);
  if (inv_it == world.registry.inventories.end()) {
    return false;
  }

  // Ищем слот с нужным предметом и достаточным количеством.
  auto &slots = inv_it->second.slots;
  for (auto it = slots.begin(); it != slots.end(); ++it) {
    if (it->item_id == item_id && it->count >= count) {
      it->count -= count;
      if (it->count == 0) {
        // Слот с нулевым количеством удаляется, чтобы не отображать x0.
        slots.erase(it);
      }
      return true;
    }
  }
  return false;
}

bool inventory_system::HasItem(const WorldState &world, Entity owner,
                               int item_id) {
  // Проверка используется, например, запертыми дверями для ключей.
  const auto inv_it = world.registry.inventories.find(owner);
  if (inv_it == world.registry.inventories.end()) {
    return false;
  }

  return std::any_of(inv_it->second.slots.begin(), inv_it->second.slots.end(),
                     [item_id](const InventorySlot &slot) {
                       return slot.item_id == item_id && slot.count > 0;
                     });
}

void inventory_system::UseItem(WorldState &world, const GameDatabase &db,
                               Entity owner, int item_id) {
  // Все свойства предмета берутся из GameDatabase, а не из ItemComponent.
  const auto item_it = db.items.find(item_id);
  if (item_it == db.items.end()) {
    world.message = "Неизвестный предмет.";
    return;
  }

  // used нужен, чтобы понять: предмет реально применился или нужно показать
  // описание.
  const ItemData &item = item_it->second;
  bool used = false;

  // sanity_restore означает, что предмет восстанавливает рассудок.
  if (item.sanity_restore > 0) {
    sanity_system::ChangeSanity(world, item.sanity_restore);
    world.message = "Вы используете: " + item.name + ".";
    if (item.dialogue_bonus > 0 &&
        !(world.inventory_return_mode == GameMode::DialogueCombat)) {
      world.message += " Диалоговый эффект предмета потерян.";
    }
    used = true;
  }

  // Если инвентарь открыт из диалога, предмет может усилить следующую реплику.
  if (world.mode == GameMode::Inventory &&
      world.inventory_return_mode == GameMode::DialogueCombat &&
      item.dialogue_bonus > 0) {
    auto combat_it = world.registry.dialogue_combats.find(world.active_patient);
    if (combat_it != world.registry.dialogue_combats.end()) {
      combat_it->second.pending_item_bonus += item.dialogue_bonus;
      world.message = item.name + " поможет в следующей реплике.";
      used = true;
    }
  }

  // Неприменимый предмет не расходуется, а просто выводит описание.
  if (!used) {
    world.message = item.description.empty()
                        ? "Этот предмет сейчас не помогает."
                        : item.description;
    return;
  }

  // Расходуемые предметы удаляются только после успешного применения.
  if (item.consumable) {
    inventory_system::RemoveItem(world, owner, item_id, 1);
  }
}
