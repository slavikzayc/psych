#include "systems/HUDSystem.h"

#include <iomanip>
#include <sstream>

namespace {
std::string PhaseName(NightPhase phase) {
  // Перевод enum-фазы в короткую строку для HUD.
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
  // Форматирует секунды как MM:SS.
  std::ostringstream out;
  out << std::setfill('0') << std::setw(2) << (seconds / 60) << ":"
      << std::setw(2) << (seconds % 60);
  return out.str();
}
} // namespace

std::string hud_system::BuildHud(const WorldState &world,
                                 const std::string &prompt) {
  // HUD строится как строка, которую потом RenderSystem добавляет под карту.
  std::ostringstream out;

  // Рассудок читается из SanityComponent игрока.
  const auto sanity_it = world.registry.sanities.find(world.player);
  if (sanity_it != world.registry.sanities.end()) {
    out << "\nРассудок: " << sanity_it->second.current << " / "
        << sanity_it->second.max;
  }
  out << "\nВремя смены: " << FormatTime(world.shift_time_seconds) << " / "
      << FormatTime(world.shift_duration_seconds);
  out << "\nФаза: " << PhaseName(world.phase);
  out << "\nСообщение: "
      << (world.message.empty() ? "Тишина давит сильнее шума." : world.message);
  if (!prompt.empty()) {
    // prompt появляется только если рядом есть интерактивный объект.
    out << "\nF - " << prompt;
  }
  out << "\nWASD/стрелки - движение, F - действие, E - инвентарь, "
         "Esc - выход\n";
  return out.str();
}
