#pragma once

#include <string>

struct ItemData {
  int id = 0;

  std::string name;

  std::string type;

  std::string description;

  int sanity_restore = 0;

  int dialogue_bonus = 0;

  bool consumable = false;
};
