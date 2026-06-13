#include "systems/PatientSystem.h"

void patient_system::Update(WorldState &world, float dt,
                            float &time_accumulator) {
  // Пациенты обновляются только во время активной игры.
  if (world.mode != GameMode::Exploration &&
      world.mode != GameMode::Inventory &&
      world.mode != GameMode::DialogueCombat) {
    return;
  }

  // Накопитель переводит дробный dt в целые секунды.
  time_accumulator += dt;
  const int elapsed = static_cast<int>(time_accumulator);
  if (elapsed <= 0) {
    return;
  }
  time_accumulator -= static_cast<float>(elapsed);

  for (auto &[entity, patient] : world.registry.patients) {
    // Нас интересуют только стабилизированные пациенты с активным таймером.
    if (patient.state != PatientState::Treated ||
        patient.relapse_timer_seconds <= 0) {
      continue;
    }

    patient.relapse_timer_seconds -= elapsed;
    if (patient.relapse_timer_seconds <= 0) {
      // Таймер закончился: пациент снова доступен для разговора.
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
