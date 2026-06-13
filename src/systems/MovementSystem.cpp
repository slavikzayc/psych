#include "systems/MovementSystem.h"

void movement_system::Update(WorldState &world, const GameDatabase &db,
                             InputCommand command) {
  // Сначала переводим команду в смещение по x/y.
  int dx = 0;
  int dy = 0;
  if (command == InputCommand::Up)
    dy = -1;
  if (command == InputCommand::Down)
    dy = 1;
  if (command == InputCommand::Left)
    dx = -1;
  if (command == InputCommand::Right)
    dx = 1;
  if (dx == 0 && dy == 0) {
    // Если команда не является движением, MovementSystem ничего не делает.
    return;
  }

  // Игрок должен иметь позицию. Если компонента нет, движение невозможно.
  auto pos_it = world.registry.positions.find(world.player);
  if (pos_it == world.registry.positions.end()) {
    return;
  }

  // Целевая клетка = текущая позиция + смещение.
  const int next_x = pos_it->second.x + dx;
  const int next_y = pos_it->second.y + dy;

  // Перед изменением позиции проверяем стены, границы и блокирующие сущности.
  if (!collision_system::IsBlocked(world, db, next_x, next_y, world.player)) {
    // Если клетка свободна, меняем PositionComponent игрока.
    pos_it->second.x = next_x;
    pos_it->second.y = next_y;
    world.message = "Шаги гулко отдаются в коридоре.";
  } else {
    world.message = "Проход заблокирован.";
  }
}
