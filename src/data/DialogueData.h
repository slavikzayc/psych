#pragma once

#include <string>

struct DialogueData {
  int id = 0;
  int start_node_id = 0;
  std::string success_text;
  std::string fail_text;
};

struct DialogueNodeData {
  int id = 0;
  int dialogue_id = 0;
  std::string patient_text;
  std::string state_hint;
};

struct ReplyData {
  int id = 0;
  int node_id = 0;
  std::string text;
  int base_impact = 0;
  int min_authority = 0;
  int min_medication = 0;
  int authority_scale = 0;
  int medication_scale = 0;
  int fail_sanity_damage = 0;
  int next_node_id = 0;
};
