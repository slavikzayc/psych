#include "game/Game.h"

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

namespace {
int DigitToIndex(InputCommand command) {
  switch (command) {
    case InputCommand::Digit1:
      return 0;
    case InputCommand::Digit2:
      return 1;
    case InputCommand::Digit3:
      return 2;
    case InputCommand::Digit4:
      return 3;
    case InputCommand::Digit5:
      return 4;
    case InputCommand::Digit6:
      return 5;
    case InputCommand::Digit7:
      return 6;
    case InputCommand::Digit8:
      return 7;
    case InputCommand::Digit9:
      return 8;
    default:
      return -1;
  }
}

char SymbolForItemType(const std::string &type) {
  if (type == "key") return 'k';
  if (type == "document") return 'n';
  return 'i';
}

}

int Game::Run() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
  if (output != INVALID_HANDLE_VALUE) {
    DWORD mode = 0;
    if (GetConsoleMode(output, &mode)) {
      SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
  }
  std::cout << "\x1B[?25l";

  if (!db_.LoadAll("assets")) {
    std::cout << "Data load error: " << db_.last_error << "\n\x1B[?25h";
    return 1;
  }
  ResetToMenu();

  last_frame_time_ = std::chrono::steady_clock::now();

  std::cout << "\x1B[2J\x1B[H" << std::flush;

  float render_accumulator = 1.0f;

  while (world_.running) {
    const float real_dt = RestartTimer(last_frame_time_);

    const float game_dt =
        std::clamp(real_dt * db_.config.time_scale, 0.0f, 1.0f);

    const InputCommand command = input_system::ReadCommandNonBlocking();
    Update(command, game_dt);

    render_accumulator += real_dt;
    if (command != InputCommand::None || render_accumulator >= 0.2f) {
      Render();
      render_accumulator = 0.0f;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  std::cout << "\x1B[2J\x1B[HGame ended.\n\x1B[?25h" << std::flush;
  return 0;
}

void Game::Render() {
  std::cout << "\x1B[H";

  std::string frame;
  switch (world_.mode) {
    case GameMode::MainMenu:
      frame = render_system::RenderMainMenu(menu_selected_);
      break;
    case GameMode::StatAllocation:
      frame = render_system::RenderStatAllocation(
          pending_authority_, pending_medication_, free_stat_points_,
          stat_selected_);
      break;
    case GameMode::Exploration:
      frame = render_system::RenderExploration(
          world_, db_, interaction_system::GetPrompt(world_));
      break;
    case GameMode::Inventory:
      frame = render_system::RenderInventory(world_, db_, inventory_selected_);
      break;
    case GameMode::DialogueCombat:
      frame = render_system::RenderDialogue(world_, db_);
      break;
    case GameMode::Victory:
      frame = render_system::RenderVictory(world_);
      break;
    case GameMode::Defeat:
      frame = render_system::RenderDefeat(world_);
      break;
  }

  for (char ch : frame) {
    if (ch == '\n') {
      std::cout << "\x1B[K\n";
    } else {
      std::cout << ch;
    }
  }

  std::cout << "\x1B[K\x1B[J" << std::flush;
}

void Game::Update(InputCommand command, float dt) {
  UpdateContinuousSystems(dt);

  if (world_.mode == GameMode::Victory || world_.mode == GameMode::Defeat) {
    if (command != InputCommand::None) {
      HandleEndScreen(command);
    }
    return;
  }

  switch (world_.mode) {
    case GameMode::MainMenu:
      HandleMainMenu(command);
      break;
    case GameMode::StatAllocation:
      HandleStatAllocation(command);
      break;
    case GameMode::Exploration:
      HandleExploration(command, dt);
      break;
    case GameMode::Inventory:
      HandleInventory(command);
      break;
    case GameMode::DialogueCombat:
      dialogue_combat_system::HandleInput(world_, db_, command);
      break;
    case GameMode::Victory:
    case GameMode::Defeat:
      HandleEndScreen(command);
      break;
  }
}

void Game::UpdateContinuousSystems(float dt) {
  night_event_system::Update(world_, db_, dt, night_time_accumulator_);
  if (world_.mode == GameMode::Victory) {
    return;
  }

  patient_system::Update(world_, dt, patient_time_accumulator_);

  sanity_system::UpdateSanity(world_, dt, sanity_drain_accumulator_);
}

void Game::ResetToMenu() {
  world_ = WorldState{};

  menu_selected_ = 0;
  stat_selected_ = 0;
  pending_authority_ = db_.config.stat_allocation.start_authority;
  pending_medication_ = db_.config.stat_allocation.start_medication;
  free_stat_points_ = db_.config.stat_allocation.free_points;
  inventory_selected_ = 0;

  sanity_drain_accumulator_ = 0.0f;
  night_time_accumulator_ = 0.0f;
  patient_time_accumulator_ = 0.0f;
}

void Game::HandleMainMenu(InputCommand command) {
  if (command == InputCommand::Up || command == InputCommand::Down) {
    menu_selected_ = 1 - menu_selected_;
  } else if (command == InputCommand::Confirm) {
    if (menu_selected_ == 0) {
      StartNewGame();
    } else {
      world_.running = false;
    }
  } else if (command == InputCommand::Cancel) {
    world_.running = false;
  }
}

void Game::StartNewGame() {
  world_.mode = GameMode::StatAllocation;
  world_.message = "Вы готовитесь к ночной смене.";
}

void Game::HandleStatAllocation(InputCommand command) {
  if (command == InputCommand::Up) {
    stat_selected_ = (stat_selected_ + 2) % 3;
  } else if (command == InputCommand::Down) {
    stat_selected_ = (stat_selected_ + 1) % 3;
  } else if (command == InputCommand::Left) {
    if (stat_selected_ == 0 &&
        pending_authority_ > db_.config.stat_allocation.start_authority) {
      --pending_authority_;
      ++free_stat_points_;
    } else if (stat_selected_ == 1 &&
               pending_medication_ >
                   db_.config.stat_allocation.start_medication) {
      --pending_medication_;
      ++free_stat_points_;
    }
  } else if (command == InputCommand::Right) {
    if (stat_selected_ == 0 &&
        pending_authority_ < db_.config.stat_allocation.max_stat &&
        free_stat_points_ > 0) {
      ++pending_authority_;
      --free_stat_points_;
    } else if (stat_selected_ == 1 &&
               pending_medication_ < db_.config.stat_allocation.max_stat &&
               free_stat_points_ > 0) {
      ++pending_medication_;
      --free_stat_points_;
    }
  } else if (command == InputCommand::Confirm &&
             (stat_selected_ == 2 || free_stat_points_ == 0)) {
    BuildWorld();
  } else if (command == InputCommand::Cancel) {
    world_.mode = GameMode::MainMenu;
  }
}

void Game::BuildWorld() {
  world_.registry = Registry{};

  world_.currentmap_id = db_.config.starting_map_id;
  world_.shift_time_seconds = 0;
  world_.shift_duration_seconds = db_.config.shift_duration_seconds;
  world_.phase = NightPhase::Early;
  sanity_drain_accumulator_ = 0.0f;
  night_time_accumulator_ = 0.0f;
  patient_time_accumulator_ = 0.0f;
  world_.message = "Ночная смена началась.";

  for (NightEventData &event : db_.night_events) {
    event.triggered = false;
  }

  for (const SpawnPointData &spawn : db_.spawn_points) {
    if (spawn.map_id == world_.currentmap_id && spawn.entity_type == "player") {
      const Entity player = CreateEntity(world_.registry);

      world_.registry.positions[player] = {spawn.x, spawn.y};
      world_.registry.maps[player] = {spawn.map_id};
      world_.registry.renderables[player] = {'@', 10};
      world_.registry.players[player] = {};
      world_.registry.stats[player] = {pending_authority_, pending_medication_};
      world_.registry.sanities[player] = {
          db_.config.sanity.initial, db_.config.sanity.max,
          db_.config.sanity.passive_drain_per_minute,
          db_.config.sanity.passive_drain_interval_seconds};
      world_.registry.inventories[player] = {};
      world_.player = player;
      break;
    }
  }

  for (const SpawnPointData &spawn : db_.spawn_points) {
    if (spawn.entity_type == "player") {
      continue;
    }

    const Entity entity = CreateEntity(world_.registry);
    world_.registry.positions[entity] = {spawn.x, spawn.y};
    world_.registry.maps[entity] = {spawn.map_id};

    if (spawn.entity_type == "item") {
      const char symbol = SymbolForItemType(db_.items.at(spawn.data_id).type);
      world_.registry.renderables[entity] = {symbol, 4};
      world_.registry.items[entity] = {spawn.data_id, true};
      world_.registry.interactables[entity] = {
          InteractionType::Pickup, spawn.prompt_text, spawn.empty_prompt_text};
    } else if (spawn.entity_type == "patient") {
      const PatientData &patient_data = db_.patients.at(spawn.data_id);
      const int max_tension = patient_data.max_tension;
      const int dialogue_id = patient_data.dialogue_id;
      const int cooldown = patient_data.dialogue_cooldown_seconds;
      world_.registry.renderables[entity] = {'p', 5};
      world_.registry.collisions[entity] = {true};
      world_.registry.patients[entity] = {spawn.data_id,
                                          PatientState::Calm,
                                          max_tension,
                                          max_tension,
                                          dialogue_id,
                                          cooldown,
                                          0};
      world_.registry.interactables[entity] = {InteractionType::StartDialogue,
                                               spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "door") {
      world_.registry.renderables[entity] = {'D', 3};
      world_.registry.collisions[entity] = {true};
      world_.registry.doors[entity] = {spawn.target_map_id, spawn.target_x,
                                       spawn.target_y, spawn.locked,
                                       spawn.required_key_item_id};
      world_.registry.interactables[entity] = {InteractionType::OpenDoor,
                                               spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "container") {
      world_.registry.renderables[entity] = {'c', 3};
      world_.registry.collisions[entity] = {true};
      world_.registry.items[entity] = {spawn.data_id, false};
      world_.registry.interactables[entity] = {InteractionType::SearchContainer,
                                               spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "document") {
      world_.registry.renderables[entity] = {'?', 3};
      world_.registry.items[entity] = {spawn.data_id, false};
      world_.registry.interactables[entity] = {InteractionType::ReadDocument,
                                               spawn.prompt_text,
                                               spawn.empty_prompt_text};
    }
  }

  world_.mode = GameMode::Exploration;
}

void Game::HandleExploration(InputCommand command, float dt) {
  if (command == InputCommand::Cancel) {
    world_.running = false;
    return;
  }
  if (command == InputCommand::Inventory) {
    inventory_selected_ = 0;
    world_.inventory_return_mode = GameMode::Exploration;
    world_.mode = GameMode::Inventory;
    return;
  }
  if (command == InputCommand::Interact) {
    interaction_system::Interact(world_, db_);
  } else {
    movement_system::Update(world_, db_, command);
  }

  (void)dt;
}

int Game::SelectedInventoryItemId() const {
  const InventoryComponent &inventory =
      world_.registry.inventories.at(world_.player);
  if (inventory.slots.empty()) {
    return 0;
  }
  const int clamped = std::clamp(inventory_selected_, 0,
                                 static_cast<int>(inventory.slots.size()) - 1);

  return inventory.slots[clamped].item_id;
}

void Game::HandleInventory(InputCommand command) {
  const int item_count = static_cast<int>(
      world_.registry.inventories.at(world_.player).slots.size());

  if (command == InputCommand::Cancel || command == InputCommand::Inventory) {
    world_.mode = world_.inventory_return_mode;
    return;
  }

  if (item_count <= 0) {
    return;
  }

  if (command == InputCommand::Up) {
    inventory_selected_ = (inventory_selected_ + item_count - 1) % item_count;
  } else if (command == InputCommand::Down) {
    inventory_selected_ = (inventory_selected_ + 1) % item_count;
  } else {
    const int digit_index = DigitToIndex(command);
    if (digit_index >= 0 && digit_index < item_count) {
      inventory_selected_ = digit_index;
      inventory_system::UseItem(world_, db_, world_.player,
                                SelectedInventoryItemId());
    } else if (command == InputCommand::Confirm ||
               command == InputCommand::Use) {
      inventory_system::UseItem(world_, db_, world_.player,
                                SelectedInventoryItemId());
    }
  }

  const int refreshed_count = static_cast<int>(
      world_.registry.inventories.at(world_.player).slots.size());
  if (refreshed_count > 0) {
    inventory_selected_ =
        std::clamp(inventory_selected_, 0, refreshed_count - 1);
  } else {
    inventory_selected_ = 0;
  }
}

void Game::HandleEndScreen(InputCommand command) {
  if (command == InputCommand::Confirm) {
    ResetToMenu();
  } else if (command == InputCommand::Cancel) {
    world_.running = false;
  }
}
