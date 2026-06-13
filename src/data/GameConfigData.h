#pragma once

struct StatAllocationConfig {
  // Стартовое значение авторитета на экране распределения характеристик.
  int start_authority = 1;

  // Стартовое значение медикации.
  int start_medication = 1;

  // Сколько свободных очков игрок может распределить.
  int free_points = 8;

  // Верхняя граница для каждой характеристики.
  int max_stat = 10;
};

struct SanityConfig {
  // Начальный рассудок игрока.
  int initial = 100;

  // Максимальный рассудок игрока.
  int max = 100;

  // Сколько рассудка теряется за один пассивный тик.
  int passive_drain_per_minute = 3;

  // Через сколько игровых секунд происходит пассивный тик.
  int passive_drain_interval_seconds = 12;
};

struct GameConfigData {
  // Карта, на которой начинается новая смена.
  int starting_map_id = 1;

  // Длительность смены в игровых секундах.
  int shift_duration_seconds = 900;

  // Множитель времени: real_dt * time_scale = game_dt.
  float time_scale = 2.0f;

  // Вложенные настройки распределения характеристик.
  StatAllocationConfig stat_allocation;

  // Вложенные настройки рассудка.
  SanityConfig sanity;
};
