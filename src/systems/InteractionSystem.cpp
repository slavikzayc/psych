#include "systems/InteractionSystem.h"

#include <array>

#include "ecs/Registry.h"

namespace {

Entity FindInteractable(const WorldState &world) {
  const PositionComponent &pp = world.registry.positions.at(world.player);

  constexpr std::array<PositionComponent, 5> cells = {
      {{0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

  for (const PositionComponent &offset : cells) {
    const int x = pp.x + offset.x;
    const int y = pp.y + offset.y;

    for (const auto &[entity, interactable] : world.registry.interactables) {
      if (world.registry.maps.at(entity).map_id != world.currentmap_id)
        continue;

      const PositionComponent &position = world.registry.positions.at(entity);
      if (position.x != x || position.y != y) continue;

      if (interactable.type == InteractionType::Pickup && offset.x == 0 &&
          offset.y == 0) {
        return entity;
      }

      if (interactable.type != InteractionType::Pickup &&
          (offset.x != 0 || offset.y != 0)) {
        return entity;
      }
    }
  }
  return 0;
}

}

void interaction_system::Interact(WorldState &world, const GameDatabase &db) {
  const Entity target = FindInteractable(world);
  if (target == 0) {
    world.message = "Рядом нет ничего, с чем можно взаимодействовать.";
    return;
  }

  const InteractableComponent &interactable =
      world.registry.interactables.at(target);

  switch (interactable.type) {
    case InteractionType::Pickup: {
      const ItemComponent &item = world.registry.items.at(target);
      inventory_system::AddItem(world, world.player, item.item_id, 1);
      world.message = "Подобрано: " + db.items.at(item.item_id).name + ".";
      DestroyEntity(world.registry, target);
      break;
    }
    case InteractionType::OpenDoor: {
      DoorComponent &door = world.registry.doors.at(target);
      if (door.locked && door.required_key_item_id > 0 &&
          !inventory_system::HasItem(world, world.player,
                                     door.required_key_item_id)) {
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
        world.message = "Дверь открыта. За ней пахнет хлоркой и пылью.";
      }
      break;
    }
    case InteractionType::StartDialogue: {
      PatientComponent &patient = world.registry.patients.at(target);
      if (patient.state == PatientState::Treated) {
        world.message =
            "Пациент пока стабилен. Дайте ему немного "
            "времени.";
        break;
      }
      const DialogueData &dialogue = db.dialogues.at(patient.dialogue_id);
      patient.state = PatientState::InDialogue;

      patient.current_tension = patient.current_tension <= 0
                                    ? patient.max_tension
                                    : patient.current_tension;
      world.active_patient = target;

      world.registry.dialogue_combats[target] = {
          patient.dialogue_id, dialogue.start_node_id, target, true, 0};
      world.mode = GameMode::DialogueCombat;
      world.message = "Вы осторожно начинаете разговор.";
      break;
    }
    case InteractionType::SearchContainer: {
      const auto item_it = world.registry.items.find(target);
      if (item_it != world.registry.items.end() &&
          item_it->second.item_id > 0) {
        inventory_system::AddItem(world, world.player, item_it->second.item_id,
                                  1);
        world.message =
            "В шкафчике найдено: " + db.items.at(item_it->second.item_id).name +
            ".";
        world.registry.items.erase(target);
        InteractableComponent &container_interactable =
            world.registry.interactables[target];
        if (!container_interactable.empty_prompt_text.empty()) {
          container_interactable.prompt_text =
              container_interactable.empty_prompt_text;
        }
      } else {
        world.message = "Шкафчик пуст.";
      }
      break;
    }
    case InteractionType::ReadDocument: {
      const ItemComponent &item = world.registry.items.at(target);
      world.message = db.items.at(item.item_id).description;
      break;
    }
  }
}

std::string interaction_system::GetPrompt(const WorldState &world) {
  const Entity target = FindInteractable(world);
  if (target == 0) {
    return "";
  }

  return world.registry.interactables.at(target).prompt_text;
}
