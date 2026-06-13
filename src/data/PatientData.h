#pragma once

#include <string>

struct PatientData {
  // Первичный ключ пациента.
  int id = 0;

  // Имя пациента для экрана диалога.
  std::string name;

  // Краткое описание состояния/архетипа.
  std::string archetype;

  // Начальная и максимальная шкала напряжения.
  int max_tension = 0;

  // Сопротивление пациента вычитается из impact реплики.
  int resistance = 0;

  // Внешний ключ на DialogueData.
  int dialogue_id = 0;

  // Через сколько секунд после стабилизации пациент снова станет доступен.
  int dialogue_cooldown_seconds = 120;
};
