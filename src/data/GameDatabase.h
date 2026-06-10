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
  GameConfigData config;

  std::unordered_map<int, ItemData> items;
  std::unordered_map<int, PatientData> patients;
  std::unordered_map<int, DialogueData> dialogues;
  std::unordered_map<int, DialogueNodeData> dialogue_nodes;
  std::unordered_map<int, std::vector<ReplyData>> replies_by_node;
  std::unordered_map<int, MapData> maps;
  std::vector<SpawnPointData> spawn_points;
  std::vector<NightEventData> night_events;

  std::string last_error;

  bool LoadAll(const std::string& assets_path);
};
