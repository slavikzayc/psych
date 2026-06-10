#pragma once

#include <chrono>

class Timer {
 public:
  Timer();
  float Restart();

 private:
  std::chrono::steady_clock::time_point last_;
};
