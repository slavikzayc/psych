#include "systems/InputSystem.h"

#include <conio.h>

namespace {
InputCommand DigitCommand(int ch) {
  switch (ch) {
    case '0':
      return InputCommand::Digit0;
    case '1':
      return InputCommand::Digit1;
    case '2':
      return InputCommand::Digit2;
    case '3':
      return InputCommand::Digit3;
    case '4':
      return InputCommand::Digit4;
    case '5':
      return InputCommand::Digit5;
    case '6':
      return InputCommand::Digit6;
    case '7':
      return InputCommand::Digit7;
    case '8':
      return InputCommand::Digit8;
    case '9':
      return InputCommand::Digit9;
    default:
      return InputCommand::None;
  }
}

bool HasPendingKey() { return _kbhit() != 0; }
}  // namespace

InputCommand InputSystem::ReadCommand() {
  int ch = _getch();
  if (ch == 0 || ch == 224) {
    ch = _getch();
    switch (ch) {
      case 72:
        return InputCommand::Up;
      case 80:
        return InputCommand::Down;
      case 75:
        return InputCommand::Left;
      case 77:
        return InputCommand::Right;
      default:
        return InputCommand::None;
    }
  }

  switch (ch) {
    case 'w':
    case 'W':
      return InputCommand::Up;
    case 's':
    case 'S':
      return InputCommand::Down;
    case 'a':
    case 'A':
      return InputCommand::Left;
    case 'd':
    case 'D':
      return InputCommand::Right;
    case '\r':
    case '\n':
      return InputCommand::Confirm;
    case 27:
      return InputCommand::Cancel;
    case 'f':
    case 'F':
      return InputCommand::Interact;
    case 'e':
    case 'E':
      return InputCommand::Inventory;
    case 'u':
    case 'U':
      return InputCommand::Use;
    default:
      return DigitCommand(ch);
  }
}

InputCommand InputSystem::ReadCommandNonBlocking() {
  if (!HasPendingKey()) {
    return InputCommand::None;
  }
  return ReadCommand();
}
