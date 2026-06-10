#include "systems/MovementSystem.h"

void MovementSystem::Update(WorldState& world, const GameDatabase& db,
                            const CollisionSystem& collision, InputCommand command) {
  int dx = 0;
  int dy = 0;
  if (command == InputCommand::Up) dy = -1;
  if (command == InputCommand::Down) dy = 1;
  if (command == InputCommand::Left) dx = -1;
  if (command == InputCommand::Right) dx = 1;
  if (dx == 0 && dy == 0) {
    return;
  }

  auto pos_it = world.registry.positions.find(world.player);
  if (pos_it == world.registry.positions.end()) {
    return;
  }

  const int next_x = pos_it->second.x + dx;
  const int next_y = pos_it->second.y + dy;
  if (!collision.IsBlocked(world, db, next_x, next_y, world.player)) {
    pos_it->second.x = next_x;
    pos_it->second.y = next_y;
    world.message = "Шаги гулко отдаются в коридоре.";
  } else {
    world.message = "Проход заблокирован.";
  }
}
