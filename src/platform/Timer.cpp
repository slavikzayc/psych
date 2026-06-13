#include "platform/Timer.h"

float RestartTimer(std::chrono::steady_clock::time_point &last) {
  // steady_clock монотонный: он не прыгает назад при изменении системного
  // времени.
  const auto now = std::chrono::steady_clock::now();

  // elapsed — сколько реального времени прошло с прошлого кадра.
  const std::chrono::duration<float> elapsed = now - last;

  // Обновляем last, чтобы следующий вызов считал время уже от текущего момента.
  last = now;
  return elapsed.count();
}
