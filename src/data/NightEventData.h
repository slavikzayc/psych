#pragma once

#include <string>

struct NightEventData {
  // id события.
  int id = 0;

  // Фаза ночи из JSON. В текущей логике основным триггером является
  // trigger_time.
  std::string phase;

  // Игровая секунда, после которой событие должно сработать.
  int trigger_time = 0;

  // Тип события: activate_patient, show_message, increase_sanity_drain.
  std::string event_type;

  // Универсальное поле цели. Например patient_id или величина усиления drain.
  int target_id = 0;

  // Сообщение, которое попадёт в HUD при срабатывании.
  std::string message;

  // Runtime-флаг: событие уже было выполнено в текущей смене.
  bool triggered = false;
};
