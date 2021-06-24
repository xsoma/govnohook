#include <core/hooks/hooked.hpp>
#include <menu/menu/menu.hpp>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <features/misc/misc.hpp>

#define width_menu 600
#define height_menu 416

// optimization.
struct subtab_icon_saved
{
	float alpha;
	std::string icon;
	std::string name;
};

// optimization.
struct keybinds_saved
{
	float alpha = 0.f;
	float start_listen_time = 0.f;
	bool prev_state = false;
	bool toggled = false;
	bool toggled_settings = false;
	Vector2D lock_settings_pos = {};
};

std::unordered_map<int, float> hover_alpha = {};
std::unordered_map<int, float> toggle_alpha = {};
std::unordered_map<int, float> tab_hover_shit_alpha = {};
std::unordered_map<int, keybinds_saved> was_pressed_keybind = {};
std::unordered_map<int, subtab_icon_saved> subtab_icons_alpha = {};
std::unordered_map<int, bool> was_holding = {};

auto main_color = Color(57, 121, 217);//Color(175, 255, 0);
auto main_color_fade = Color(57, 121, 217);//Color(120, 175, 0);

//std::unordered_map <int, Vector2D> combo_mouse_pos = {};
std::unordered_map <int, float> combo_buttons = {};
std::unordered_map <int, Vector2D> hovering = {};
std::unordered_map <int, bool> opened = {};

bool was_moved;

bool c_menu::mouse_in_pos(Vector start, Vector end)
{
	return ((_cursor_position.x >= start.x) && (_cursor_position.y >= start.y) && (_cursor_position.x <= end.x) && (_cursor_position.y <= end.y));
}

bool c_menu::mouse_in_pos(Vector2D start, Vector2D end)
{
	return ((_cursor_position.x >= start.x) && (_cursor_position.y >= start.y) && (_cursor_position.x <= end.x) && (_cursor_position.y <= end.y));
}