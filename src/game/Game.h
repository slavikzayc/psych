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
  // Точка входа в игровой объект.
  // Загружает JSON, настраивает консоль, запускает главный цикл и возвращает
  // код выхода.
  int Run();

private:
  // Собирает и выводит текущий экран: меню, карту, инвентарь, диалог или
  // финальный экран.
  void Render();

  // Выполняет один шаг логики: сначала постоянные системы, потом обработка
  // режима.
  void Update(InputCommand command, float dt);

  // Обновляет время, рассудок и пациентов независимо от действий игрока.
  void UpdateContinuousSystems(float dt);

  // Переводит игру из меню к экрану распределения характеристик.
  void StartNewGame();

  // Создаёт ECS-мир на основе spawn_points.json и загруженных data-таблиц.
  void BuildWorld();

  // Сбрасывает runtime-состояние к главному меню.
  void ResetToMenu();

  // Обработчики ввода по режимам.
  void HandleMainMenu(InputCommand command);
  void HandleStatAllocation(InputCommand command);
  void HandleExploration(InputCommand command, float dt);
  void HandleInventory(InputCommand command);
  void HandleEndScreen(InputCommand command);

  // Возвращает item_id выбранного слота инвентаря.
  int SelectedInventoryItemId() const;

  // Загруженные геймдизайнерские данные из JSON.
  GameDatabase db_;

  // Всё изменяемое состояние мира: ECS, режим, время, активный пациент и
  // сообщение.
  WorldState world_;

  // Время предыдущего кадра. Используется для вычисления dt.
  std::chrono::steady_clock::time_point last_frame_time_;

  // Индекс выбранного пункта главного меню.
  int menu_selected_ = 0;

  // Индекс выбранной строки на экране распределения характеристик.
  int stat_selected_ = 0;

  // Временные значения характеристик до старта игры.
  int pending_authority_ = 1;
  int pending_medication_ = 1;
  int free_stat_points_ = 8;

  // Индекс выбранного предмета в инвентаре.
  int inventory_selected_ = 0;

  // Аккумуляторы времени для функциональных систем.
  // Они нужны, потому что после удаления system-классов состояние таймеров
  // должно храниться в Game, а не в объектах систем.
  float sanity_drain_accumulator_ = 0.0f;
  float night_time_accumulator_ = 0.0f;
  float patient_time_accumulator_ = 0.0f;
};
