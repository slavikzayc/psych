#pragma once

#include <string>

struct DialogueData {
  // id диалога. На него ссылается PatientData::dialogue_id.
  int id = 0;

  // Узел, с которого начинается диалог.
  int start_node_id = 0;

  // Сообщение при успешной стабилизации пациента.
  std::string success_text;

  // Сообщение при провале. В текущем прототипе используется как задел.
  std::string fail_text;
};

struct DialogueNodeData {
  // id узла диалога.
  int id = 0;

  // Внешний ключ на DialogueData.
  int dialogue_id = 0;

  // Текст пациента в этом узле.
  std::string patient_text;

  // Описание состояния пациента для игрока.
  std::string state_hint;
};

struct ReplyData {
  // id реплики.
  int id = 0;

  // Узел, в котором эта реплика доступна.
  int node_id = 0;

  // Текст ответа врача.
  std::string text;

  // Базовое воздействие на напряжение пациента.
  int base_impact = 0;

  // Минимальные характеристики для выбора реплики.
  int min_authority = 0;
  int min_medication = 0;

  // Множители характеристик в формуле impact.
  int authority_scale = 0;
  int medication_scale = 0;

  // Урон рассудку при неэффективной реплике.
  int fail_sanity_damage = 0;

  // Следующий узел диалога. 0 означает, что переход не задан.
  int next_node_id = 0;
};
