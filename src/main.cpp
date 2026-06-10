#include <exception>
#include <iostream>

#include "game/Game.h"

int main() {
  try {
    Game game;
    return game.Run();
  } catch (const std::exception& ex) {
    std::cerr << "Fatal error: " << ex.what() << "\n";
    return 1;
  }
}
