#pragma once

enum class InputCommand {
  None,
  Up,
  Down,
  Left,
  Right,
  Confirm,
  Cancel,
  Interact,
  Inventory,
  Use,
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

class InputSystem {
 public:
  InputCommand ReadCommand();
  InputCommand ReadCommandNonBlocking();
};
