#include "systems/CollisionSystem.h"

bool CollisionSystem::IsBlocked(const WorldState& world, const GameDatabase& db, int x, int y,
                                Entity ignored) const {
  const auto map_it = db.maps.find(world.currentmap_id);
  if (map_it == db.maps.end()) {
    return true;
  }

  const MapData& map = map_it->second;
  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return true;
  }

  if (y >= static_cast<int>(map.layout.size()) || x >= static_cast<int>(map.layout[y].size())) {
    return true;
  }

  if (map.layout[y][x] == '#') {
    return true;
  }

  for (const auto& [entity, collision] : world.registry.collisions) {
    if (entity == ignored || !collision.blocks_movement) {
      continue;
    }

    const auto map_entity_it = world.registry.maps.find(entity);
    if (map_entity_it != world.registry.maps.end() &&
        map_entity_it->second.map_id != world.currentmap_id) {
      continue;
    }

    const auto pos_it = world.registry.positions.find(entity);
    if (pos_it != world.registry.positions.end() && pos_it->second.x == x &&
        pos_it->second.y == y) {
      return true;
    }
  }

  return false;
}
