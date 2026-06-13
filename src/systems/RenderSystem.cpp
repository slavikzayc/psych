#include "systems/RenderSystem.h"

#include <algorithm>
#include <sstream>

namespace {
std::string Marker(bool selected) { return selected ? "> " : "  "; }

bool ReplyAvailable(const ReplyData &reply, const StatsComponent &stats) {
  return stats.authority >= reply.min_authority &&
         stats.medication >= reply.min_medication;
}
}

std::string render_system::RenderExploration(const WorldState &world,
                                             const GameDatabase &db,
                                             const std::string &prompt) {
  std::ostringstream out;

  const MapData &map = db.maps.at(world.currentmap_id);
  out << "Комната: " << map.name << "\n\n";

  std::vector<std::string> canvas = map.layout;

  std::vector<std::pair<int, Entity>> ordered;
  for (const auto &[entity, renderable] : world.registry.renderables) {
    if (world.registry.maps.at(entity).map_id != world.currentmap_id) {
      continue;
    }
    ordered.push_back({renderable.layer, entity});
  }
  std::sort(ordered.begin(), ordered.end());

  for (const auto &[layer, entity] : ordered) {
    (void)layer;
    const PositionComponent &position = world.registry.positions.at(entity);
    const RenderableComponent &renderable =
        world.registry.renderables.at(entity);
    if (position.y >= 0 && position.y < static_cast<int>(canvas.size()) &&
        position.x >= 0 &&
        position.x < static_cast<int>(canvas[position.y].size())) {
      canvas[position.y][position.x] = renderable.symbol;
    }
  }

  for (const std::string &row : canvas) {
    out << row << "\n";
  }
  out << hud_system::BuildHud(world, prompt);
  return out.str();
}

std::string render_system::RenderMainMenu(int selected) {
  std::ostringstream out;
  out << "PSYCHEMPATHY\n\n";
  out << Marker(selected == 0) << "Начать игру\n";
  out << Marker(selected == 1) << "Выход\n\n";
  out << "W/S или стрелки - выбор, Enter - подтвердить\n";
  return out.str();
}

std::string render_system::RenderStatAllocation(int authority, int medication,
                                                int points, int selected) {
  std::ostringstream out;
  out << "Распределение характеристик\n\n";
  out << Marker(selected == 0) << "Авторитет: " << authority << "\n";
  out << Marker(selected == 1) << "Медикация: " << medication << "\n";
  out << "\nСвободные очки: " << points << "\n";
  out << Marker(selected == 2) << "Начать смену\n\n";
  out << "W/S - выбрать, A/D - изменить, Enter - начать\n";
  return out.str();
}

std::string render_system::RenderInventory(const WorldState &world,
                                           const GameDatabase &db,
                                           int selected) {
  std::ostringstream out;
  out << "Инвентарь\n\n";
  const InventoryComponent &inventory =
      world.registry.inventories.at(world.player);
  if (inventory.slots.empty()) {
    out << "Пусто.\n";
  } else {
    for (std::size_t i = 0; i < inventory.slots.size(); ++i) {
      const InventorySlot &slot = inventory.slots[i];
      const std::string &name = db.items.at(slot.item_id).name;
      out << Marker(selected == static_cast<int>(i)) << (i + 1) << ". " << name
          << " x" << slot.count << "\n";
    }
  }
  out << "\nEnter/U/цифра - использовать, Esc/E - назад\n";
  out << "Сообщение: " << (world.message.empty() ? "-" : world.message) << "\n";
  return out.str();
}

std::string render_system::RenderDialogue(const WorldState &world,
                                          const GameDatabase &db) {
  std::ostringstream out;

  const PatientComponent &patient =
      world.registry.patients.at(world.active_patient);
  const DialogueCombatComponent &combat =
      world.registry.dialogue_combats.at(world.active_patient);
  const SanityComponent &sanity = world.registry.sanities.at(world.player);
  const StatsComponent &stats = world.registry.stats.at(world.player);
  const PatientData &patient_data = db.patients.at(patient.patient_id);
  const DialogueNodeData &node = db.dialogue_nodes.at(combat.current_node_id);

  out << patient_data.name << "\n";
  out << "Состояние: " << patient_data.archetype << "\n";
  out << "Рассудок врача: " << sanity.current << " / " << sanity.max << "\n";
  if (combat.pending_item_bonus > 0) {
    out << "Подготовка: медикаменты усилят следующую "
           "реплику.\n";
  }
  out << "\n";

  out << node.state_hint << "\n";
  out << "\"" << node.patient_text << "\"\n\n";

  const auto &replies = db.replies_by_node.at(combat.current_node_id);
  std::size_t reply_count = 0;
  reply_count = replies.size();
  for (std::size_t i = 0; i < replies.size(); ++i) {
    const ReplyData &reply = replies[i];
    out << (i + 1) << ". " << reply.text;
    if (!ReplyAvailable(reply, stats)) {
      out << " [требование не выполнено]";
    }
    out << "\n";
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

std::string render_system::RenderVictory(const WorldState &world) {
  std::ostringstream out;
  out << "ПОБЕДА\n\n";
  out << "Смена окончена. Больница пережила ночь, а вы "
         "сохранили рассудок.\n";
  out << "\n" << world.message << "\n\nEnter - в меню, Esc - выход\n";
  return out.str();
}

std::string render_system::RenderDefeat(const WorldState &world) {
  std::ostringstream out;
  out << "ПОРАЖЕНИЕ\n\n";
  out << "Ночь оказалась сильнее. Коридоры больше не "
         "различаются между собой.\n";
  out << "\n" << world.message << "\n\nEnter - в меню, Esc - выход\n";
  return out.str();
}
