#pragma once

#include <chrono>

// Возвращает количество секунд с прошлого вызова и обновляет last.
// Функция используется в Game::Run для вычисления dt каждого кадра.
float RestartTimer(std::chrono::steady_clock::time_point &last);
