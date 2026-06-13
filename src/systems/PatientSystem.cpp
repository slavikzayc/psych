#include "systems/PatientSystem.h"

void patient_system::Update(WorldState &world, float dt,
                            float &time_accumulator) {
  if (world.mode != GameMode::Exploration &&
      world.mode != GameMode::Inventory &&
      world.mode != GameMode::DialogueCombat) {
    return;
  }

  time_accumulator += dt;
  const int elapsed = static_cast<int>(time_accumulator);
  if (elapsed <= 0) {
    return;
  }
  time_accumulator -= static_cast<float>(elapsed);

  for (auto &[entity, patient] : world.registry.patients) {
    if (patient.state != PatientState::Treated ||
        patient.relapse_timer_seconds <= 0) {
      continue;
    }

    patient.relapse_timer_seconds -= elapsed;
    if (patient.relapse_timer_seconds <= 0) {
      patient.state = PatientState::Calm;
      patient.current_tension = patient.max_tension;
      world.registry.renderables[entity].symbol = 'p';
      world.registry.collisions[entity] = {true};
      if (world.registry.maps.at(entity).map_id == world.currentmap_id) {
        world.message = "Один из пациентов снова ищет разговора.";
      }
    }
  }
}
