#include "systems/DialogueCombatSystem.h"

#include <algorithm>

#include "systems/SanitySystem.h"

namespace {
int CommandToIndex(InputCommand command) {
  // Цифры 1-9 переводятся в индексы реплик 0-8.
  switch (command) {
  case InputCommand::Digit1:
    return 0;
  case InputCommand::Digit2:
    return 1;
  case InputCommand::Digit3:
    return 2;
  case InputCommand::Digit4:
    return 3;
  case InputCommand::Digit5:
    return 4;
  case InputCommand::Digit6:
    return 5;
  case InputCommand::Digit7:
    return 6;
  case InputCommand::Digit8:
    return 7;
  case InputCommand::Digit9:
    return 8;
  default:
    return -1;
  }
}

void LeaveDialogue(WorldState &world) {
  // Если игрок уходит до стабилизации, пациент возвращается в спокойное
  // состояние.
  auto patient_it = world.registry.patients.find(world.active_patient);
  if (patient_it != world.registry.patients.end() &&
      patient_it->second.state == PatientState::InDialogue) {
    patient_it->second.state = PatientState::Calm;
  }

  // Активный диалог удаляется, а режим возвращается к исследованию.
  world.registry.dialogue_combats.erase(world.active_patient);
  world.active_patient = 0;
  world.mode = GameMode::Exploration;
  world.message = "Вы прекращаете разговор и отступаете на шаг.";
}
} // namespace

void dialogue_combat_system::HandleInput(WorldState &world,
                                         const GameDatabase &db,
                                         InputCommand command) {
  // 0 и Esc — выход из диалога.
  if (command == InputCommand::Digit0 || command == InputCommand::Cancel) {
    LeaveDialogue(world);
    return;
  }

  // E открывает инвентарь, но после закрытия нужно вернуться в диалог.
  if (command == InputCommand::Inventory) {
    world.inventory_return_mode = GameMode::DialogueCombat;
    world.mode = GameMode::Inventory;
    return;
  }

  // Остальной ввод должен быть выбором реплики.
  const int index = CommandToIndex(command);
  if (index < 0) {
    return;
  }

  // Для хода нужны три сущности данных: активный бой, пациент и статы игрока.
  auto combat_it = world.registry.dialogue_combats.find(world.active_patient);
  auto patient_it = world.registry.patients.find(world.active_patient);
  auto stats_it = world.registry.stats.find(world.player);
  if (combat_it == world.registry.dialogue_combats.end() ||
      patient_it == world.registry.patients.end() ||
      stats_it == world.registry.stats.end()) {
    world.mode = GameMode::Exploration;
    world.message = "Диалог прерван.";
    return;
  }

  // Получаем список реплик текущего узла.
  DialogueCombatComponent &combat = combat_it->second;
  PatientComponent &patient = patient_it->second;
  const auto replies_it = db.replies_by_node.find(combat.current_node_id);
  if (replies_it == db.replies_by_node.end() ||
      index >= static_cast<int>(replies_it->second.size())) {
    return;
  }

  // Проверяем требования реплики по характеристикам игрока.
  const ReplyData &reply = replies_it->second[index];
  const StatsComponent &stats = stats_it->second;
  if (stats.authority < reply.min_authority ||
      stats.medication < reply.min_medication) {
    world.message = "Вы понимаете, что эта линия разговора сейчас не "
                    "сработает.";
    return;
  }

  // Сопротивление пациента берётся из PatientData и вычитается из impact.
  const auto patientdata_it = db.patients.find(patient.patient_id);
  const int resistance = patientdata_it == db.patients.end()
                             ? 0
                             : patientdata_it->second.resistance;
  // Формула воздействия: база + вклад характеристик + бонус предмета -
  // сопротивление.
  int impact = reply.base_impact + stats.authority * reply.authority_scale +
               stats.medication * reply.medication_scale +
               combat.pending_item_bonus - resistance;
  // Отрицательный impact не допускается: плохая реплика просто не снижает
  // tension.
  impact = std::max(0, impact);

  // Бонус от предмета одноразовый и сбрасывается после выбора реплики.
  combat.pending_item_bonus = 0;

  // Напряжение пациента скрыто от игрока, но именно оно определяет успех.
  patient.current_tension -= impact;

  if (reply.fail_sanity_damage > 0) {
    // Полностью неэффективная реплика наносит полный урон рассудку.
    if (impact <= 0) {
      sanity_system::ChangeSanity(world, -reply.fail_sanity_damage);
    } else if (impact < reply.base_impact) {
      // Ослабленная реплика наносит половину штрафа.
      sanity_system::ChangeSanity(world, -(reply.fail_sanity_damage / 2));
    }
  }

  if (world.mode == GameMode::Defeat) {
    return;
  }

  if (patient.current_tension <= 0) {
    // Пациент стабилизирован: запускаем cooldown, меняем символ и убираем
    // коллизию.
    patient.state = PatientState::Treated;
    patient.relapse_timer_seconds = patient.dialogue_cooldown_seconds;
    world.registry.renderables[world.active_patient].symbol = 't';
    world.registry.collisions.erase(world.active_patient);
    combat.active = false;

    // Успешная стабилизация немного восстанавливает рассудок врачу.
    sanity_system::ChangeSanity(world, 8);

    const auto dialogue_it = db.dialogues.find(combat.dialogue_id);
    world.message = dialogue_it == db.dialogues.end()
                        ? "Пациент стабилизирован."
                        : dialogue_it->second.success_text;
    world.mode = GameMode::Exploration;
    return;
  }

  // Если реплика ведёт к другому узлу, переключаем current_node_id.
  if (reply.next_node_id > 0) {
    combat.current_node_id = reply.next_node_id;
  }

  world.message = impact > 0 ? "Пациент слышит вас, но напряжение ещё держится."
                             : "Слова не достигают цели.";
}
