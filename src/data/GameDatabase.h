#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "data/DialogueData.h"
#include "data/GameConfigData.h"
#include "data/ItemData.h"
#include "data/MapData.h"
#include "data/NightEventData.h"
#include "data/PatientData.h"

struct GameDatabase {
  // Общие настройки игры из game_config.json.
  GameConfigData config;

  // Таблицы данных. Ключ unordered_map — id записи из JSON.
  std::unordered_map<int, ItemData> items;
  std::unordered_map<int, PatientData> patients;
  std::unordered_map<int, DialogueData> dialogues;
  std::unordered_map<int, DialogueNodeData> dialogue_nodes;

  // Реплики сгруппированы по node_id, потому что в диалоге нужно быстро
  // получить все ответы для текущего узла.
  std::unordered_map<int, std::vector<ReplyData>> replies_by_node;

  std::unordered_map<int, MapData> maps;

  // Spawn-точки и события хранятся в vector, потому что они обрабатываются
  // проходом.
  std::vector<SpawnPointData> spawn_points;
  std::vector<NightEventData> night_events;

  // Текст последней ошибки загрузки. Используется Game::Run при неудачном
  // старте.
  std::string last_error;

  // Загружает все JSON-файлы из папки assets_path.
  bool LoadAll(const std::string &assets_path);
};
