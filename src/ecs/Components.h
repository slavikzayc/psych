#pragma once

#include <string>
#include <vector>

#include "ecs/Entity.h"

struct PositionComponent {
  // Позиция сущности на ASCII-карте.
  // x увеличивается вправо, y увеличивается вниз.
  int x = 0;
  int y = 0;
};

struct MapComponent {
  // id комнаты/карты, на которой находится сущность.
  // Все сущности живут в одном Registry, поэтому системы фильтруют их по
  // map_id.
  int map_id = 1;
};

struct RenderableComponent {
  // Символ, который будет положен на canvas карты при отрисовке.
  char symbol = '?';

  // Идентификатор цвета оставлен как часть модели, даже если текущий вывод
  // не раскрашивает символы.
  int color_id = 7;

  // Слой отрисовки: сущности с большим layer рисуются позже и перекрывают
  // нижние.
  int layer = 0;
};

struct CollisionComponent {
  // true означает, что клетка с этой сущностью блокирует движение игрока.
  bool blocks_movement = false;
};

// Пустой тег-компонент. Если entity присутствует в registry.players,
// значит эта сущность является игроком.
struct PlayerTag {};

struct StatsComponent {
  // Авторитет влияет на доступность и эффективность реплик.
  int authority = 5;

  // Медикация влияет на медицинские реплики и бонусы лечения.
  int medication = 5;
};

struct SanityComponent {
  // Текущее значение рассудка. При 0 игра переходит в поражение.
  int current = 100;

  // Максимально возможный рассудок.
  int max = 100;

  // Сколько рассудка снимается за один пассивный тик.
  int passive_drain_per_minute = 3;

  // Частота пассивного тика в игровых секундах.
  int passive_drain_interval_seconds = 12;
};

struct InventorySlot {
  // id предмета из items.json.
  int item_id = 0;

  // Количество предметов этого типа.
  int count = 0;
};

struct InventoryComponent {
  // Список слотов инвентаря владельца.
  std::vector<InventorySlot> slots;
};

struct ItemComponent {
  // id предмета из базы GameDatabase::items.
  int item_id = 0;

  // true — предмет лежит на карте и может быть подобран.
  // false — предмет используется как содержимое контейнера или документа.
  bool can_pickup = true;
};

// Текущее состояние пациента в игровом мире.
enum class PatientState { Calm, Unstable, InDialogue, Treated, Failed };

struct PatientComponent {
  // id пациента из patients.json.
  int patient_id = 0;

  // Текущее состояние пациента.
  PatientState state = PatientState::Calm;

  // Скрытое напряжение пациента. Игрок видит только текстовое состояние.
  int current_tension = 0;

  // Верхняя граница напряжения, к которой пациент возвращается после cooldown.
  int max_tension = 0;

  // id диалога из dialogues.json.
  int dialogue_id = 0;

  // Через сколько секунд после стабилизации можно снова говорить с пациентом.
  int dialogue_cooldown_seconds = 120;

  // Сколько секунд осталось до повторной доступности разговора.
  int relapse_timer_seconds = 0;
};

struct DoorComponent {
  // id карты, куда ведёт дверь.
  int target_map_id = 0;

  // Координаты игрока после перехода в target_map_id.
  int target_x = 0;
  int target_y = 0;

  // Заперта ли дверь.
  bool locked = false;

  // id ключа из items.json. Если 0, ключ не требуется.
  int required_key_item_id = 0;
};

// Тип действия, которое выполнит InteractionSystem при нажатии F.
enum class InteractionType {
  Pickup,
  OpenDoor,
  StartDialogue,
  SearchContainer,
  ReadDocument
};

struct InteractableComponent {
  // Конкретный тип взаимодействия.
  InteractionType type = InteractionType::Pickup;

  // Текст подсказки в HUD.
  std::string prompt_text;

  // Текст подсказки после того, как контейнер был опустошён.
  std::string empty_prompt_text;
};

struct DialogueCombatComponent {
  // id активного диалога.
  int dialogue_id = 0;

  // id текущего узла диалога.
  int current_node_id = 0;

  // Entity пациента, с которым связан бой.
  Entity patient_entity = 0;

  // true, пока диалоговый бой активен.
  bool active = false;

  // Бонус от предмета, который прибавится к следующей реплике.
  int pending_item_bonus = 0;
};
