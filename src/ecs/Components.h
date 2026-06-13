#pragma once

#include <string>
#include <vector>

#include "ecs/Entity.h"

struct PositionComponent {
  int x = 0;
  int y = 0;
};

struct MapComponent {
  int map_id = 1;
};

struct RenderableComponent {
  char symbol = '?';

  int layer = 0;
};

struct CollisionComponent {
  bool blocks_movement = false;
};

struct PlayerTag {};

struct StatsComponent {
  int authority = 5;

  int medication = 5;
};

struct SanityComponent {
  int current = 100;

  int max = 100;

  int passive_drain_per_minute = 3;

  int passive_drain_interval_seconds = 12;
};

struct InventorySlot {
  int item_id = 0;

  int count = 0;
};

struct InventoryComponent {
  std::vector<InventorySlot> slots;
};

struct ItemComponent {
  int item_id = 0;

  bool can_pickup = true;
};

enum class PatientState { Calm, Unstable, InDialogue, Treated, Failed };

struct PatientComponent {
  int patient_id = 0;

  PatientState state = PatientState::Calm;

  int current_tension = 0;

  int max_tension = 0;

  int dialogue_id = 0;

  int dialogue_cooldown_seconds = 120;

  int relapse_timer_seconds = 0;
};

struct DoorComponent {
  int target_map_id = 0;

  int target_x = 0;
  int target_y = 0;

  bool locked = false;

  int required_key_item_id = 0;
};

enum class InteractionType {
  Pickup,
  OpenDoor,
  StartDialogue,
  SearchContainer,
  ReadDocument
};

struct InteractableComponent {
  InteractionType type = InteractionType::Pickup;

  std::string prompt_text;

  std::string empty_prompt_text;
};

struct DialogueCombatComponent {
  int dialogue_id = 0;

  int current_node_id = 0;

  Entity patient_entity = 0;

  bool active = false;

  int pending_item_bonus = 0;
};
