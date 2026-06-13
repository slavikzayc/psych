#pragma once

#include <string>

struct NightEventData {
  int id = 0;

  std::string phase;

  int trigger_time = 0;

  std::string event_type;

  int target_id = 0;

  std::string message;

  bool triggered = false;
};
