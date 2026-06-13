#include "systems/CollisionSystem.h"

bool collision_system::IsBlocked(const WorldState &world,
                                 const GameDatabase &db, int x, int y,
                                 Entity ignored) {
  // Берём текущую карту. Если id карты неверный, движение запрещается.
  const auto map_it = db.maps.find(world.currentmap_id);
  if (map_it == db.maps.end()) {
    return true;
  }

  const MapData &map = map_it->second;

  // Проверка выхода за границы прямоугольника карты.
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return true;
  }

  // Проверка фактических строк layout на случай некорректных данных.
  if (y >= static_cast<int>(map.layout.size()) ||
      x >= static_cast<int>(map.layout[y].size())) {
    return true;
  }

  // Стена layout всегда блокирует движение.
  if (map.layout[y][x] == '#') {
    return true;
  }

  // Проверяем сущности с CollisionComponent.
  for (const auto &[entity, collision] : world.registry.collisions) {
    // ignored используется, чтобы игрок не сталкивался сам с собой.
    if (entity == ignored || !collision.blocks_movement) {
      continue;
    }

    // Коллизии сущностей с других карт игнорируются.
    const auto map_entity_it = world.registry.maps.find(entity);
    if (map_entity_it != world.registry.maps.end() &&
        map_entity_it->second.map_id != world.currentmap_id) {
      continue;
    }

    // Если блокирующая сущность стоит в целевой клетке, клетка занята.
    const auto pos_it = world.registry.positions.find(entity);
    if (pos_it != world.registry.positions.end() && pos_it->second.x == x &&
        pos_it->second.y == y) {
      return true;
    }
  }

  return false;
}
