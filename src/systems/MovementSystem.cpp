#include "systems/MovementSystem.h"

void movement_system::Update(WorldState &world, const GameDatabase &db,
                             InputCommand command) {
  int dx = 0;
  int dy = 0;
  if (command == InputCommand::Up) dy = -1;
  if (command == InputCommand::Down) dy = 1;
  if (command == InputCommand::Left) dx = -1;
  if (command == InputCommand::Right) dx = 1;
  if (dx == 0 && dy == 0) {
    return;
  }

  PositionComponent &position = world.registry.positions.at(world.player);

  const int next_x = position.x + dx;
  const int next_y = position.y + dy;

  if (!collision_system::IsBlocked(world, db, next_x, next_y, world.player)) {
    position.x = next_x;
    position.y = next_y;
    world.message = "Шаги гулко отдаются в коридоре.";
  } else {
    world.message = "Проход заблокирован.";
  }
}
