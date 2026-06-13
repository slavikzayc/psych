#include "systems/DialogueCombatSystem.h"

#include <algorithm>

#include "systems/SanitySystem.h"

namespace {
int CommandToIndex(InputCommand command) {
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

void LeaveDialogue(WorldState& world) {
  auto patient_it = world.registry.patients.find(world.active_patient);
  if (patient_it != world.registry.patients.end() &&
      patient_it->second.state == PatientState::InDialogue) {
    patient_it->second.state = PatientState::Calm;
  }

  world.registry.dialogue_combats.erase(world.active_patient);
  world.active_patient = 0;
  world.mode = GameMode::Exploration;
  world.message =
      "Вы прекращаете разговор и отступаете на шаг.";
}
}  // namespace

void DialogueCombatSystem::HandleInput(WorldState& world, const GameDatabase& db,
                                       InputCommand command) {
  if (command == InputCommand::Digit0 || command == InputCommand::Cancel) {
    LeaveDialogue(world);
    return;
  }

  if (command == InputCommand::Inventory) {
    world.inventory_return_mode = GameMode::DialogueCombat;
    world.mode = GameMode::Inventory;
    return;
  }

  const int index = CommandToIndex(command);
  if (index < 0) {
    return;
  }

  auto combat_it = world.registry.dialogue_combats.find(world.active_patient);
  auto patient_it = world.registry.patients.find(world.active_patient);
  auto stats_it = world.registry.stats.find(world.player);
  if (combat_it == world.registry.dialogue_combats.end() ||
      patient_it == world.registry.patients.end() || stats_it == world.registry.stats.end()) {
    world.mode = GameMode::Exploration;
    world.message = "Диалог прерван.";
    return;
  }

  DialogueCombatComponent& combat = combat_it->second;
  PatientComponent& patient = patient_it->second;
  const auto replies_it = db.replies_by_node.find(combat.current_node_id);
  if (replies_it == db.replies_by_node.end() ||
      index >= static_cast<int>(replies_it->second.size())) {
    return;
  }

  const ReplyData& reply = replies_it->second[index];
  const StatsComponent& stats = stats_it->second;
  if (stats.authority < reply.min_authority || stats.medication < reply.min_medication) {
    world.message =
        "Вы понимаете, что эта линия разговора сейчас не "
        "сработает.";
    return;
  }

  const auto patientdata_it = db.patients.find(patient.patient_id);
  const int resistance =
      patientdata_it == db.patients.end() ? 0 : patientdata_it->second.resistance;
  int impact = reply.base_impact + stats.authority * reply.authority_scale +
               stats.medication * reply.medication_scale + combat.pending_item_bonus - resistance;
  impact = std::max(0, impact);
  combat.pending_item_bonus = 0;

  patient.current_tension -= impact;
  
  if (reply.fail_sanity_damage > 0) {
      SanitySystem sanity;
      if (impact <= 0) {
          // Реплика полностью заблокирована — полный штраф
          sanity.ChangeSanity(world, -reply.fail_sanity_damage);
      }
      else if (impact < reply.base_impact) {
          // Реплика ослаблена сопротивлением — частичный штраф
          sanity.ChangeSanity(world, -(reply.fail_sanity_damage / 2));
      }
      // Иначе impact >= base_impact — реплика сработала полностью, штрафа нет
  }

  if (world.mode == GameMode::Defeat) {
    return;
  }

  if (patient.current_tension <= 0) {
    patient.state = PatientState::Treated;
    patient.relapse_timer_seconds = patient.dialogue_cooldown_seconds;
    world.registry.renderables[world.active_patient].symbol = 't';
    world.registry.collisions.erase(world.active_patient);
    combat.active = false;
    SanitySystem sanity;
    sanity.ChangeSanity(world, 8);

    const auto dialogue_it = db.dialogues.find(combat.dialogue_id);
    world.message = dialogue_it == db.dialogues.end()
                        ? "Пациент стабилизирован."
                        : dialogue_it->second.success_text;
    world.mode = GameMode::Exploration;
    return;
  }

  if (reply.next_node_id > 0) {
    combat.current_node_id = reply.next_node_id;
  }


  world.message =
      impact > 0
          ? "Пациент слышит вас, но напряжение ещё держится."
          : "Слова не достигают цели.";
}
