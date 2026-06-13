#include "systems/CollisionSystem.h"

bool collision_system::IsBlocked(const WorldState &world,
                                 const GameDatabase &db, int x, int y,
                                 Entity ignored) {
  const MapData &map = db.maps.at(world.currentmap_id);

  if (x < 0 || y < 0 || x >= map.width || y >= map.height) {
    return true;
  }

  if (y >= static_cast<int>(map.layout.size()) ||
      x >= static_cast<int>(map.layout[y].size())) {
    return true;
  }

  if (map.layout[y][x] == '#') {
    return true;
  }

  for (const auto &[entity, collision] : world.registry.collisions) {
    if (entity == ignored || !collision.blocks_movement) {
      continue;
    }

    if (world.registry.maps.at(entity).map_id != world.currentmap_id) {
      continue;
    }

    const PositionComponent &position = world.registry.positions.at(entity);
    if (position.x == x && position.y == y) {
      return true;
    }
  }

  return false;
}
