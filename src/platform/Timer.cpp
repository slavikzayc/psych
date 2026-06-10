#include "platform/Timer.h"

Timer::Timer() : last_(std::chrono::steady_clock::now()) {}

float Timer::Restart() {
  const auto now = std::chrono::steady_clock::now();
  const std::chrono::duration<float> elapsed = now - last_;
  last_ = now;
  return elapsed.count();
}
