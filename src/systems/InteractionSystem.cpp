#include "systems/InteractionSystem.h"

#include <array>

#include "ecs/Registry.h"

namespace {

Entity FindInteractable(const WorldState &world) {
  // Берём позицию игрока как центр поиска интерактивных объектов.
  const auto player_pos_it = world.registry.positions.find(world.player);
  if (player_pos_it == world.registry.positions.end())
    return 0;
  const PositionComponent &pp = player_pos_it->second;

  // Проверяются пять клеток: текущая клетка игрока и четыре соседние.
  constexpr std::array<PositionComponent, 5> cells = {
      {{0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

  for (const PositionComponent &offset : cells) {
    const int x = pp.x + offset.x;
    const int y = pp.y + offset.y;

    for (const auto &[entity, interactable] : world.registry.interactables) {
      // Объекты с других карт игнорируются.
      const auto map_it = world.registry.maps.find(entity);
      if (map_it != world.registry.maps.end() &&
          map_it->second.map_id != world.currentmap_id)
        continue;

      // Без PositionComponent объект нельзя сопоставить с клеткой карты.
      const auto pos_it = world.registry.positions.find(entity);
      if (pos_it == world.registry.positions.end())
        continue;
      if (pos_it->second.x != x || pos_it->second.y != y)
        continue;

      // Предметы подбираются только когда игрок стоит прямо на клетке предмета.
      if (interactable.type == InteractionType::Pickup && offset.x == 0 &&
          offset.y == 0) {
        return entity;
      }
      // Остальные интерактивные объекты активируются из соседней клетки.
      if (interactable.type != InteractionType::Pickup &&
          (offset.x != 0 || offset.y != 0)) {
        return entity;
      }
    }
  }
  return 0;
}

} // namespace

void interaction_system::Interact(WorldState &world, const GameDatabase &db) {
  // Находим ближайший объект, доступный для взаимодействия.
  const Entity target = FindInteractable(world);
  if (target == 0) {
    world.message = "Рядом нет ничего, с чем можно взаимодействовать.";
    return;
  }

  // Тип взаимодействия хранится в InteractableComponent.
  const auto interactable_it = world.registry.interactables.find(target);
  if (interactable_it == world.registry.interactables.end()) {
    return;
  }

  switch (interactable_it->second.type) {
  case InteractionType::Pickup: {
    // Pickup: предмет добавляется в инвентарь, а entity удаляется с карты.
    const auto item_it = world.registry.items.find(target);
    if (item_it == world.registry.items.end()) {
      break;
    }
    inventory_system::AddItem(world, world.player, item_it->second.item_id, 1);
    const auto data_it = db.items.find(item_it->second.item_id);
    world.message = "Подобрано: " +
                    (data_it == db.items.end() ? std::string("предмет")
                                               : data_it->second.name) +
                    ".";
    DestroyEntity(world.registry, target);
    break;
  }
  case InteractionType::OpenDoor: {
    // OpenDoor: проверяем замок, ключ и возможный переход на другую карту.
    auto door_it = world.registry.doors.find(target);
    if (door_it == world.registry.doors.end()) {
      break;
    }
    DoorComponent &door = door_it->second;
    if (door.locked && door.required_key_item_id > 0 &&
        !inventory_system::HasItem(world, world.player,
                                   door.required_key_item_id)) {
      world.message = "Дверь заперта. Нужен ключ.";
      break;
    }
    door.locked = false;
    if (door.target_map_id > 0) {
      // Если target_map_id задан, дверь работает как переход между комнатами.
      world.currentmap_id = door.target_map_id;
      world.registry.maps[world.player] = {door.target_map_id};
      world.registry.positions[world.player] = {door.target_x, door.target_y};
      world.message = "Вы переходите в другую комнату.";
    } else {
      // Если target_map_id не задан, дверь просто открывается в этой же
      // комнате.
      world.registry.collisions.erase(target);
      world.registry.renderables[target].symbol = '/';
      world.message = "Дверь открыта. За ней пахнет хлоркой и пылью.";
    }
    break;
  }
  case InteractionType::StartDialogue: {
    // StartDialogue: создаём DialogueCombatComponent на пациенте и меняем режим
    // игры.
    auto patient_it = world.registry.patients.find(target);
    if (patient_it == world.registry.patients.end()) {
      break;
    }
    PatientComponent &patient = patient_it->second;
    if (patient.state == PatientState::Treated) {
      world.message = "Пациент пока стабилен. Дайте ему немного "
                      "времени.";
      break;
    }
    const auto dialogue_it = db.dialogues.find(patient.dialogue_id);
    if (dialogue_it == db.dialogues.end()) {
      world.message = "У пациента нет доступного диалога.";
      break;
    }
    patient.state = PatientState::InDialogue;
    // Если напряжение уже было сброшено, перед новым разговором возвращаем
    // максимум.
    patient.current_tension = patient.current_tension <= 0
                                  ? patient.max_tension
                                  : patient.current_tension;
    world.active_patient = target;
    // Компонент диалогового боя хранит текущий узел и ссылку на пациента.
    world.registry.dialogue_combats[target] = {
        patient.dialogue_id, dialogue_it->second.start_node_id, target, true,
        0};
    world.mode = GameMode::DialogueCombat;
    world.message = "Вы осторожно начинаете разговор.";
    break;
  }
  case InteractionType::SearchContainer: {
    // SearchContainer: контейнер выдаёт предмет один раз, затем становится
    // пустым.
    const auto item_it = world.registry.items.find(target);
    if (item_it != world.registry.items.end() && item_it->second.item_id > 0) {
      inventory_system::AddItem(world, world.player, item_it->second.item_id,
                                1);
      const auto data_it = db.items.find(item_it->second.item_id);
      world.message = "В шкафчике найдено: " +
                      (data_it == db.items.end() ? std::string("предмет")
                                                 : data_it->second.name) +
                      ".";
      world.registry.items.erase(target);
      InteractableComponent &interactable =
          world.registry.interactables[target];
      if (!interactable.empty_prompt_text.empty()) {
        interactable.prompt_text = interactable.empty_prompt_text;
      }
    } else {
      world.message = "Шкафчик пуст.";
    }
    break;
  }
  case InteractionType::ReadDocument: {
    // ReadDocument: берём description связанного ItemData и показываем его как
    // сообщение.
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

std::string interaction_system::GetPrompt(const WorldState &world) {
  // HUD вызывает эту функцию, чтобы понять, нужно ли показывать строку "F -
  // ...".
  const Entity target = FindInteractable(world);
  if (target == 0) {
    return "";
  }

  const auto interactable_it = world.registry.interactables.find(target);
  return interactable_it == world.registry.interactables.end()
             ? ""
             : interactable_it->second.prompt_text;
}
