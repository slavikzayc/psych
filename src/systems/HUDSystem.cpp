#include "systems/HUDSystem.h"

#include <iomanip>
#include <sstream>

namespace {
std::string PhaseName(NightPhase phase) {
  switch (phase) {
    case NightPhase::Early:
      return "Early";
    case NightPhase::Middle:
      return "Middle";
    case NightPhase::Final:
      return "Final";
  }
  return "Unknown";
}

std::string FormatTime(int seconds) {
  std::ostringstream out;
  out << std::setfill('0') << std::setw(2) << (seconds / 60) << ":"
      << std::setw(2) << (seconds % 60);
  return out.str();
}
}

std::string hud_system::BuildHud(const WorldState &world,
                                 const std::string &prompt) {
  std::ostringstream out;

  const SanityComponent &sanity = world.registry.sanities.at(world.player);
  out << "\nРассудок: " << sanity.current << " / " << sanity.max;
  out << "\nВремя смены: " << FormatTime(world.shift_time_seconds) << " / "
      << FormatTime(world.shift_duration_seconds);
  out << "\nФаза: " << PhaseName(world.phase);
  out << "\nСообщение: "
      << (world.message.empty() ? "Тишина давит сильнее шума." : world.message);
  if (!prompt.empty()) {
    out << "\nF - " << prompt;
  }
  out << "\nWASD/стрелки - движение, F - действие, E - инвентарь, "
         "Esc - выход\n";
  return out.str();
}
