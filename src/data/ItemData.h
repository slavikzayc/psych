#pragma once

#include <string>

struct ItemData {
  // Первичный ключ предмета. На него ссылаются ItemComponent и
  // spawn_points.json.
  int id = 0;

  // Название для инвентаря и сообщений.
  std::string name;

  // Тип предмета: medicine, key, document и т.п.
  std::string type;

  // Описание предмета. Для документов это фактически текст чтения.
  std::string description;

  // Насколько предмет восстанавливает рассудок при использовании.
  int sanity_restore = 0;

  // Бонус к следующей реплике в диалоговом бою.
  int dialogue_bonus = 0;

  // Если true, предмет удаляется из инвентаря после использования.
  bool consumable = false;
};
