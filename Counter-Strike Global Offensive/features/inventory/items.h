#pragma once

#include <map>
#include <vector>
#include <unordered_map>
struct weapon_infos
{
	constexpr weapon_infos(const char* model, const char* icon = nullptr) :
		model(model),
		icon(icon)
	{}
	const char* model;
	const char* icon;
};

extern const std::map<short, weapon_infos> k_weapon_info;
extern const std::map<short, const char*> k_glove_names;
extern const std::map<short, const char*> k_knife_names;
extern const std::map<short, const char*> k_item_names;
extern const std::vector<std::pair<int, const char*>> k_inventory_names;
extern const std::vector<std::pair<int, const char*>> save_guns;