#include "data/GameDatabase.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace {
using json = nlohmann::json;

std::string JoinPath(const std::string &base, const std::string &file) {
  // Маленький helper для сборки пути без зависимости от std::filesystem.
  // Если base уже заканчивается слешем, второй слеш не добавляется.
  if (base.empty())
    return file;
  const char last = base.back();
  if (last == '/' || last == '\\')
    return base + file;
  return base + "/" + file;
}

bool LoadJson(const std::string &path, json &out_json, std::string &error) {
  // Открываем файл как обычный текстовый поток.
  std::ifstream file(path);
  if (!file) {
    // Ошибка возвращается наружу строкой, чтобы Game мог показать её
    // пользователю.
    error = "Не удалось открыть JSON-файл: " + path;
    return false;
  }

  try {
    // nlohmann::json сам парсит поток и выбрасывает json::exception при ошибке.
    file >> out_json;
    return true;
  } catch (const json::exception &ex) {
    error = "Ошибка разбора JSON-файла " + path + ": " + ex.what();
    return false;
  }
}
} // namespace

bool GameDatabase::LoadAll(const std::string &assets_path) {
  // Перед загрузкой очищаем старые данные, чтобы повторный LoadAll не смешал
  // записи из предыдущей попытки с новыми.
  items.clear();
  patients.clear();
  dialogues.clear();
  dialogue_nodes.clear();
  replies_by_node.clear();
  maps.clear();
  spawn_points.clear();
  night_events.clear();
  config = GameConfigData{};
  last_error.clear();

  json root;
  std::string error;

  // game_config.json загружается первым, потому что его значения нужны Game
  // ещё до создания мира.
  if (!LoadJson(JoinPath(assets_path, "game_config.json"), root, error)) {
    last_error = error;
    return false;
  }

  try {
    // value(key, default) позволяет не падать, если необязательное поле
    // отсутствует. В таком случае остаётся значение по умолчанию из структуры.
    config.starting_map_id =
        root.value("starting_map_id", config.starting_map_id);
    config.shift_duration_seconds =
        root.value("shift_duration_seconds", config.shift_duration_seconds);
    config.time_scale = root.value("time_scale", config.time_scale);

    const json stat_config = root.value("stat_allocation", json::object());
    config.stat_allocation.start_authority = stat_config.value(
        "start_authority", config.stat_allocation.start_authority);
    config.stat_allocation.start_medication = stat_config.value(
        "start_medication", config.stat_allocation.start_medication);
    config.stat_allocation.free_points =
        stat_config.value("free_points", config.stat_allocation.free_points);
    config.stat_allocation.max_stat =
        stat_config.value("max_stat", config.stat_allocation.max_stat);

    const json sanity_config = root.value("sanity", json::object());
    config.sanity.initial =
        sanity_config.value("initial", config.sanity.initial);
    config.sanity.max = sanity_config.value("max", config.sanity.max);
    config.sanity.passive_drain_per_minute = sanity_config.value(
        "passive_drain_per_minute", config.sanity.passive_drain_per_minute);
    config.sanity.passive_drain_interval_seconds =
        sanity_config.value("passive_drain_interval_seconds",
                            config.sanity.passive_drain_interval_seconds);

    // Предметы грузятся в таблицу items[id].
    if (!LoadJson(JoinPath(assets_path, "items.json"), root, error)) {
      last_error = error;
      return false;
    }
    for (const json &item_value : root) {
      // Каждая запись JSON превращается в ItemData.
      ItemData item;
      item.id = item_value.value("id", 0);
      item.name = item_value.value("name", "");
      item.type = item_value.value("type", "");
      item.description = item_value.value("description", "");
      item.sanity_restore = item_value.value("sanity_restore", 0);
      item.dialogue_bonus = item_value.value("dialogue_bonus", 0);
      item.consumable = item_value.value("consumable", false);
      items[item.id] = item;
    }

    // Пациенты грузятся в таблицу patients[id].
    if (!LoadJson(JoinPath(assets_path, "patients.json"), root, error)) {
      last_error = error;
      return false;
    }
    for (const json &patient_value : root) {
      PatientData patient;
      patient.id = patient_value.value("id", 0);
      patient.name = patient_value.value("name", "");
      patient.archetype = patient_value.value("archetype", "");
      patient.max_tension = patient_value.value("max_tension", 0);
      patient.resistance = patient_value.value("resistance", 0);
      patient.dialogue_id = patient_value.value("dialogue_id", 0);
      patient.dialogue_cooldown_seconds = patient_value.value(
          "dialogue_cooldown_seconds", patient.dialogue_cooldown_seconds);
      patients[patient.id] = patient;
    }

    // dialogues.json содержит три связанные таблицы: dialogues, nodes и
    // replies.
    if (!LoadJson(JoinPath(assets_path, "dialogues.json"), root, error)) {
      last_error = error;
      return false;
    }
    for (const json &dialogue_value : root.value("dialogues", json::array())) {
      // Верхний уровень диалога: стартовый узел и тексты успеха/провала.
      DialogueData dialogue;
      dialogue.id = dialogue_value.value("id", 0);
      dialogue.start_node_id = dialogue_value.value("start_node_id", 0);
      dialogue.success_text = dialogue_value.value("success_text", "");
      dialogue.fail_text = dialogue_value.value("fail_text", "");
      dialogues[dialogue.id] = dialogue;
    }
    for (const json &node_value : root.value("nodes", json::array())) {
      // Узел диалога: реплика пациента и подсказка состояния.
      DialogueNodeData node;
      node.id = node_value.value("id", 0);
      node.dialogue_id = node_value.value("dialogue_id", 0);
      node.patient_text = node_value.value("patient_text", "");
      node.state_hint = node_value.value("state_hint", "");
      dialogue_nodes[node.id] = node;
    }
    for (const json &reply_value : root.value("replies", json::array())) {
      // Реплики группируются по node_id, чтобы DialogueCombatSystem мог быстро
      // получить список вариантов для текущего узла.
      ReplyData reply;
      reply.id = reply_value.value("id", 0);
      reply.node_id = reply_value.value("node_id", 0);
      reply.text = reply_value.value("text", "");
      reply.base_impact = reply_value.value("base_impact", 0);
      reply.min_authority = reply_value.value("min_authority", 0);
      reply.min_medication = reply_value.value("min_medication", 0);
      reply.authority_scale = reply_value.value("authority_scale", 0);
      reply.medication_scale = reply_value.value("medication_scale", 0);
      reply.fail_sanity_damage = reply_value.value("fail_sanity_damage", 0);
      reply.next_node_id = reply_value.value("next_node_id", 0);
      replies_by_node[reply.node_id].push_back(reply);
    }

    // Карты грузятся отдельно от сущностей. layout описывает только базовые
    // тайлы.
    if (!LoadJson(JoinPath(assets_path, "maps.json"), root, error)) {
      last_error = error;
      return false;
    }
    for (const json &map_value : root) {
      MapData map;
      map.id = map_value.value("id", 0);
      map.name = map_value.value("name", "");
      map.width = map_value.value("width", 0);
      map.height = map_value.value("height", 0);
      for (const json &row : map_value.value("layout", json::array())) {
        map.layout.push_back(row.get<std::string>());
      }
      maps[map.id] = map;
    }

    // Spawn-точки описывают, какие сущности создать поверх карт.
    if (!LoadJson(JoinPath(assets_path, "spawn_points.json"), root, error)) {
      last_error = error;
      return false;
    }
    for (const json &spawn_value : root) {
      SpawnPointData spawn;
      spawn.id = spawn_value.value("id", 0);
      spawn.map_id = spawn_value.value("map_id", 0);
      spawn.entity_type = spawn_value.value("entity_type", "");
      spawn.data_id = spawn_value.value("data_id", 0);
      spawn.x = spawn_value.value("x", 0);
      spawn.y = spawn_value.value("y", 0);
      spawn.target_map_id = spawn_value.value("target_map_id", 0);
      spawn.target_x = spawn_value.value("target_x", 0);
      spawn.target_y = spawn_value.value("target_y", 0);
      spawn.locked = spawn_value.value("locked", false);
      spawn.required_key_item_id =
          spawn_value.value("required_key_item_id", spawn.data_id);
      ;
      spawn.prompt_text = spawn_value.value("prompt_text", "");
      spawn.empty_prompt_text = spawn_value.value("empty_prompt_text", "");
      spawn_points.push_back(spawn);
    }

    // Ночные события запускаются по shift_time_seconds.
    if (!LoadJson(JoinPath(assets_path, "night_events.json"), root, error)) {
      last_error = error;
      return false;
    }
    for (const json &event_value : root) {
      NightEventData event;
      event.id = event_value.value("id", 0);
      event.phase = event_value.value("phase", "");
      event.trigger_time = event_value.value("trigger_time", 0);
      event.event_type = event_value.value("event_type", "");
      event.target_id = event_value.value("target_id", 0);
      event.message = event_value.value("message", "");
      night_events.push_back(event);
    }
  } catch (const json::exception &ex) {
    // Ошибки структуры JSON: например поле есть, но имеет неподходящий тип.
    last_error = std::string("Ошибка структуры JSON-данных: ") + ex.what();
    return false;
  } catch (const std::exception &ex) {
    // Остальные ошибки загрузки.
    last_error = std::string("Ошибка загрузки данных: ") + ex.what();
    return false;
  }

  return true;
}
