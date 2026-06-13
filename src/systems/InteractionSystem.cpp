#include "systems/InteractionSystem.h"

#include <array>

#include "ecs/Registry.h"

namespace {
bool SameCell(const PositionComponent& a, const PositionComponent& b) {
  return a.x == b.x && a.y == b.y;
}
}  

Entity InteractionSystem::FindInteractable(const WorldState& world) const {
    const auto player_pos_it = world.registry.positions.find(world.player);
    if (player_pos_it == world.registry.positions.end()) return 0;
    const PositionComponent& pp = player_pos_it->second;

    constexpr std::array<PositionComponent, 5> cells = { {{0,0},{0,-1},{0,1},{-1,0},{1,0}} };

    for (const PositionComponent& offset : cells) {
        const int x = pp.x + offset.x;
        const int y = pp.y + offset.y;

        for (const auto& [entity, interactable] : world.registry.interactables) {
            const auto map_it = world.registry.maps.find(entity);
            if (map_it != world.registry.maps.end() &&
                map_it->second.map_id != world.currentmap_id) continue;

            const auto pos_it = world.registry.positions.find(entity);
            if (pos_it == world.registry.positions.end()) continue;
            if (pos_it->second.x != x || pos_it->second.y != y) continue;

            // Pickup — только на клетке игрока (offset == {0,0})
            if (interactable.type == InteractionType::Pickup && offset.x == 0 && offset.y == 0) {
                return entity;
            }
            // Остальные типы — только в соседних клетках
            if (interactable.type != InteractionType::Pickup && (offset.x != 0 || offset.y != 0)) {
                return entity;
            }
        }
    }
    return 0;
}

void InteractionSystem::Interact(WorldState& world, const GameDatabase& db,
                                 InventorySystem& inventory) {
  const Entity target = FindInteractable(world);
  if (target == 0) {
    world.message =
        "Рядом нет ничего, с чем можно взаимодействовать.";
    return;
  }

  const auto interactable_it = world.registry.interactables.find(target);
  if (interactable_it == world.registry.interactables.end()) {
    return;
  }

  switch (interactable_it->second.type) {
    case InteractionType::Pickup: {
      const auto item_it = world.registry.items.find(target);
      if (item_it == world.registry.items.end()) {
        break;
      }
      inventory.AddItem(world, world.player, item_it->second.item_id, 1);
      const auto data_it = db.items.find(item_it->second.item_id);
      world.message =
          "Подобрано: " +
          (data_it == db.items.end() ? std::string("предмет") : data_it->second.name) + ".";
      DestroyEntity(world.registry, target);
      break;
    }
    case InteractionType::OpenDoor: {
      auto door_it = world.registry.doors.find(target);
      if (door_it == world.registry.doors.end()) {
        break;
      }
      DoorComponent& door = door_it->second;
      if (door.locked && door.required_key_item_id > 0 &&
          !inventory.HasItem(world, world.player, door.required_key_item_id)) {
        world.message = "Дверь заперта. Нужен ключ.";
        break;
      }
      door.locked = false;
      if (door.target_map_id > 0) {
        world.currentmap_id = door.target_map_id;
        world.registry.maps[world.player] = {door.target_map_id};
        world.registry.positions[world.player] = {door.target_x, door.target_y};
        world.message = "Вы переходите в другую комнату.";
      } else {
        world.registry.collisions.erase(target);
        world.registry.renderables[target].symbol = '/';
        world.message =
            "Дверь открыта. За ней пахнет хлоркой и пылью.";
      }
      break;
    }
    case InteractionType::StartDialogue: {
      auto patient_it = world.registry.patients.find(target);
      if (patient_it == world.registry.patients.end()) {
        break;
      }
      PatientComponent& patient = patient_it->second;
      if (patient.state == PatientState::Treated) {
        world.message =
            "Пациент пока стабилен. Дайте ему немного "
            "времени.";
        break;
      }
      const auto dialogue_it = db.dialogues.find(patient.dialogue_id);
      if (dialogue_it == db.dialogues.end()) {
        world.message = "У пациента нет доступного диалога.";
        break;
      }
      patient.state = PatientState::InDialogue;
      patient.current_tension =
          patient.current_tension <= 0 ? patient.max_tension : patient.current_tension;
      world.active_patient = target;
      world.registry.dialogue_combats[target] = {
          patient.dialogue_id, dialogue_it->second.start_node_id, target, true, 0};
      world.mode = GameMode::DialogueCombat;
      world.message = "Вы осторожно начинаете разговор.";
      break;
    }
    case InteractionType::SearchContainer: {
      const auto item_it = world.registry.items.find(target);
      if (item_it != world.registry.items.end() && item_it->second.item_id > 0) {
        inventory.AddItem(world, world.player, item_it->second.item_id, 1);
        const auto data_it = db.items.find(item_it->second.item_id);
        world.message =
            "В шкафчике найдено: " +
            (data_it == db.items.end() ? std::string("предмет") : data_it->second.name) +
            ".";
        world.registry.items.erase(target);
        InteractableComponent& interactable = world.registry.interactables[target];
        if (!interactable.empty_prompt_text.empty()) {
          interactable.prompt_text = interactable.empty_prompt_text;
        }
      } else {
        world.message = "Шкафчик пуст.";
      }
      break;
    }
    case InteractionType::ReadDocument: {
      const auto item_it = world.registry.items.find(target);
      const auto data_it = item_it == world.registry.items.end()
                               ? db.items.end()
                               : db.items.find(item_it->second.item_id);
      world.message = data_it == db.items.end()
                          ? "Вы читаете обрывок медицинской карты."
                          : data_it->second.description;
      break;
    }
  }
}

std::string InteractionSystem::GetPrompt(const WorldState& world) const {
  const Entity target = FindInteractable(world);
  if (target == 0) {
    return "";
  }

  const auto interactable_it = world.registry.interactables.find(target);
  return interactable_it == world.registry.interactables.end()
             ? ""
             : interactable_it->second.prompt_text;
}
