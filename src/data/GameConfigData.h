#pragma once

struct StatAllocationConfig {
  int start_authority = 1;

  int start_medication = 1;

  int free_points = 8;

  int max_stat = 10;
};

struct SanityConfig {
  int initial = 100;

  int max = 100;

  int passive_drain_per_minute = 3;

  int passive_drain_interval_seconds = 12;
};

struct GameConfigData {
  int starting_map_id = 1;

  int shift_duration_seconds = 900;

  float time_scale = 2.0f;

  StatAllocationConfig stat_allocation;

  SanityConfig sanity;
};
