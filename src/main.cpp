#include <exception>
#include <iostream>

#include "game/Game.h"

int main() {
  try {
    // Game является единственной "управляющей" сущностью приложения.
    // Внутри него создаётся мир, загружаются JSON-данные, запускается
    // главный цикл игры и вызываются все функциональные системы.
    Game game;
    return game.Run();
  } catch (const std::exception &ex) {
    // Этот catch нужен как последняя защита от аварийного завершения.
    // Нормальные ошибки данных обрабатываются внутри GameDatabase::LoadAll,
    // а сюда попадают только непредвиденные исключения.
    std::cerr << "Fatal error: " << ex.what() << "\n";
    return 1;
  }
}
