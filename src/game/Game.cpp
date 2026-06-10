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

char SymbolForItemType(const std::string& type) {
  if (type == "key") return 'k';
  if (type == "document") return 'n';
  return 'i';
}

void InitTerminal() {
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
}

void ClearScreen() { std::cout << "\x1B[2J\x1B[H" << std::flush; }

void BeginFrame() { std::cout << "\x1B[H"; }

void EndFrame() { std::cout << "\x1B[J" << std::flush; }

void ShowCursor() { std::cout << "\x1B[?25h" << std::flush; }

void WriteTerminal(const std::string& text) {
  for (char ch : text) {
    if (ch == '\n') {
      std::cout << "\x1B[K\n";
    } else {
      std::cout << ch;
    }
  }
}
}  // namespace

int Game::Run() {
  InitTerminal();
  if (!db_.LoadAll("assets")) {
    WriteTerminal("Ошибка загрузки данных: " + db_.last_error + "\n");
    ShowCursor();
    return 1;
  }
  ResetToMenu();

  timer_.Restart();
  ClearScreen();
  float render_accumulator = 1.0f;
  while (world_.running) {
    const float real_dt = timer_.Restart();
    const float game_dt = std::clamp(real_dt * db_.config.time_scale, 0.0f, 1.0f);
    const InputCommand command = input_.ReadCommandNonBlocking();
    Update(command, game_dt);

    render_accumulator += real_dt;
    if (command != InputCommand::None || render_accumulator >= 0.2f) {
      Render();
      render_accumulator = 0.0f;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
  ClearScreen();
  WriteTerminal("До следующей смены.\n");
  ShowCursor();
  return 0;
}

void Game::Render() {
  BeginFrame();
  switch (world_.mode) {
    case GameMode::MainMenu:
      WriteTerminal(render_.RenderMainMenu(menu_selected_));
      break;
    case GameMode::StatAllocation:
      WriteTerminal(render_.RenderStatAllocation(pending_authority_, pending_medication_,
                                                 free_stat_points_, stat_selected_));
      break;
    case GameMode::Exploration:
      WriteTerminal(render_.RenderExploration(world_, db_, interaction_.GetPrompt(world_), hud_));
      break;
    case GameMode::Inventory:
      WriteTerminal(render_.RenderInventory(world_, db_, inventory_selected_));
      break;
    case GameMode::DialogueCombat:
      WriteTerminal(render_.RenderDialogue(world_, db_));
      break;
    case GameMode::Victory:
      WriteTerminal(render_.RenderVictory(world_));
      break;
    case GameMode::Defeat:
      WriteTerminal(render_.RenderDefeat(world_));
      break;
  }
  EndFrame();
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
      dialogue_.HandleInput(world_, db_, command);
      break;
    case GameMode::Victory:
    case GameMode::Defeat:
      HandleEndScreen(command);
      break;
  }
}

void Game::UpdateContinuousSystems(float dt) {
  night_events_.Update(world_, db_, dt);
  if (world_.mode == GameMode::Victory) {
    return;
  }

  patients_.Update(world_, dt);
  sanity_.UpdateSanity(world_, dt);
}

void Game::ResetToMenu() {
  world_ = WorldState{};
  menu_selected_ = 0;
  stat_selected_ = 0;
  pending_authority_ = db_.config.stat_allocation.start_authority;
  pending_medication_ = db_.config.stat_allocation.start_medication;
  free_stat_points_ = db_.config.stat_allocation.free_points;
  inventory_selected_ = 0;
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
    if (stat_selected_ == 0 && pending_authority_ > db_.config.stat_allocation.start_authority) {
      --pending_authority_;
      ++free_stat_points_;
    } else if (stat_selected_ == 1 &&
               pending_medication_ > db_.config.stat_allocation.start_medication) {
      --pending_medication_;
      ++free_stat_points_;
    }
  } else if (command == InputCommand::Right) {
    if (stat_selected_ == 0 && pending_authority_ < db_.config.stat_allocation.max_stat &&
        free_stat_points_ > 0) {
      ++pending_authority_;
      --free_stat_points_;
    } else if (stat_selected_ == 1 && pending_medication_ < db_.config.stat_allocation.max_stat &&
               free_stat_points_ > 0) {
      ++pending_medication_;
      --free_stat_points_;
    }
  } else if (command == InputCommand::Confirm && (stat_selected_ == 2 || free_stat_points_ == 0)) {
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
  world_.message = "Ночная смена началась.";

  for (NightEventData& event : db_.night_events) {
    event.triggered = false;
  }

  Entity player = 0;
  for (const SpawnPointData& spawn : db_.spawn_points) {
    if (spawn.map_id == world_.currentmap_id && spawn.entity_type == "player") {
      player = CreateEntity(world_.registry);
      world_.registry.positions[player] = {spawn.x, spawn.y};
      world_.registry.maps[player] = {spawn.map_id};
      world_.registry.renderables[player] = {'@', 15, 10};
      world_.registry.players[player] = {};
      world_.registry.stats[player] = {pending_authority_, pending_medication_};
      world_.registry.sanities[player] = {db_.config.sanity.initial, db_.config.sanity.max,
                                          db_.config.sanity.passive_drain_per_minute,
                                          db_.config.sanity.passive_drain_interval_seconds};
      world_.registry.inventories[player] = {};
      world_.player = player;
      break;
    }
  }

  if (player == 0) {
    player = CreateEntity(world_.registry);
    world_.registry.positions[player] = {1, 1};
    world_.registry.maps[player] = {world_.currentmap_id};
    world_.registry.renderables[player] = {'@', 15, 10};
    world_.registry.players[player] = {};
    world_.registry.stats[player] = {pending_authority_, pending_medication_};
    world_.registry.sanities[player] = {db_.config.sanity.initial, db_.config.sanity.max,
                                        db_.config.sanity.passive_drain_per_minute,
                                        db_.config.sanity.passive_drain_interval_seconds};
    world_.registry.inventories[player] = {};
    world_.player = player;
  }

  for (const SpawnPointData& spawn : db_.spawn_points) {
    if (spawn.entity_type == "player") {
      continue;
    }

    const Entity entity = CreateEntity(world_.registry);
    world_.registry.positions[entity] = {spawn.x, spawn.y};
    world_.registry.maps[entity] = {spawn.map_id};

    if (spawn.entity_type == "item") {
      const auto item_it = db_.items.find(spawn.data_id);
      const char symbol =
          item_it == db_.items.end() ? 'i' : SymbolForItemType(item_it->second.type);
      world_.registry.renderables[entity] = {symbol, 10, 4};
      world_.registry.items[entity] = {spawn.data_id, true};
      world_.registry.interactables[entity] = {InteractionType::Pickup, spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "patient") {
      const auto patient_it = db_.patients.find(spawn.data_id);
      const int max_tension =
          patient_it == db_.patients.end() ? 20 : patient_it->second.max_tension;
      const int dialogue_id = patient_it == db_.patients.end() ? 0 : patient_it->second.dialogue_id;
      const int cooldown =
          patient_it == db_.patients.end() ? 120 : patient_it->second.dialogue_cooldown_seconds;
      world_.registry.renderables[entity] = {'p', 12, 5};
      world_.registry.collisions[entity] = {true};
      world_.registry.patients[entity] = {
          spawn.data_id, PatientState::Calm, max_tension, max_tension, dialogue_id, cooldown, 0};
      world_.registry.interactables[entity] = {InteractionType::StartDialogue, spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "door") {
      world_.registry.renderables[entity] = {'D', 14, 3};
      world_.registry.collisions[entity] = {true};
      world_.registry.doors[entity] = {spawn.target_map_id, spawn.target_x, spawn.target_y,
                                       spawn.locked, spawn.required_key_item_id};
      world_.registry.interactables[entity] = {InteractionType::OpenDoor, spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "container") {
      world_.registry.renderables[entity] = {'c', 11, 3};
      world_.registry.collisions[entity] = {true};
      world_.registry.items[entity] = {spawn.data_id, false};
      world_.registry.interactables[entity] = {InteractionType::SearchContainer, spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "document") {
      world_.registry.renderables[entity] = {'?', 7, 3};
      world_.registry.items[entity] = {spawn.data_id, false};
      world_.registry.interactables[entity] = {InteractionType::ReadDocument, spawn.prompt_text,
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
    interaction_.Interact(world_, db_, inventory_);
  } else {
    movement_.Update(world_, db_, collision_, command);
  }

  (void)dt;
}

int Game::SelectedInventoryItemId() const {
  const auto inv_it = world_.registry.inventories.find(world_.player);
  if (inv_it == world_.registry.inventories.end() || inv_it->second.slots.empty()) {
    return 0;
  }
  const int clamped =
      std::clamp(inventory_selected_, 0, static_cast<int>(inv_it->second.slots.size()) - 1);
  return inv_it->second.slots[clamped].item_id;
}

void Game::HandleInventory(InputCommand command) {
  const auto inv_it = world_.registry.inventories.find(world_.player);
  const int item_count = inv_it == world_.registry.inventories.end()
                             ? 0
                             : static_cast<int>(inv_it->second.slots.size());

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
      inventory_.UseItem(world_, db_, world_.player, SelectedInventoryItemId());
    } else if (command == InputCommand::Confirm || command == InputCommand::Use) {
      inventory_.UseItem(world_, db_, world_.player, SelectedInventoryItemId());
    }
  }

  const auto refreshedinv_it = world_.registry.inventories.find(world_.player);
  const int refreshed_count = refreshedinv_it == world_.registry.inventories.end()
                                  ? 0
                                  : static_cast<int>(refreshedinv_it->second.slots.size());
  if (refreshed_count > 0) {
    inventory_selected_ = std::clamp(inventory_selected_, 0, refreshed_count - 1);
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
