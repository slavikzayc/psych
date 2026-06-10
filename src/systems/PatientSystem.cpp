#include "systems/PatientSystem.h"

void PatientSystem::Update(WorldState& world, float dt) {
  if (world.mode != GameMode::Exploration && world.mode != GameMode::Inventory &&
      world.mode != GameMode::DialogueCombat) {
    return;
  }

  time_accumulator_ += dt;
  const int elapsed = static_cast<int>(time_accumulator_);
  if (elapsed <= 0) {
    return;
  }
  time_accumulator_ -= static_cast<float>(elapsed);

  for (auto& [entity, patient] : world.registry.patients) {
    if (patient.state != PatientState::Treated || patient.relapse_timer_seconds <= 0) {
      continue;
    }

    patient.relapse_timer_seconds -= elapsed;
    if (patient.relapse_timer_seconds <= 0) {
      patient.state = PatientState::Calm;
      patient.current_tension = patient.max_tension;
      world.registry.renderables[entity].symbol = 'p';
      world.registry.collisions[entity] = {true};
      if (world.registry.maps.find(entity) != world.registry.maps.end() &&
          world.registry.maps[entity].map_id == world.currentmap_id) {
        world.message = "Один из пациентов снова ищет разговора.";
      }
    }
  }
}
