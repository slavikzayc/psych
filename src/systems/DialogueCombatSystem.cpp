#include "systems/DialogueCombatSystem.h"

#include <algorithm>
#include <vector>

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

void LeaveDialogue(WorldState &world) {
  PatientComponent &patient = world.registry.patients.at(world.active_patient);
  if (patient.state == PatientState::InDialogue) {
    patient.state = PatientState::Calm;
  }

  world.registry.dialogue_combats.erase(world.active_patient);
  world.active_patient = 0;
  world.mode = GameMode::Exploration;
  world.message = "Вы прекращаете разговор и отступаете на шаг.";
}
}

void dialogue_combat_system::HandleInput(WorldState &world,
                                         const GameDatabase &db,
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

  DialogueCombatComponent &combat =
      world.registry.dialogue_combats.at(world.active_patient);
  PatientComponent &patient = world.registry.patients.at(world.active_patient);
  const StatsComponent &stats = world.registry.stats.at(world.player);
  const std::vector<ReplyData> &replies =
      db.replies_by_node.at(combat.current_node_id);
  if (index >= static_cast<int>(replies.size())) {
    return;
  }

  const ReplyData &reply = replies[index];
  if (stats.authority < reply.min_authority ||
      stats.medication < reply.min_medication) {
    world.message =
        "Вы понимаете, что эта линия разговора сейчас не "
        "сработает.";
    return;
  }

  const int resistance = db.patients.at(patient.patient_id).resistance;

  int impact = reply.base_impact + stats.authority * reply.authority_scale +
               stats.medication * reply.medication_scale +
               combat.pending_item_bonus - resistance;

  impact = std::max(0, impact);

  combat.pending_item_bonus = 0;

  patient.current_tension -= impact;

  if (reply.fail_sanity_damage > 0) {
    if (impact <= 0) {
      sanity_system::ChangeSanity(world, -reply.fail_sanity_damage);
    } else if (impact < reply.base_impact) {
      sanity_system::ChangeSanity(world, -(reply.fail_sanity_damage / 2));
    }
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

    sanity_system::ChangeSanity(world, 8);

    world.message = db.dialogues.at(combat.dialogue_id).success_text;
    world.mode = GameMode::Exploration;
    return;
  }

  if (reply.next_node_id > 0) {
    combat.current_node_id = reply.next_node_id;
  }

  world.message = impact > 0 ? "Пациент слышит вас, но напряжение ещё держится."
                             : "Слова не достигают цели.";
}
