#pragma once

#include <string>

#include "ecs/Registry.h"
#include "game/GameState.h"

struct WorldState {
  Registry registry;

  GameMode mode = GameMode::MainMenu;
  GameMode inventory_return_mode = GameMode::Exploration;
  NightPhase phase = NightPhase::Early;

  int currentmap_id = 1;
  int shift_time_seconds = 0;
  int shift_duration_seconds = 900;

  Entity player = 0;
  Entity active_patient = 0;

  bool running = true;
  bool dev_mode = false;

  std::string message;
};
