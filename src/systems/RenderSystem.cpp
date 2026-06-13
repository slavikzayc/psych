#include "systems/RenderSystem.h"

#include <algorithm>
#include <sstream>

namespace {
std::string Marker(bool selected) { return selected ? "> " : "  "; }

bool ReplyAvailable(const ReplyData &reply, const StatsComponent &stats) {
  // Реплика доступна только если характеристики игрока удовлетворяют
  // требованиям.
  return stats.authority >= reply.min_authority &&
         stats.medication >= reply.min_medication;
}
} // namespace

std::string render_system::RenderExploration(const WorldState &world,
                                             const GameDatabase &db,
                                             const std::string &prompt) {
  // Вся отрисовка собирается в строковый поток, а Game::Render уже печатает
  // кадр.
  std::ostringstream out;

  // Берём текущую карту по id из WorldState.
  const auto map_it = db.maps.find(world.currentmap_id);
  if (map_it == db.maps.end()) {
    return "Карта не загружена.\n";
  }

  out << "Комната: " << map_it->second.name << "\n\n";

  // canvas — изменяемая копия layout карты.
  // В неё будут поверх базовых тайлов наложены символы сущностей.
  std::vector<std::string> canvas = map_it->second.layout;

  // Сущности сортируются по layer, чтобы игрок рисовался поверх
  // предметов/дверей.
  std::vector<std::pair<int, Entity>> ordered;
  for (const auto &[entity, renderable] : world.registry.renderables) {
    // Не рисуем сущности, которые находятся на другой карте.
    const auto map_entity_it = world.registry.maps.find(entity);
    if (map_entity_it != world.registry.maps.end() &&
        map_entity_it->second.map_id != world.currentmap_id) {
      continue;
    }
    ordered.push_back({renderable.layer, entity});
  }
  std::sort(ordered.begin(), ordered.end());

  // Накладываем символы сущностей на canvas.
  for (const auto &[layer, entity] : ordered) {
    (void)layer;
    const auto pos_it = world.registry.positions.find(entity);
    const auto render_it = world.registry.renderables.find(entity);
    if (pos_it == world.registry.positions.end() ||
        render_it == world.registry.renderables.end()) {
      continue;
    }
    if (pos_it->second.y >= 0 &&
        pos_it->second.y < static_cast<int>(canvas.size()) &&
        pos_it->second.x >= 0 &&
        pos_it->second.x < static_cast<int>(canvas[pos_it->second.y].size())) {
      // Позиция находится внутри canvas, значит символ можно безопасно
      // записать.
      canvas[pos_it->second.y][pos_it->second.x] = render_it->second.symbol;
    }
  }

  // Выводим строки карты.
  for (const std::string &row : canvas) {
    out << row << "\n";
  }
  out << hud_system::BuildHud(world, prompt);
  return out.str();
}

std::string render_system::RenderMainMenu(int selected) {
  // selected определяет, перед каким пунктом стоит маркер "> ".
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
  // Инвентарь показывает слоты игрока и имена предметов из GameDatabase.
  std::ostringstream out;
  out << "Инвентарь\n\n";
  const auto inv_it = world.registry.inventories.find(world.player);
  if (inv_it == world.registry.inventories.end() ||
      inv_it->second.slots.empty()) {
    out << "Пусто.\n";
  } else {
    for (std::size_t i = 0; i < inv_it->second.slots.size(); ++i) {
      const InventorySlot &slot = inv_it->second.slots[i];
      const auto item_it = db.items.find(slot.item_id);
      const std::string name = item_it == db.items.end() ? "Неизвестный предмет"
                                                         : item_it->second.name;
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
  // Экран диалога берёт данные из трёх мест:
  // PatientComponent, DialogueCombatComponent и JSON-таблиц GameDatabase.
  std::ostringstream out;
  const auto patient_it = world.registry.patients.find(world.active_patient);
  const auto combat_it =
      world.registry.dialogue_combats.find(world.active_patient);
  const auto sanity_it = world.registry.sanities.find(world.player);
  const auto stats_it = world.registry.stats.find(world.player);
  if (patient_it == world.registry.patients.end() ||
      combat_it == world.registry.dialogue_combats.end()) {
    return "Диалог недоступен.\n";
  }

  // Текущий узел определяет текст пациента и список доступных реплик.
  const PatientComponent &patient = patient_it->second;
  const DialogueCombatComponent &combat = combat_it->second;
  const auto data_it = db.patients.find(patient.patient_id);
  const auto node_it = db.dialogue_nodes.find(combat.current_node_id);

  out << (data_it == db.patients.end() ? "Пациент" : data_it->second.name)
      << "\n";
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
    const StatsComponent stats = stats_it == world.registry.stats.end()
                                     ? StatsComponent{}
                                     : stats_it->second;
    for (std::size_t i = 0; i < replies_it->second.size(); ++i) {
      // Недоступные реплики показываются, но помечаются требованием.
      const ReplyData &reply = replies_it->second[i];
      out << (i + 1) << ". " << reply.text;
      if (!ReplyAvailable(reply, stats)) {
        out << " [требование не выполнено]";
      }
      out << "\n";
    }
  }

  // 0 всегда доступен как безопасный выход из диалога.
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
