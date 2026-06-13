#include "game/Game.h"

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

namespace {
int DigitToIndex(InputCommand command) {
  // Игрок нажимает цифры 1-9, а массивы в C++ индексируются с 0.
  // Поэтому Digit1 становится индексом 0, Digit2 — индексом 1 и т.д.
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
  // В JSON предметы имеют строковый тип.
  // Здесь тип превращается в короткий ASCII-символ для карты.
  if (type == "key")
    return 'k';
  if (type == "document")
    return 'n';
  return 'i';
}

} // namespace

int Game::Run() {
  // Настройка Windows-консоли для UTF-8 и ANSI escape-кодов.
  // UTF-8 нужен для русских строк, ANSI-коды — для перерисовки без мерцания.
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

  // Загружаем все геймдизайнерские данные до показа меню.
  // Если загрузка не удалась, дальше запускать игру нельзя.
  if (!db_.LoadAll("assets")) {
    std::cout << "Data load error: " << db_.last_error << "\n\x1B[?25h";
    return 1;
  }
  ResetToMenu();

  // Инициализируем точку отсчёта времени для первого кадра.
  last_frame_time_ = std::chrono::steady_clock::now();

  // Первичная очистка экрана. Дальше Render будет обновлять кадр без cls.
  std::cout << "\x1B[2J\x1B[H" << std::flush;

  // Отдельный аккумулятор отрисовки позволяет не перерисовывать консоль слишком
  // часто.
  float render_accumulator = 1.0f;

  // Главный цикл приложения.
  // Он крутится независимо от ввода, поэтому время и рассудок идут сами по
  // себе.
  while (world_.running) {
    // real_dt — реальное время между кадрами.
    const float real_dt = RestartTimer(last_frame_time_);

    // game_dt — игровое время после масштабирования из game_config.json.
    const float game_dt =
        std::clamp(real_dt * db_.config.time_scale, 0.0f, 1.0f);

    // Ввод неблокирующий: если игрок ничего не нажал, вернётся None.
    const InputCommand command = input_system::ReadCommandNonBlocking();
    Update(command, game_dt);

    // Рендерим сразу после ввода или примерно раз в 0.2 секунды.
    // Это сохраняет обновление таймера без постоянного мигания консоли.
    render_accumulator += real_dt;
    if (command != InputCommand::None || render_accumulator >= 0.2f) {
      Render();
      render_accumulator = 0.0f;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  // Перед выходом очищаем экран и возвращаем курсор.
  std::cout << "\x1B[2J\x1B[HGame ended.\n\x1B[?25h" << std::flush;
  return 0;
}

void Game::Render() {
  // Возвращаем курсор в начало экрана, но не делаем cls.
  // Так предыдущий кадр перезаписывается без сильного мерцания.
  std::cout << "\x1B[H";

  // Кадр сначала собирается в строку, чтобы потом вывести его единообразно.
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

  // Каждая строка заканчивается очисткой остатка строки.
  // Это убирает "хвосты" старых длинных строк справа от карты и заголовков.
  for (char ch : frame) {
    if (ch == '\n') {
      std::cout << "\x1B[K\n";
    } else {
      std::cout << ch;
    }
  }

  // Очищаем хвост текущей строки и всё ниже последней строки кадра.
  std::cout << "\x1B[K\x1B[J" << std::flush;
}

void Game::Update(InputCommand command, float dt) {
  // Эти системы обновляются всегда, пока режим это разрешает.
  // Поэтому таймер смены и рассудок не зависят от движения игрока.
  UpdateContinuousSystems(dt);

  // На экранах победы/поражения обычная игровая логика уже не нужна.
  if (world_.mode == GameMode::Victory || world_.mode == GameMode::Defeat) {
    if (command != InputCommand::None) {
      HandleEndScreen(command);
    }
    return;
  }

  // Основная диспетчеризация ввода по текущему режиму.
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
  // Система ночи двигает общий таймер смены и запускает события.
  night_event_system::Update(world_, db_, dt, night_time_accumulator_);
  if (world_.mode == GameMode::Victory) {
    return;
  }

  // Система пациентов отсчитывает время до повторной доступности диалога.
  patient_system::Update(world_, dt, patient_time_accumulator_);

  // Система рассудка пассивно снижает sanity по таймеру.
  sanity_system::UpdateSanity(world_, dt, sanity_drain_accumulator_);
}

void Game::ResetToMenu() {
  // Новый WorldState сбрасывает ECS, режим, текущую карту и сообщение.
  world_ = WorldState{};

  // Сбрасываем UI-выбор и стартовые значения статов из конфигурации.
  menu_selected_ = 0;
  stat_selected_ = 0;
  pending_authority_ = db_.config.stat_allocation.start_authority;
  pending_medication_ = db_.config.stat_allocation.start_medication;
  free_stat_points_ = db_.config.stat_allocation.free_points;
  inventory_selected_ = 0;

  // Сбрасываем дробные накопители времени, чтобы новая игра не наследовала
  // старые тики.
  sanity_drain_accumulator_ = 0.0f;
  night_time_accumulator_ = 0.0f;
  patient_time_accumulator_ = 0.0f;
}

void Game::HandleMainMenu(InputCommand command) {
  // В меню всего два пункта, поэтому Up/Down просто переключают 0 <-> 1.
  if (command == InputCommand::Up || command == InputCommand::Down) {
    menu_selected_ = 1 - menu_selected_;
  } else if (command == InputCommand::Confirm) {
    // Пункт 0 — начать игру, пункт 1 — выход.
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
  // Сама карта ещё не создаётся: сначала игрок должен распределить
  // характеристики.
  world_.mode = GameMode::StatAllocation;
  world_.message = "Вы готовитесь к ночной смене.";
}

void Game::HandleStatAllocation(InputCommand command) {
  // На экране есть три строки: Авторитет, Медикация, Начать смену.
  if (command == InputCommand::Up) {
    stat_selected_ = (stat_selected_ + 2) % 3;
  } else if (command == InputCommand::Down) {
    stat_selected_ = (stat_selected_ + 1) % 3;
  } else if (command == InputCommand::Left) {
    // Влево уменьшает выбранную характеристику и возвращает очко в пул.
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
    // Вправо увеличивает выбранную характеристику, если есть свободные очки.
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
    // Игру можно начать либо с пункта "Начать смену", либо когда очки
    // закончились.
    BuildWorld();
  } else if (command == InputCommand::Cancel) {
    world_.mode = GameMode::MainMenu;
  }
}

void Game::BuildWorld() {
  // Полностью пересоздаём Registry: новая игра должна начинаться без старых
  // сущностей.
  world_.registry = Registry{};

  // Базовые параметры смены берутся из game_config.json.
  world_.currentmap_id = db_.config.starting_map_id;
  world_.shift_time_seconds = 0;
  world_.shift_duration_seconds = db_.config.shift_duration_seconds;
  world_.phase = NightPhase::Early;
  sanity_drain_accumulator_ = 0.0f;
  night_time_accumulator_ = 0.0f;
  patient_time_accumulator_ = 0.0f;
  world_.message = "Ночная смена началась.";

  // Ночные события хранят флаг triggered прямо в данных,
  // поэтому при старте новой смены их нужно снова сделать неактивированными.
  for (NightEventData &event : db_.night_events) {
    event.triggered = false;
  }

  // Сначала ищем spawn-точку игрока на стартовой карте.
  // Игрок создаётся отдельно, потому что его Entity нужен почти всем системам.
  Entity player = 0;
  for (const SpawnPointData &spawn : db_.spawn_points) {
    if (spawn.map_id == world_.currentmap_id && spawn.entity_type == "player") {
      player = CreateEntity(world_.registry);

      // Дальше к одному Entity добавляется набор компонентов:
      // позиция, карта, отрисовка, тег игрока, статы, рассудок и инвентарь.
      world_.registry.positions[player] = {spawn.x, spawn.y};
      world_.registry.maps[player] = {spawn.map_id};
      world_.registry.renderables[player] = {'@', 15, 10};
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

  if (player == 0) {
    // Защитный fallback: если геймдизайнер случайно удалил spawn игрока,
    // игра всё равно стартует в координатах (1, 1).
    player = CreateEntity(world_.registry);
    world_.registry.positions[player] = {1, 1};
    world_.registry.maps[player] = {world_.currentmap_id};
    world_.registry.renderables[player] = {'@', 15, 10};
    world_.registry.players[player] = {};
    world_.registry.stats[player] = {pending_authority_, pending_medication_};
    world_.registry.sanities[player] = {
        db_.config.sanity.initial, db_.config.sanity.max,
        db_.config.sanity.passive_drain_per_minute,
        db_.config.sanity.passive_drain_interval_seconds};
    world_.registry.inventories[player] = {};
    world_.player = player;
  }

  for (const SpawnPointData &spawn : db_.spawn_points) {
    // Игрок уже создан выше, поэтому второй раз его не создаём.
    if (spawn.entity_type == "player") {
      continue;
    }

    // Для любого объекта сначала создаётся Entity и базовые компоненты
    // позиции/карты.
    const Entity entity = CreateEntity(world_.registry);
    world_.registry.positions[entity] = {spawn.x, spawn.y};
    world_.registry.maps[entity] = {spawn.map_id};

    if (spawn.entity_type == "item") {
      // Обычный предмет на полу: его можно подобрать, поэтому есть
      // ItemComponent и InteractableComponent типа Pickup.
      const auto item_it = db_.items.find(spawn.data_id);
      const char symbol = item_it == db_.items.end()
                              ? 'i'
                              : SymbolForItemType(item_it->second.type);
      world_.registry.renderables[entity] = {symbol, 10, 4};
      world_.registry.items[entity] = {spawn.data_id, true};
      world_.registry.interactables[entity] = {
          InteractionType::Pickup, spawn.prompt_text, spawn.empty_prompt_text};
    } else if (spawn.entity_type == "patient") {
      // Пациент получает состояние, скрытую шкалу напряжения, коллизию и
      // действие "начать диалог".
      const auto patient_it = db_.patients.find(spawn.data_id);
      const int max_tension = patient_it == db_.patients.end()
                                  ? 20
                                  : patient_it->second.max_tension;
      const int dialogue_id =
          patient_it == db_.patients.end() ? 0 : patient_it->second.dialogue_id;
      const int cooldown = patient_it == db_.patients.end()
                               ? 120
                               : patient_it->second.dialogue_cooldown_seconds;
      world_.registry.renderables[entity] = {'p', 12, 5};
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
      // Дверь блокирует движение и при взаимодействии либо переносит на другую
      // карту, либо открывается, если target_map_id равен 0.
      world_.registry.renderables[entity] = {'D', 14, 3};
      world_.registry.collisions[entity] = {true};
      world_.registry.doors[entity] = {spawn.target_map_id, spawn.target_x,
                                       spawn.target_y, spawn.locked,
                                       spawn.required_key_item_id};
      world_.registry.interactables[entity] = {InteractionType::OpenDoor,
                                               spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "container") {
      // Контейнер не подбирается сам, но содержит item_id в ItemComponent.
      // После обыска item удаляется из контейнера.
      world_.registry.renderables[entity] = {'c', 11, 3};
      world_.registry.collisions[entity] = {true};
      world_.registry.items[entity] = {spawn.data_id, false};
      world_.registry.interactables[entity] = {InteractionType::SearchContainer,
                                               spawn.prompt_text,
                                               spawn.empty_prompt_text};
    } else if (spawn.entity_type == "document") {
      // Документ показывает description предмета, но не добавляется в
      // инвентарь.
      world_.registry.renderables[entity] = {'?', 7, 3};
      world_.registry.items[entity] = {spawn.data_id, false};
      world_.registry.interactables[entity] = {InteractionType::ReadDocument,
                                               spawn.prompt_text,
                                               spawn.empty_prompt_text};
    }
  }

  // После создания сущностей можно переходить на основной экран исследования.
  world_.mode = GameMode::Exploration;
}

void Game::HandleExploration(InputCommand command, float dt) {
  // Esc из исследования завершает приложение.
  if (command == InputCommand::Cancel) {
    world_.running = false;
    return;
  }
  if (command == InputCommand::Inventory) {
    // Инвентарь запоминает, что возвращаться нужно именно в Exploration.
    inventory_selected_ = 0;
    world_.inventory_return_mode = GameMode::Exploration;
    world_.mode = GameMode::Inventory;
    return;
  }
  if (command == InputCommand::Interact) {
    // F запускает систему взаимодействия: подбор, двери, пациенты, контейнеры.
    interaction_system::Interact(world_, db_);
  } else {
    // Все остальные команды передаются движению; MovementSystem сам игнорирует
    // не-движение.
    movement_system::Update(world_, db_, command);
  }

  // dt здесь не нужен, потому что постоянные системы обновляются в
  // UpdateContinuousSystems.
  (void)dt;
}

int Game::SelectedInventoryItemId() const {
  // Если у игрока нет инвентаря или он пуст, возвращаем 0 как "нет предмета".
  const auto inv_it = world_.registry.inventories.find(world_.player);
  if (inv_it == world_.registry.inventories.end() ||
      inv_it->second.slots.empty()) {
    return 0;
  }
  const int clamped =
      std::clamp(inventory_selected_, 0,
                 static_cast<int>(inv_it->second.slots.size()) - 1);
  // Возвращаем item_id выбранного слота, а не индекс слота.
  return inv_it->second.slots[clamped].item_id;
}

void Game::HandleInventory(InputCommand command) {
  // Сначала определяем количество доступных слотов.
  // Это нужно для циклического выбора и проверки цифровых клавиш.
  const auto inv_it = world_.registry.inventories.find(world_.player);
  const int item_count = inv_it == world_.registry.inventories.end()
                             ? 0
                             : static_cast<int>(inv_it->second.slots.size());

  if (command == InputCommand::Cancel || command == InputCommand::Inventory) {
    // Инвентарь может быть открыт из Exploration или DialogueCombat.
    // Поэтому возвращаемся не в фиксированный режим, а в inventory_return_mode.
    world_.mode = world_.inventory_return_mode;
    return;
  }

  if (item_count <= 0) {
    // Пустой инвентарь нечем обрабатывать.
    return;
  }

  if (command == InputCommand::Up) {
    // Циклический переход вверх: с первого элемента попадаем на последний.
    inventory_selected_ = (inventory_selected_ + item_count - 1) % item_count;
  } else if (command == InputCommand::Down) {
    // Циклический переход вниз.
    inventory_selected_ = (inventory_selected_ + 1) % item_count;
  } else {
    // Цифры выбирают конкретный слот и сразу используют предмет.
    const int digit_index = DigitToIndex(command);
    if (digit_index >= 0 && digit_index < item_count) {
      inventory_selected_ = digit_index;
      inventory_system::UseItem(world_, db_, world_.player,
                                SelectedInventoryItemId());
    } else if (command == InputCommand::Confirm ||
               command == InputCommand::Use) {
      // Enter или U используют текущий выбранный предмет.
      inventory_system::UseItem(world_, db_, world_.player,
                                SelectedInventoryItemId());
    }
  }

  // После использования предмет мог быть удалён из инвентаря.
  // Поэтому перечитываем количество слотов и зажимаем выбранный индекс.
  const auto refreshedinv_it = world_.registry.inventories.find(world_.player);
  const int refreshed_count =
      refreshedinv_it == world_.registry.inventories.end()
          ? 0
          : static_cast<int>(refreshedinv_it->second.slots.size());
  if (refreshed_count > 0) {
    inventory_selected_ =
        std::clamp(inventory_selected_, 0, refreshed_count - 1);
  } else {
    inventory_selected_ = 0;
  }
}

void Game::HandleEndScreen(InputCommand command) {
  // На финальном экране Enter возвращает в главное меню.
  if (command == InputCommand::Confirm) {
    ResetToMenu();
  } else if (command == InputCommand::Cancel) {
    // Esc завершает приложение.
    world_.running = false;
  }
}
