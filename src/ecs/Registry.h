#pragma once

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ecs/Components.h"
#include "ecs/Entity.h"

struct Registry {
  // Следующий свободный id сущности.
  // 0 не используется: его удобно считать значением "нет сущности".
  Entity next_entity = 1;

  // Набор всех живых сущностей.
  // unordered_set выбран потому, что проверка существования по id должна быть
  // быстрой.
  std::unordered_set<Entity> alive_entities;

  // Ниже идут отдельные хранилища компонентов.
  // Сущность является "игроком", "пациентом" или "предметом" не через
  // наследование, а через наличие соответствующих компонентов в этих
  // unordered_map.
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

inline Entity CreateEntity(Registry &registry) {
  // Выдаём новый id и сразу увеличиваем счётчик для следующей сущности.
  const Entity entity = registry.next_entity++;

  // Помечаем entity как живую. Компоненты будут добавлены отдельно тем кодом,
  // который создаёт конкретный тип игрового объекта.
  registry.alive_entities.insert(entity);
  return entity;
}

inline bool IsAlive(const Registry &registry, Entity entity) {
  // Если id есть в alive_entities, системы могут считать сущность существующей.
  return registry.alive_entities.count(entity) > 0;
}

inline void DestroyEntity(Registry &registry, Entity entity) {
  // Удаление сущности в ECS должно убрать её из всех компонентных контейнеров.
  // Если удалить только alive_entities, то, например, RenderSystem всё ещё мог
  // бы увидеть старый RenderableComponent и нарисовать уже несуществующий
  // объект.
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
