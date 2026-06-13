#include "systems/NightEventSystem.h"

namespace {
NightPhase PhaseForProgress(float progress) {
  if (progress < 0.34f) return NightPhase::Early;
  if (progress < 0.75f) return NightPhase::Middle;
  return NightPhase::Final;
}
}

void night_event_system::Update(WorldState &world, GameDatabase &db, float dt,
                                float &time_accumulator) {
  if (world.mode != GameMode::Exploration &&
      world.mode != GameMode::Inventory &&
      world.mode != GameMode::DialogueCombat) {
    return;
  }

  time_accumulator += dt;
  const int elapsed_seconds = static_cast<int>(time_accumulator);
  if (elapsed_seconds <= 0) {
    return;
  }
  time_accumulator -= static_cast<float>(elapsed_seconds);

  world.shift_time_seconds += elapsed_seconds;
  if (world.shift_time_seconds >= world.shift_duration_seconds) {
    world.shift_time_seconds = world.shift_duration_seconds;
    world.mode = GameMode::Victory;
    world.message =
        "Рассвет просачивается через жалюзи. Смена "
        "окончена.";
    return;
  }

  const float progress = static_cast<float>(world.shift_time_seconds) /
                         static_cast<float>(world.shift_duration_seconds);
  world.phase = PhaseForProgress(progress);

  for (NightEventData &event : db.night_events) {
    if (event.triggered || world.shift_time_seconds < event.trigger_time) {
      continue;
    }

    event.triggered = true;
    if (event.event_type == "activate_patient") {
      for (auto &[entity, patient] : world.registry.patients) {
        if (patient.patient_id == event.target_id &&
            patient.state == PatientState::Calm) {
          patient.state = PatientState::Unstable;
          world.registry.renderables[entity].symbol = 'P';
          world.message = event.message;
          break;
        }
      }
    } else if (event.event_type == "show_message") {
      world.message = event.message;
    } else if (event.event_type == "increase_sanity_drain") {
      world.registry.sanities.at(world.player).passive_drain_per_minute +=
          event.target_id;
      world.message = event.message;
    }
  }
}
