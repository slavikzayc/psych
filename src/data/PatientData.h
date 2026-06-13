#pragma once

#include <string>

struct PatientData {
  int id = 0;

  std::string name;

  std::string archetype;

  int max_tension = 0;

  int resistance = 0;

  int dialogue_id = 0;

  int dialogue_cooldown_seconds = 120;
};
