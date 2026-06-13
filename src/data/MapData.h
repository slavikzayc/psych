#pragma once

#include <string>
#include <vector>

struct MapData {
  // id карты. На него ссылаются WorldState::currentmap_id и
  // SpawnPointData::map_id.
  int id = 0;

  // Название комнаты для верхней строки интерфейса.
  std::string name;

  // Размеры карты. Используются CollisionSystem для проверки границ.
  int width = 0;
  int height = 0;

  // ASCII-layout базовых тайлов. '#' — стена, '.' — пол.
  // Игровые сущности поверх layout создаются отдельно через spawn_points.json.
  std::vector<std::string> layout;
};

struct SpawnPointData {
  // id spawn-точки.
  int id = 0;

  // Карта, на которой появляется объект.
  int map_id = 0;

  // Тип создаваемой сущности: player, item, patient, door, container, document.
  std::string entity_type;

  // id данных, к которым относится объект: item_id, patient_id или ключ двери.
  int data_id = 0;

  // Координаты появления на карте.
  int x = 0;
  int y = 0;

  // Для дверей: куда переносить игрока.
  int target_map_id = 0;
  int target_x = 0;
  int target_y = 0;

  // Для дверей: заперта ли дверь.
  bool locked = false;

  // Для дверей: id ключа из items.json.
  int required_key_item_id = 0;

  // Текст подсказки взаимодействия.
  std::string prompt_text;

  // Альтернативная подсказка после опустошения контейнера.
  std::string empty_prompt_text;
};
