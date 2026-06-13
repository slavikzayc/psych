#pragma once

#include <string>
#include <vector>

struct MapData {
  int id = 0;

  std::string name;

  int width = 0;
  int height = 0;

  std::vector<std::string> layout;
};

struct SpawnPointData {
  int id = 0;

  int map_id = 0;

  std::string entity_type;

  int data_id = 0;

  int x = 0;
  int y = 0;

  int target_map_id = 0;
  int target_x = 0;
  int target_y = 0;

  bool locked = false;

  int required_key_item_id = 0;

  std::string prompt_text;

  std::string empty_prompt_text;
};
