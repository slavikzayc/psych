#include "systems/RenderSystem.h"

#include <algorithm>
#include <sstream>

namespace {
std::string Marker(bool selected) { return selected ? "> " : "  "; }

bool ReplyAvailable(const ReplyData& reply, const StatsComponent& stats) {
  return stats.authority >= reply.min_authority && stats.medication >= reply.min_medication;
}
}  // namespace

std::string RenderSystem::RenderExploration(const WorldState& world, const GameDatabase& db,
                                            const std::string& prompt, const HUDSystem& hud) const {
  std::ostringstream out;
  const auto map_it = db.maps.find(world.currentmap_id);
  if (map_it == db.maps.end()) {
    return "Карта не загружена.\n";
  }

  out << "Комната: " << map_it->second.name << "\n\n";

  std::vector<std::string> canvas = map_it->second.layout;
  std::vector<std::pair<int, Entity>> ordered;
  for (const auto& [entity, renderable] : world.registry.renderables) {
    const auto map_entity_it = world.registry.maps.find(entity);
    if (map_entity_it != world.registry.maps.end() &&
        map_entity_it->second.map_id != world.currentmap_id) {
      continue;
    }
    ordered.push_back({renderable.layer, entity});
  }
  std::sort(ordered.begin(), ordered.end());

  for (const auto& [layer, entity] : ordered) {
    (void)layer;
    const auto pos_it = world.registry.positions.find(entity);
    const auto render_it = world.registry.renderables.find(entity);
    if (pos_it == world.registry.positions.end() || render_it == world.registry.renderables.end()) {
      continue;
    }
    if (pos_it->second.y >= 0 && pos_it->second.y < static_cast<int>(canvas.size()) &&
        pos_it->second.x >= 0 &&
        pos_it->second.x < static_cast<int>(canvas[pos_it->second.y].size())) {
      canvas[pos_it->second.y][pos_it->second.x] = render_it->second.symbol;
    }
  }

  for (const std::string& row : canvas) {
    out << row << "\n";
  }
  out << hud.BuildHud(world, prompt);
  return out.str();
}

std::string RenderSystem::RenderMainMenu(int selected) const {
  std::ostringstream out;
  out << "PSYCHEMPATHY\n\n";
  out << Marker(selected == 0) << "Начать игру\n";
  out << Marker(selected == 1) << "Выход\n\n";
  out << "W/S или стрелки - выбор, Enter - подтвердить\n";
  return out.str();
}

std::string RenderSystem::RenderStatAllocation(int authority, int medication, int points,
                                               int selected) const {
  std::ostringstream out;
  out << "Распределение характеристик\n\n";
  out << Marker(selected == 0) << "Авторитет: " << authority << "\n";
  out << Marker(selected == 1) << "Медикация: " << medication << "\n";
  out << "\nСвободные очки: " << points << "\n";
  out << Marker(selected == 2) << "Начать смену\n\n";
  out << "W/S - выбрать, A/D - изменить, Enter - начать\n";
  return out.str();
}

std::string RenderSystem::RenderInventory(const WorldState& world, const GameDatabase& db,
                                          int selected) const {
  std::ostringstream out;
  out << "Инвентарь\n\n";
  const auto inv_it = world.registry.inventories.find(world.player);
  if (inv_it == world.registry.inventories.end() || inv_it->second.slots.empty()) {
    out << "Пусто.\n";
  } else {
    for (std::size_t i = 0; i < inv_it->second.slots.size(); ++i) {
      const InventorySlot& slot = inv_it->second.slots[i];
      const auto item_it = db.items.find(slot.item_id);
      const std::string name = item_it == db.items.end() ? "Неизвестный предмет"
                                                         : item_it->second.name;
      out << Marker(selected == static_cast<int>(i)) << (i + 1) << ". " << name << " x"
          << slot.count << "\n";
    }
  }
  out << "\nEnter/U/цифра - использовать, Esc/E - назад\n";
  out << "Сообщение: " << (world.message.empty() ? "-" : world.message) << "\n";
  return out.str();
}

std::string RenderSystem::RenderDialogue(const WorldState& world, const GameDatabase& db) const {
  std::ostringstream out;
  const auto patient_it = world.registry.patients.find(world.active_patient);
  const auto combat_it = world.registry.dialogue_combats.find(world.active_patient);
  const auto sanity_it = world.registry.sanities.find(world.player);
  const auto stats_it = world.registry.stats.find(world.player);
  if (patient_it == world.registry.patients.end() ||
      combat_it == world.registry.dialogue_combats.end()) {
    return "Диалог недоступен.\n";
  }

  const PatientComponent& patient = patient_it->second;
  const DialogueCombatComponent& combat = combat_it->second;
  const auto data_it = db.patients.find(patient.patient_id);
  const auto node_it = db.dialogue_nodes.find(combat.current_node_id);

  out << (data_it == db.patients.end() ? "Пациент" : data_it->second.name) << "\n";
  if (data_it != db.patients.end()) {
    out << "Состояние: " << data_it->second.archetype << "\n";
  }
  if (sanity_it != world.registry.sanities.end()) {
    out << "Рассудок врача: " << sanity_it->second.current << " / "
        << sanity_it->second.max << "\n";
  }
  if (combat.pending_item_bonus > 0) {
    out << "Подготовка: медикаменты усилят следующую "
           "реплику.\n";
  }
  out << "\n";

  if (node_it != db.dialogue_nodes.end()) {
    out << node_it->second.state_hint << "\n";
    out << "\"" << node_it->second.patient_text << "\"\n\n";
  }

  const auto replies_it = db.replies_by_node.find(combat.current_node_id);
  std::size_t reply_count = 0;
  if (replies_it != db.replies_by_node.end()) {
    reply_count = replies_it->second.size();
    const StatsComponent stats =
        stats_it == world.registry.stats.end() ? StatsComponent{} : stats_it->second;
    for (std::size_t i = 0; i < replies_it->second.size(); ++i) {
      const ReplyData& reply = replies_it->second[i];
      out << (i + 1) << ". " << reply.text;
      if (!ReplyAvailable(reply, stats)) {
        out << " [требование не выполнено]";
      }
      out << "\n";
    }
  }

  out << "0. Уйти\n";
  out << "\nE - инвентарь, ";
  if (reply_count > 0) {
    out << "1-" << reply_count << " - выбрать реплику, ";
  }
  out << "0/Esc - уйти\n";
  out << "Сообщение: " << (world.message.empty() ? "-" : world.message) << "\n";
  return out.str();
}

std::string RenderSystem::RenderVictory(const WorldState& world) const {
  std::ostringstream out;
  out << "ПОБЕДА\n\n";
  out << "Смена окончена. Больница пережила ночь, а вы "
         "сохранили рассудок.\n";
  out << "\n" << world.message << "\n\nEnter - в меню, Esc - выход\n";
  return out.str();
}

std::string RenderSystem::RenderDefeat(const WorldState& world) const {
  std::ostringstream out;
  out << "ПОРАЖЕНИЕ\n\n";
  out << "Ночь оказалась сильнее. Коридоры больше не "
         "различаются между собой.\n";
  out << "\n" << world.message << "\n\nEnter - в меню, Esc - выход\n";
  return out.str();
}
