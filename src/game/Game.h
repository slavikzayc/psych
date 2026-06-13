#pragma once

#include <chrono>

#include "data/GameDatabase.h"
#include "game/WorldState.h"
#include "platform/Timer.h"
#include "systems/CollisionSystem.h"
#include "systems/DialogueCombatSystem.h"
#include "systems/HUDSystem.h"
#include "systems/InputSystem.h"
#include "systems/InteractionSystem.h"
#include "systems/InventorySystem.h"
#include "systems/MovementSystem.h"
#include "systems/NightEventSystem.h"
#include "systems/PatientSystem.h"
#include "systems/RenderSystem.h"
#include "systems/SanitySystem.h"

class Game {
 public:
  int Run();

 private:
  void Render();

  void Update(InputCommand command, float dt);

  void UpdateContinuousSystems(float dt);

  void StartNewGame();

  void BuildWorld();

  void ResetToMenu();

  void HandleMainMenu(InputCommand command);
  void HandleStatAllocation(InputCommand command);
  void HandleExploration(InputCommand command, float dt);
  void HandleInventory(InputCommand command);
  void HandleEndScreen(InputCommand command);

  int SelectedInventoryItemId() const;

  GameDatabase db_;

  WorldState world_;

  std::chrono::steady_clock::time_point last_frame_time_;

  int menu_selected_ = 0;

  int stat_selected_ = 0;

  int pending_authority_ = 1;
  int pending_medication_ = 1;
  int free_stat_points_ = 8;

  int inventory_selected_ = 0;

  float sanity_drain_accumulator_ = 0.0f;
  float night_time_accumulator_ = 0.0f;
  float patient_time_accumulator_ = 0.0f;
};
