#include "platform/Timer.h"

float RestartTimer(std::chrono::steady_clock::time_point &last) {
  const auto now = std::chrono::steady_clock::now();

  const std::chrono::duration<float> elapsed = now - last;

  last = now;
  return elapsed.count();
}
