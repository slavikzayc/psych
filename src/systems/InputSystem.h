#pragma once

enum class InputCommand {
  // Нет нажатой клавиши в текущем кадре.
  None,

  // Направления используются и для WASD, и для стрелок.
  Up,
  Down,
  Left,
  Right,

  // Подтвердить/отменить действие.
  Confirm,
  Cancel,

  // Игровые действия.
  Interact,
  Inventory,
  Use,

  // Цифры используются в диалогах и инвентаре.
  Digit0,
  Digit1,
  Digit2,
  Digit3,
  Digit4,
  Digit5,
  Digit6,
  Digit7,
  Digit8,
  Digit9
};

namespace input_system {

// Блокирующее чтение одной команды клавиатуры.
InputCommand ReadCommand();

// Неблокирующее чтение: если клавиши нет, возвращает None.
InputCommand ReadCommandNonBlocking();

} // namespace input_system
