#pragma once

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ecs/Components.h"
#include "ecs/Entity.h"

struct Registry {
  Entity next_entity = 1;

  std::unordered_set<Entity> alive_entities;

  std::unordered_map<Entity, PositionComponent> positions;
  std::unordered_map<Entity, MapComponent> maps;
  std::unordered_map<Entity, RenderableComponent> renderables;
  std::unordered_map<Entity, CollisionComponent> collisions;
  std::unordered_map<Entity, PlayerTag> players;
  std::unordered_map<Entity, StatsComponent> stats;
  std::unordered_map<Entity, SanityComponent> sanities;
  std::unordered_map<Entity, InventoryComponent> inventories;
  std::unordered_map<Entity, ItemComponent> items;
  std::unordered_map<Entity, PatientComponent> patients;
  std::unordered_map<Entity, DoorComponent> doors;
  std::unordered_map<Entity, InteractableComponent> interactables;
  std::unordered_map<Entity, DialogueCombatComponent> dialogue_combats;
};

inline Entity CreateEntity(Registry& registry) {
  const Entity entity = registry.next_entity++;
  registry.alive_entities.insert(entity);
  return entity;
}

inline bool IsAlive(const Registry& registry, Entity entity) {
    return registry.alive_entities.count(entity) > 0;
}


inline void DestroyEntity(Registry& registry, Entity entity) {
  registry.alive_entities.erase(entity);

  registry.positions.erase(entity);
  registry.maps.erase(entity);
  registry.renderables.erase(entity);
  registry.collisions.erase(entity);
  registry.players.erase(entity);
  registry.stats.erase(entity);
  registry.sanities.erase(entity);
  registry.inventories.erase(entity);
  registry.items.erase(entity);
  registry.patients.erase(entity);
  registry.doors.erase(entity);
  registry.interactables.erase(entity);
  registry.dialogue_combats.erase(entity);
}
