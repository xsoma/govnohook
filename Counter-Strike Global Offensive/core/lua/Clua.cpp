// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "CLua.h"	
#include <ShlObj_core.h>
#include <Windows.h>
#include <any>
#include <visuals/visuals.hpp>
#include <sdk/sdk.hpp>
#include <core/source.hpp>
#include <hooks/hooked.hpp>
#include <menu/menu/menu.hpp>
#include <configs/configs.h>
void lua_panic(sol::optional <std::string> message)
{
	if (!message)
		return;

	//auto log = /*sxor*/("Lua error: ") + message.value_or("unknown");
	//eventlogs::get().add(log, false);
}
std::string get_current_script(sol::this_state s)
{
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = source.substr(1).c_str();

	filename.erase(0, 16);

	return filename;
}
int get_current_script_id(sol::this_state s)
{
	return c_lua::get().get_script_id(get_current_script(s));
}
std::vector <std::pair <std::string, menu_item>>::iterator find_item(std::vector <std::pair <std::string, menu_item>>& items, const std::string& name)
{
	for (auto it = items.begin(); it != items.end(); ++it)
		if (it->first == name)
			return it;

	return items.end();
}
menu_item find_item(std::vector <std::vector <std::pair <std::string, menu_item>>>& scripts, const std::string& name)
{
	for (auto& script : scripts)
	{
		for (auto& item : script)
		{
			std::string item_name;

			auto first_point = false;
			auto second_point = false;

			for (auto& c : item.first)
			{
				if (c == '.')
				{
					if (first_point)
					{
						second_point = true;
						continue;
					}
					else
					{
						first_point = true;
						continue;
					}
				}

				if (!second_point)
					continue;

				item_name.push_back(c);
			}

			if (item_name == name)
				return item.second;
		}
	}

	return menu_item();
}
namespace ns_material
{
	IMaterial* FindMaterial(const char* matname, const char* group)
	{
		return csgo.m_material_system()->FindMaterial(matname, group);
	}
}
namespace ns_client
{
	void add_callback(sol::this_state s, std::string eventname, sol::protected_function func)
	{
		if (c_lua::get().loaded.at(get_current_script_id(s)))//new
			c_lua::get().hooks.registerHook(eventname, get_current_script_id(s), func);
	}

	void load_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().load_script(c_lua::get().get_script_id(name));
	}

	void unload_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().unload_script(c_lua::get().get_script_id(name));
	}

	void log(std::string text)
	{
		_events.push_back(text);
	}
}
namespace ns_menu
{
	bool get_visible()
	{
		return feature::menu->_menu_opened;
	}

	void set_visible(bool visible)
	{
		feature::menu->_menu_opened = visible;
	}

	auto next_line_counter = 0;

	void next_line(sol::this_state s)
	{
		c_lua::get().items.at(get_current_script_id(s)).emplace_back(std::make_pair(sxor("next_line_") + std::to_string(next_line_counter), menu_item()));
		++next_line_counter;
	}

	void add_check_box(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(false)));
	}

	void add_combo_box(sol::this_state s, std::string name, std::vector <std::string> labels) //-V813
	{
		if (labels.empty())
			return;

		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(labels, 0)));
	}

	void add_slider_int(sol::this_state s, const std::string& name, int min, int max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_slider_float(sol::this_state s, const std::string& name, float min, float max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_color_picker(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(Color(255, 255, 255, 255))));
	}

	void add_key_bind(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(0, 0)));
	}

	std::unordered_map <std::string, bool> first_update;
	std::unordered_map <std::string, menu_item> stored_values;
	std::unordered_map <std::string, void*> config_items;

	bool find_config_item(std::string name, std::string type)
	{
		if (config_items.find(name) == config_items.end())
		{
			auto found = false;

			for (auto item : cfg_manager->items)
			{
				if (item->name == name)
				{
					if (item->type != type)
					{
						//eventlogs::get().add(sxor("Lua error: invalid config item type, must be ") + type, false);
						auto log = sxor("Lua error: invalid config item type, must be ") + type;
						_events.push_back(log);
						return false;
					}

					found = true;
					config_items[name] = item->pointer;
					break;
				}
			}

			if (!found)
			{
				//eventlogs::get().add(sxor("Lua error: cannot find config variable \"") + name + '\"', false);

				auto log = sxor("Lua error: cannot find config variable \"") + name + '\"';
				_events.push_back(log);
				return false;
			}
		}

		return true;
	}

	bool get_bool(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!feature::menu->_menu_opened && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].check_box_value;
			else if (config_items.find(name) != config_items.end())
				return *(bool*)config_items[name];
			else
				return false;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, sxor("bool")))
				return *(bool*)config_items[name];

			//eventlogs::get().add(sxor("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = sxor("Lua error: cannot find menu variable \"") + name + '\"';
			_events.push_back(log);
			return false;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.check_box_value;
	}

	int get_int(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!feature::menu->_menu_opened && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].type == COMBO_BOX ? stored_values[name].combo_box_value : stored_values[name].slider_int_value;
			else if (config_items.find(name) != config_items.end())
				return *(int*)config_items[name]; //-V206
			else
				return 0;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, sxor("int")))
				return *(int*)config_items[name]; //-V206

			//eventlogs::get().add(sxor("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = sxor("Lua error: cannot find menu variable \"") + name + '\"';
			_events.push_back(log);
			return 0;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.type == COMBO_BOX ? it.combo_box_value : it.slider_int_value;
	}

	float get_float(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!feature::menu->_menu_opened && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].slider_float_value;
			else if (config_items.find(name) != config_items.end())
				return *(float*)config_items[name];
			else
				return 0.0f;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, sxor("float")))
				return *(float*)config_items[name];

			//eventlogs::get().add(sxor("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = sxor("Lua error: cannot find menu variable \"") + name + '\"';
			_events.push_back(log);
			return 0.0f;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.slider_float_value;
	}

	Color get_color(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!feature::menu->_menu_opened && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return Color(stored_values[name].color_picker_value[0] * 255.f, stored_values[name].color_picker_value[1] * 255.f, stored_values[name].color_picker_value[2] * 255.f, stored_values[name].color_picker_value[3] * 255.f);
			else if (config_items.find(name) != config_items.end())
				return *(Color*)config_items[name];
			else
				return Color(255, 255, 255, 255);
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, sxor("Color")))
				return *(Color*)config_items[name];

			//eventlogs::get().add(sxor("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = sxor("Lua error: cannot find menu variable \"") + name + '\"';
			_events.push_back(log);

			return Color(255, 255, 255, 255);
		}

		first_update[name] = true;
		stored_values[name] = it;

		return Color(it.color_picker_value[0] * 255.f, it.color_picker_value[1] * 255.f, it.color_picker_value[2] * 255.f, it.color_picker_value[3] * 255.f);
	}
	int get_keybind_key(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!feature::menu->_menu_opened && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].keybind_key_value;
			else if (config_items.find(name) != config_items.end())
				return *(int*)config_items[name];
			else
				return 0;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, sxor("int")))
				return *(int*)config_items[name];

			//eventlogs::get().add(sxor("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = sxor("Lua error: cannot find menu variable \"") + name + '\"';
			_events.push_back(log);

			return 0;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.keybind_key_value;
	}
	bool get_key_bind_state(int key_bind, int mode)
	{
		return ctx.get_key_press(c_keybind{ key_bind , mode, false, 0 });
	}
	int get_keybind_mode(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!feature::menu->_menu_opened && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].keybind_key_mode;
			else if (config_items.find(name) != config_items.end())
				return *(int*)config_items[name];
			else
				return 0;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, sxor("int")))
				return *(int*)config_items[name];

			//eventlogs::get().add(sxor("Lua error: cannot find menu variable \"") + name + '\"', false);

			auto log = sxor("Lua error: cannot find menu variable \"") + name + '\"';
			_events.push_back(log);

			return 0;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.keybind_key_mode;
	}
	void set_bool(std::string name, bool value)
	{
		if (!find_config_item(name, sxor("bool")))
			return;

		*(bool*)config_items[name] = value;
	}

	void set_int(std::string name, int value)
	{
		if (!find_config_item(name, sxor("int")))
			return;

		*(int*)config_items[name] = value; //-V206
	}

	void set_float(std::string name, float value)
	{
		if (!find_config_item(name, sxor("float")))
			return;

		*(float*)config_items[name] = value;
	}

	void set_color(std::string name, Color value)
	{
		if (!find_config_item(name, sxor("Color")))
			return;

		*(Color*)config_items[name] = value;
	}
}
namespace ns_globals
{
	int get_framerate()
	{
		return ctx.fps;
	}

	int get_ping()
	{
		return ctx.latency[0];
	}

	std::string get_server_address()
	{
		if (!csgo.m_engine()->IsInGame())
			return "Unknown";

		auto nci = csgo.m_engine()->GetNetChannelInfo();

		if (!nci)
			return "Unknown";

		auto server = nci->GetAddress();

		if (!strcmp(server, "loopback"))
			server = "Local server";
		else if (csgo.m_game_rules()->IsValveDS())
			server = "Valve server";

		return server;
	}

	float get_realtime()
	{
		return csgo.m_globals()->realtime;
	}

	float get_curtime()
	{
		return csgo.m_globals()->curtime;
	}

	float get_frametime()
	{
		return csgo.m_globals()->frametime;
	}

	int get_tickcount()
	{
		return csgo.m_globals()->tickcount;
	}

	int get_framecount()
	{
		return csgo.m_globals()->framecount;
	}

	float get_intervalpertick()
	{
		return csgo.m_globals()->interval_per_tick;
	}

	int get_maxclients()
	{
		return csgo.m_globals()->maxClients;
	}
}
namespace ns_engine
{
	static int width, height;

	int get_screen_width()
	{
		csgo.m_engine()->GetScreenSize(width, height);
		return width;
	}

	int get_screen_height()
	{
		csgo.m_engine()->GetScreenSize(width, height);
		return height;
	}

	player_info get_player_info(int i)
	{
		player_info player_info;
		csgo.m_engine()->GetPlayerInfo(i, &player_info);

		return player_info;
	}

	int get_player_for_user_id(int i)
	{
		return csgo.m_engine()->GetPlayerForUserID(i);
	}

	int get_local_player_index()
	{
		return csgo.m_engine()->GetLocalPlayer();
	}

	Vector get_view_angles()
	{
		Vector view_angles;
		csgo.m_engine()->GetViewAngles(view_angles);

		return view_angles;
	}

	void set_view_angles(Vector view_angles)
	{
		Math::normalize_angle(view_angles.x);
		Math::normalize_angle(view_angles.y);
		Math::normalize_angle(view_angles.z);
		csgo.m_engine()->SetViewAngles(view_angles);
	}

	bool is_in_game()
	{
		return csgo.m_engine()->IsInGame();
	}

	bool is_connected()
	{
		return csgo.m_engine()->IsConnected();
	}

	std::string get_level_name()
	{
		return csgo.m_engine()->GetLevelName();
	}

	std::string get_level_name_short()
	{
		return csgo.m_engine()->GetLevelNameShort();
	}

	std::string get_map_group_name()
	{
		return csgo.m_engine()->GetMapGroupName();
	}

	bool is_playing_demo()
	{
		return csgo.m_engine()->IsPlayingDemo();
	}

	bool is_recording_demo()
	{
		return csgo.m_engine()->IsRecordingDemo();
	}

	bool is_paused()
	{
		return csgo.m_engine()->IsPaused();
	}

	bool is_taking_screenshot()
	{
		return csgo.m_engine()->IsTakingScreenshot();
	}

	bool is_hltv()
	{
		return csgo.m_engine()->IsHLTV();
	}
}
#define ZERO Vector(0.0f, 0.0f, 0.0f)

enum FontCenteringFlags
{
	HFONT_CENTERED_NONE = (1 << 0),
	HFONT_CENTERED_X = (1 << 1),
	HFONT_CENTERED_Y = (1 << 2)
};
namespace ns_render
{
	Vector world_to_screen(const Vector& world)
	{
		Vector screen;

		if (!Drawing::WorldToScreen(world, screen))
			return ZERO;

		return screen;
	}

	int get_text_width(vgui::HFont font, const std::string& text)
	{
		return Drawing::GetTextSize(font, text.c_str()).right;
	}

	int get_text_height(vgui::HFont font, const std::string& text)
	{
		return Drawing::GetTextSize(font, text.c_str()).bottom;
	}

	vgui::HFont create_font(const std::string& name, float size, float weight, std::optional <bool> antialias, std::optional <bool> dropshadow, std::optional <bool> outline)
	{
		DWORD flags = FONTFLAG_NONE;

		if (antialias.value_or(false))
			flags |= FONTFLAG_ANTIALIAS;

		if (dropshadow.value_or(false))
			flags |= FONTFLAG_DROPSHADOW;

		if (outline.value_or(false))
			flags |= FONTFLAG_OUTLINE;

		//g_ctx.last_font_name = name;

		auto font = csgo.m_surface()->CreateFont_();
		csgo.m_surface()->SetFontGlyphSet(font, name.c_str(), (int)size, (int)weight, 0, 0, flags);

		return font;
	}

	void draw_text(vgui::HFont font, float x, float y, Color color, const std::string& text)
	{
		Drawing::DrawString(font, (int)x, (int)y, color, HFONT_CENTERED_NONE, text.c_str());
	}

	void draw_text_centered(vgui::HFont font, float x, float y, Color color, bool centered_x, bool centered_y, const std::string& text)
	{
		DWORD centered_flags = HFONT_CENTERED_NONE;

		if (centered_x)
		{
			centered_flags &= ~HFONT_CENTERED_NONE; //-V753
			centered_flags |= HFONT_CENTERED_X;
		}

		if (centered_y)
		{
			centered_flags &= ~HFONT_CENTERED_NONE;
			centered_flags |= HFONT_CENTERED_Y;
		}

		Drawing::DrawString(font, (int)x, (int)y, color, centered_flags, text.c_str());
	}

	void draw_line(float x, float y, float x2, float y2, Color color)
	{
		Drawing::DrawLine((int)x, (int)y, (int)x2, (int)y2, color);
	}

	void draw_rect(float x, float y, float w, float h, Color color)
	{
		Drawing::DrawRect((int)x, (int)y, (int)w, (int)h, color);
	}

	void draw_rect_filled(float x, float y, float w, float h, Color color)
	{
		Drawing::DrawRect((int)x, (int)y, (int)w, (int)h, color);
	}

	/*void draw_rect_filled_gradient(float x, float y, float w, float h, Color color, Color color2, int gradient_type)
	{
		gradient_type = std::clamp(gradient_type, 0, 1);
		Drawing::grad((int)x, (int)y, (int)w, (int)h, color, color2, (GradientType)gradient_type);
	}*/

	void draw_circle(float x, float y, float points, float radius, Color color)
	{
		Drawing::DrawCircle((int)x, (int)y, (int)points, (int)radius, color);
	}

	void draw_circle_filled(float x, float y, float points, float radius, Color color)
	{
		Drawing::DrawFilledCircle((int)x, (int)y, (int)points, (int)radius, color);
	}
	void draw_arc(int x, int y, int radius, int start_angle, int percent, int thickness, Color color) {
		auto precision = (2 * M_PI) / 30;
		auto step = M_PI / 180;
		auto inner = radius - thickness;
		auto end_angle = (start_angle + percent) * step;
		auto start_angles = (start_angle * M_PI) / 180;

		for (; radius > inner; --radius) {
			for (auto angle = start_angles; angle < end_angle; angle += precision) {
				auto cx = std::round(x + radius * std::cos(angle));
				auto cy = std::round(y + radius * std::sin(angle));

				auto cx2 = std::round(x + radius * std::cos(angle + precision));
				auto cy2 = std::round(y + radius * std::sin(angle + precision));

				Drawing::DrawLine(cx, cy, cx2, cy2, color);
			}
		}
	}
	void draw_triangle(float x, float y, float x2, float y2, float x3, float y3, Color color)
	{
		Drawing::filled_tilted_triangle(Vector2D(x, y), Vector2D(x2, y2), Vector2D(x3, y3), false, 10, color);
	}
}
namespace ns_console
{
	void execute(std::string& command)
	{
		if (command.empty())
			return;

		csgo.m_engine()->ExecuteClientCmd(command.c_str());
	}

	std::unordered_map <std::string, ConVar*> convars;

	bool get_bool(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = csgo.m_engine_cvars()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = sxor("Lua error: cannot find ConVar \"") + convar_name + '\"';
				_events.push_back(log);
				return false;
			}
		}

		if (!convars[convar_name])
			return false;

		return convars[convar_name]->GetBool();
	}

	int get_int(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = csgo.m_engine_cvars()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = sxor("Lua error: cannot find ConVar \"") + convar_name + '\"';
				_events.push_back(log);
				return 0;
			}
		}

		if (!convars[convar_name])
			return 0;

		return convars[convar_name]->GetInt();
	}

	float get_float(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = csgo.m_engine_cvars()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = sxor("Lua error: cannot find ConVar \"") + convar_name + '\"';
				_events.push_back(log);
				return 0.0f;
			}
		}

		if (!convars[convar_name])
			return 0.0f;

		return convars[convar_name]->GetFloat();
	}

	std::string get_string(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = csgo.m_engine_cvars()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = sxor("Lua error: cannot find ConVar \"") + convar_name + '\"';
				_events.push_back(log);
				return sxor("");
			}
		}

		if (!convars[convar_name])
			return "";

		return convars[convar_name]->GetString();
	}

	void set_bool(const std::string& convar_name, bool value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = csgo.m_engine_cvars()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = sxor("Lua error: cannot find ConVar \"") + convar_name + '\"';
				_events.push_back(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetBool() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_int(const std::string& convar_name, int value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = csgo.m_engine_cvars()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = sxor("Lua error: cannot find ConVar \"") + convar_name + '\"';
				_events.push_back(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetInt() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_float(const std::string& convar_name, float value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = csgo.m_engine_cvars()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = sxor("Lua error: cannot find ConVar \"") + convar_name + '\"';
				_events.push_back(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetFloat() != value) //-V550
			convars[convar_name]->SetValue(value);
	}

	void set_string(const std::string& convar_name, const std::string& value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = csgo.m_engine_cvars()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				auto log = sxor("Lua error: cannot find ConVar \"") + convar_name + '\"';
				_events.push_back(log);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetString() != value)
			convars[convar_name]->SetValue(value.c_str());
	}
}
namespace ns_entitylist
{
	sol::optional <C_BasePlayer*> get_local_player()
	{
		if (!csgo.m_engine()->IsInGame())
			return sol::optional <C_BasePlayer*>(sol::nullopt);

		return (C_BasePlayer*)csgo.m_entity_list()->GetClientEntity(csgo.m_engine()->GetLocalPlayer());
	}

	sol::optional <C_BasePlayer*> get_player_by_index(int i)
	{
		if (!csgo.m_engine()->IsInGame())
			return sol::optional <C_BasePlayer*>(sol::nullopt);

		return (C_BasePlayer*)csgo.m_entity_list()->GetClientEntity(i);
	}

	/*sol::optional <C_WeaponCSBaseGun*> get_weapon_by_player(sol::optional <C_BasePlayer*> player)
	{
		if (!csgo.m_engine()->IsInGame())
			return sol::optional <C_WeaponCSBaseGun*>(sol::nullopt);

		if (!player.value())
			return sol::optional <C_WeaponCSBaseGun*>(sol::nullopt);

		return player.value()->get_weapon();
	}*/

}
namespace ns_cmd
{
	int get_choke()
	{
		return csgo.m_client_state()->m_iChockedCommands;
	}
}
namespace ns_utils
{
	uint64_t find_signature(const std::string& szModule, const std::string& szSignature)
	{
		return ns_utils::find_signature(szModule.c_str(), szSignature.c_str());
	}
}
sol::state lua;

void c_lua::initialize()
{
	lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::ffi, sol::lib::jit, sol::lib::utf8);

	lua[/*sxor*/("collectgarbage")] = sol::nil;
	lua[/*sxor*/("dofilsse")] = sol::nil;
	lua[/*sxor*/("load")] = sol::nil;
	lua[/*sxor*/("loadfile")] = sol::nil;
	lua[/*sxor*/("pcall")] = sol::nil;
	lua[/*sxor*/("print")] = sol::nil;
	lua[/*sxor*/("xpcall")] = sol::nil;
	lua[/*sxor*/("getmetatable")] = sol::nil;
	lua[/*sxor*/("setmetatable")] = sol::nil;
	lua[/*sxor*/("__nil_callback")] = []() {};

	lua.new_usertype<C_BaseEntity>(sxor("entity"), // new
		(std::string)sxor("get_prop_int"), &C_BaseEntity::GetPropInt,
		(std::string)sxor("get_prop_float"), &C_BaseEntity::GetPropFloat,
		(std::string)sxor("get_prop_bool"), &C_BaseEntity::GetPropBool,
		(std::string)sxor("get_prop_string"), &C_BaseEntity::GetPropString,
		(std::string)sxor("set_prop_int"), &C_BaseEntity::SetPropInt,
		(std::string)sxor("set_prop_float"), &C_BaseEntity::SetPropFloat,
		(std::string)sxor("set_prop_bool"), &C_BaseEntity::SetPropBool,
		(std::string)sxor("set_prop_bool"), &C_BaseEntity::SetPropString
		);

	lua.new_usertype<IMaterial>(sxor("material"), // new
		(std::string)sxor("GetMaterialVarFlag"), &IMaterial::GetMaterialVarFlag,
		(std::string)sxor("SetMaterialVarFlag"), &IMaterial::SetMaterialVarFlag
		);

	lua.new_usertype <Color>(sxor("color"), sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)>(),
		(std::string)sxor("r"), &Color::r,
		(std::string)sxor("g"), &Color::g,
		(std::string)sxor("b"), &Color::b,
		(std::string)sxor("a"), &Color::a
		);

	/*lua.new_usertype <Vector>(sxor("vector"), sol::constructors <Vector(), Vector(float, float, float)>(),
		(std::string)sxor("x"), &Vector::x,
		(std::string)sxor("y"), &Vector::y,
		(std::string)sxor("z"), &Vector::z,
		(std::string)sxor("length"), &Vector::Length,
		(std::string)sxor("length_sqr"), &Vector::LengthSquared,
		(std::string)sxor("length_2d"), &Vector::Length2D,
		(std::string)sxor("length_2d_sqr"), &Vector::LengthSquared,
		(std::string)sxor("is_zero"), &Vector::IsZero,
		(std::string)sxor("is_valid"), &Vector::IsValid,
		(std::string)sxor("zero"), &Vector::Zero,
		(std::string)sxor("dist_to"), &Vector::Distance,
		(std::string)sxor("dist_to_sqr"), &Vector::DistanceSquared,
		(std::string)sxor("cross_product"), &Vector::Cross,
		(std::string)sxor("normalize"), &Vector::Normalize
		);*/

	lua.new_usertype <IGameEvent>(sxor("game_event"),
		(std::string)sxor("get_bool"), &IGameEvent::GetBool,
		(std::string)sxor("get_int"), &IGameEvent::GetInt,
		(std::string)sxor("get_float"), &IGameEvent::GetFloat,
		(std::string)sxor("get_string"), &IGameEvent::GetString,
		(std::string)sxor("set_bool"), &IGameEvent::SetBool,
		(std::string)sxor("set_int"), &IGameEvent::SetInt,
		(std::string)sxor("set_float"), &IGameEvent::SetFloat,
		(std::string)sxor("set_string"), &IGameEvent::SetString
		);

	lua.new_enum(sxor("hitboxes"),
		sxor("head"), HITBOX_HEAD,
		sxor("neck"), HITBOX_NECK,
		sxor("pelvis"), HITBOX_PELVIS,
		sxor("stomach"), HITBOX_THORAX,
		sxor("chest"), HITBOX_CHEST,
		sxor("upper_chest"), HITBOX_UPPER_CHEST,
		sxor("right_thigh"), HITBOX_RIGHT_THIGH,
		sxor("left_thigh"), HITBOX_LEFT_THIGH,
		sxor("right_calf"), HITBOX_RIGHT_CALF,
		sxor("left_calf"), HITBOX_LEFT_CALF,
		sxor("right_foot"), HITBOX_RIGHT_FOOT,
		sxor("left_foot"), HITBOX_LEFT_FOOT,
		sxor("right_hand"), HITBOX_RIGHT_HAND,
		sxor("left_hand"), HITBOX_LEFT_HAND,
		sxor("right_upper_arm"), HITBOX_RIGHT_UPPER_ARM,
		sxor("right_forearm"), HITBOX_RIGHT_FOREARM,
		sxor("left_upper_arm"), HITBOX_LEFT_UPPER_ARM,
		sxor("left_forearm"), HITBOX_LEFT_FOREARM
	);

	lua.new_enum(sxor("material_var_flags"),
		sxor("MATERIAL_VAR_DEBUG"), MATERIAL_VAR_DEBUG,
		sxor("MATERIAL_VAR_NO_DEBUG_OVERRIDE"), MATERIAL_VAR_NO_DEBUG_OVERRIDE,
		sxor("MATERIAL_VAR_NO_DRAW"), MATERIAL_VAR_NO_DRAW,
		sxor("MATERIAL_VAR_USE_IN_FILLRATE_MODE"), MATERIAL_VAR_USE_IN_FILLRATE_MODE,
		sxor("MATERIAL_VAR_VERTEXCOLOR"), MATERIAL_VAR_VERTEXCOLOR,
		sxor("MATERIAL_VAR_VERTEXALPHA"), MATERIAL_VAR_VERTEXALPHA,
		sxor("MATERIAL_VAR_SELFILLUM"), MATERIAL_VAR_SELFILLUM,
		sxor("MATERIAL_VAR_ADDITIVE"), MATERIAL_VAR_ADDITIVE,
		sxor("MATERIAL_VAR_ALPHATEST"), MATERIAL_VAR_ALPHATEST,
		sxor("MATERIAL_VAR_UNUSED"), MATERIAL_VAR_UNUSED,
		sxor("MATERIAL_VAR_ZNEARER"), MATERIAL_VAR_ZNEARER,
		sxor("MATERIAL_VAR_MODEL"), MATERIAL_VAR_MODEL,
		sxor("MATERIAL_VAR_FLAT"), MATERIAL_VAR_FLAT,
		sxor("MATERIAL_VAR_NOCULL"), MATERIAL_VAR_NOCULL,
		sxor("MATERIAL_VAR_NOFOG"), MATERIAL_VAR_NOFOG,
		sxor("MATERIAL_VAR_IGNOREZ"), MATERIAL_VAR_IGNOREZ,
		sxor("MATERIAL_VAR_DECAL"), MATERIAL_VAR_DECAL,
		sxor("MATERIAL_VAR_ENVMAPSPHERE"), MATERIAL_VAR_ENVMAPSPHERE,
		sxor("MATERIAL_VAR_UNUSED"), MATERIAL_VAR_UNUSED,
		sxor("MATERIAL_VAR_ENVMAPCAMERASPACE"), MATERIAL_VAR_ENVMAPCAMERASPACE,
		sxor("MATERIAL_VAR_BASEALPHAENVMAPMASK"), MATERIAL_VAR_BASEALPHAENVMAPMASK,
		sxor("MATERIAL_VAR_TRANSLUCENT"), MATERIAL_VAR_TRANSLUCENT,
		sxor("MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK"), MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK,
		sxor("MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING"), MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING,
		sxor("MATERIAL_VAR_OPAQUETEXTURE"), MATERIAL_VAR_OPAQUETEXTURE,
		sxor("MATERIAL_VAR_ENVMAPMODE"), MATERIAL_VAR_ENVMAPMODE,
		sxor("MATERIAL_VAR_SUPPRESS_DECALS"), MATERIAL_VAR_SUPPRESS_DECALS,
		sxor("MATERIAL_VAR_HALFLAMBERT"), MATERIAL_VAR_HALFLAMBERT,
		sxor("MATERIAL_VAR_WIREFRAME"), MATERIAL_VAR_WIREFRAME,
		sxor("MATERIAL_VAR_ALLOWALPHATOCOVERAGE"), MATERIAL_VAR_ALLOWALPHATOCOVERAGE,
		sxor("MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY"), MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY,
		sxor("MATERIAL_VAR_VERTEXFOG"), MATERIAL_VAR_VERTEXFOG
	);

	lua.new_usertype <C_BasePlayer>(sxor("player"), sol::base_classes, sol::bases<C_BaseEntity>(), //new
		(std::string)sxor("get_index"), &C_BasePlayer::entindex,
		(std::string)sxor("get_dormant"), &C_BasePlayer::IsDormant,
		(std::string)sxor("get_team"), &C_BasePlayer::m_iTeamNum,
		(std::string)sxor("get_lifestate"), &C_BasePlayer::IsDead,
		(std::string)sxor("get_velocity"), &C_BasePlayer::m_vecVelocity,
		(std::string)sxor("get_origin"), &C_BasePlayer::get_abs_origin,
		(std::string)sxor("get_angles"), &C_BasePlayer::m_angEyeAngles,
		(std::string)sxor("has_helmet"), &C_BasePlayer::m_bHasHelmet,
		(std::string)sxor("has_heavy_armor"), &C_BasePlayer::m_bHasHeavyArmor,
		(std::string)sxor("is_scoped"), &C_BasePlayer::m_bIsScoped,
		(std::string)sxor("get_health"), &C_BasePlayer::m_iHealth
		);

	/*lua.new_usertype <C_WeaponCSBaseGun> (sxor("weapon"), sol::base_classes, sol::bases<C_BaseEntity>(),
		(std::string)sxor("m_reload"), &C_WeaponCSBaseGun::m_reload,
		(std::string)sxor("can_fire"), &C_WeaponCSBaseGun::can,
		(std::string)sxor("can_aiming"), &C_WeaponCSBaseGun::is_non_aim,
		(std::string)sxor("can_double_tap"), &C_WeaponCSBaseGun::can_double_tap,
		(std::string)sxor("get_weapon_name"), &C_WeaponCSBaseGun::get_name,
		(std::string)sxor("get_inaccuracy"), &C_WeaponCSBaseGun::get_inaccuracy,
		(std::string)sxor("get_spread"), &C_WeaponCSBaseGun::get_spread
	);*/

	lua.new_enum(sxor("buttons"),
		sxor("in_attack"), IN_ATTACK,
		sxor("in_jump"), IN_JUMP,
		sxor("in_duck"), IN_DUCK,
		sxor("in_forward"), IN_FORWARD,
		sxor("in_back"), IN_BACK,
		sxor("in_use"), IN_USE,
		sxor("in_cancel"), IN_CANCEL,
		sxor("in_left"), IN_LEFT,
		sxor("in_right"), IN_RIGHT,
		sxor("in_moveleft"), IN_MOVELEFT,
		sxor("in_moveright"), IN_MOVERIGHT,
		sxor("in_attack2"), IN_ATTACK2,
		sxor("in_run"), IN_RUN,
		sxor("in_reload"), IN_RELOAD,
		sxor("in_alt1"), IN_ALT1,
		sxor("in_alt2"), IN_ALT2,
		sxor("in_score"), IN_SCORE,
		sxor("in_speed"), IN_SPEED,
		sxor("in_walk"), IN_WALK,
		sxor("in_zoom"), IN_ZOOM,
		sxor("in_weapon1"), IN_WEAPON1,
		sxor("in_weapon2"), IN_WEAPON2,
		sxor("in_bullrush"), IN_BULLRUSH,
		sxor("in_grenade1"), IN_GRENADE1,
		sxor("in_grenade2"), IN_GRENADE2
	);

	/*lua.new_usertype <shot_info> (sxor("shot_info"), sol::constructors <> (),
		(std::string)sxor("target_name"), &shot_info::target_name,
		(std::string)sxor("result"), &shot_info::result,
		(std::string)sxor("client_hitbox"), &shot_info::client_hitbox,
		(std::string)sxor("server_hitbox"), &shot_info::server_hitbox,
		(std::string)sxor("client_damage"), &shot_info::client_damage,
		(std::string)sxor("server_damage"), &shot_info::server_damage,
		(std::string)sxor("hitchance"), &shot_info::hitchance,
		(std::string)sxor("backtrack_ticks"), &shot_info::backtrack_ticks,
		(std::string)sxor("aim_point"), &shot_info::aim_point
	);*/

	auto client = lua.create_table();
	client[sxor("add_callback")] = ns_client::add_callback;
	client[sxor("load_script")] = ns_client::load_script;
	client[sxor("unload_script")] = ns_client::unload_script;
	client[sxor("add_log")] = ns_client::log;

	auto material_sys = lua.create_table();
	material_sys[sxor("find_material")] = ns_material::FindMaterial;

	auto menu = lua.create_table();
	menu[sxor("next_line")] = ns_menu::next_line;
	menu[sxor("add_check_box")] = ns_menu::add_check_box;
	menu[sxor("add_combo_box")] = ns_menu::add_combo_box;
	menu[sxor("add_slider_int")] = ns_menu::add_slider_int;
	menu[sxor("add_slider_float")] = ns_menu::add_slider_float;
	menu[sxor("add_color_picker")] = ns_menu::add_color_picker;
	menu[sxor("add_key_bind")] = ns_menu::add_key_bind;
	menu[sxor("get_key_bind_key")] = ns_menu::get_keybind_key;
	menu[sxor("get_key_bind_mode")] = ns_menu::get_keybind_mode;
	menu[sxor("get_key_bind_state")] = ns_menu::get_key_bind_state;
	menu[sxor("get_bool")] = ns_menu::get_bool;
	menu[sxor("get_int")] = ns_menu::get_int;
	menu[sxor("get_float")] = ns_menu::get_float;
	menu[sxor("get_color")] = ns_menu::get_color;
	menu[sxor("set_bool")] = ns_menu::set_bool;
	menu[sxor("set_int")] = ns_menu::set_int;
	menu[sxor("set_float")] = ns_menu::set_float;
	menu[sxor("set_color")] = ns_menu::set_color;



	auto globals = lua.create_table();
	globals[sxor("get_framerate")] = ns_globals::get_framerate;
	globals[sxor("get_ping")] = ns_globals::get_ping;
	globals[sxor("get_server_address")] = ns_globals::get_server_address;
	globals[sxor("get_realtime")] = ns_globals::get_realtime;
	globals[sxor("get_curtime")] = ns_globals::get_curtime;
	globals[sxor("get_frametime")] = ns_globals::get_frametime;
	globals[sxor("get_tickcount")] = ns_globals::get_tickcount;
	globals[sxor("get_framecount")] = ns_globals::get_framecount;
	globals[sxor("get_intervalpertick")] = ns_globals::get_intervalpertick;
	globals[sxor("get_maxclients")] = ns_globals::get_maxclients;

	auto engine = lua.create_table();
	engine[sxor("get_screen_width")] = ns_engine::get_screen_width;
	engine[sxor("get_screen_height")] = ns_engine::get_screen_height;
	engine[sxor("get_level_name")] = ns_engine::get_level_name;
	engine[sxor("get_level_name_short")] = ns_engine::get_level_name_short;
	engine[sxor("get_local_player_index")] = ns_engine::get_local_player_index;
	engine[sxor("get_map_group_name")] = ns_engine::get_map_group_name;
	engine[sxor("get_player_for_user_id")] = ns_engine::get_player_for_user_id;
	engine[sxor("get_player_info")] = ns_engine::get_player_info;
	engine[sxor("get_view_angles")] = ns_engine::get_view_angles;
	engine[sxor("is_connected")] = ns_engine::is_connected;
	engine[sxor("is_hltv")] = ns_engine::is_hltv;
	engine[sxor("is_in_game")] = ns_engine::is_in_game;
	engine[sxor("is_paused")] = ns_engine::is_paused;
	engine[sxor("is_playing_demo")] = ns_engine::is_playing_demo;
	engine[sxor("is_recording_demo")] = ns_engine::is_recording_demo;
	engine[sxor("is_taking_screenshot")] = ns_engine::is_taking_screenshot;
	engine[sxor("set_view_angles")] = ns_engine::set_view_angles;

	auto render = lua.create_table();
	render[sxor("world_to_screen")] = ns_render::world_to_screen;
	render[sxor("get_text_width")] = ns_render::get_text_width;
	render[sxor("get_text_height")] = ns_render::get_text_height;
	render[sxor("create_font")] = ns_render::create_font;
	render[sxor("draw_text")] = ns_render::draw_text;
	render[sxor("draw_text_centered")] = ns_render::draw_text_centered;
	render[sxor("draw_line")] = ns_render::draw_line;
	render[sxor("draw_rect")] = ns_render::draw_rect;
	render[sxor("draw_rect_filled")] = ns_render::draw_rect_filled;
	render[sxor("draw_circle")] = ns_render::draw_circle;
	render[sxor("draw_circle_filled")] = ns_render::draw_circle_filled;
	render[sxor("draw_arc")] = ns_render::draw_arc;
	render[sxor("draw_triangle")] = ns_render::draw_triangle;

	auto console = lua.create_table();
	console[sxor("execute")] = ns_console::execute;
	console[sxor("get_int")] = ns_console::get_int;
	console[sxor("get_float")] = ns_console::get_float;
	console[sxor("get_string")] = ns_console::get_string;
	console[sxor("set_int")] = ns_console::set_int;
	console[sxor("set_float")] = ns_console::set_float;
	console[sxor("set_string")] = ns_console::set_string;

	auto entitylist = lua.create_table();
	entitylist[sxor("get_local_player")] = ns_entitylist::get_local_player;
	entitylist[sxor("get_player_by_index")] = ns_entitylist::get_player_by_index;
	//entitylist[sxor("get_weapon_by_player")] = ns_entitylist::get_weapon_by_player;

	auto cmd = lua.create_table();
	cmd[sxor("get_choke")] = ns_cmd::get_choke;

	auto utils = lua.create_table();//new
	utils[sxor("find_signature")] = ns_utils::find_signature;//new


	lua[sxor("cheat")] = client;
	lua[sxor("mat_system")] = material_sys;
	lua[sxor("ui")] = menu;
	lua[sxor("globals")] = globals;
	lua[sxor("engine")] = engine;
	lua[sxor("draw")] = render;
	lua[sxor("console")] = console;
	lua[sxor("entity_list")] = entitylist;
	lua[sxor("cmd")] = cmd;
	lua[sxor("utils")] = utils;
	refresh_scripts();
}

int c_lua::get_script_id(const std::string& name)
{
	for (auto i = 0; i < scripts.size(); i++)
		if (scripts.at(i) == name) //-V106
			return i;

	return -1;
}

void c_lua::refresh_scripts()
{
	//auto oldLoaded = loaded;
	//auto oldScripts = scripts;

	//loaded.clear();
	////pathes.clear();
	//scripts.clear();
	////ns_console::convars.clear();

	//std::string folder;
	//static TCHAR path[MAX_PATH];

	//folder = /*sxor*/("hyperion.uno\\lua\\");
	//CreateDirectory(folder.c_str(), NULL);

	//auto i = 0;

	//for (auto& entry : std::filesystem::directory_iterator(folder))
	//{
	//	if (entry.path().extension() == /*sxor*/(".lua") || entry.path().extension() == /*sxor*/(".luac"))
	//	{
	//		auto path = entry.path();
	//		auto filename = path.filename().string();

	//		auto didPut = false;

	//		for (auto i = 0; i < oldScripts.size(); i++)
	//		{
	//			if (filename == oldScripts.at(i)) //-V106
	//			{
	//				loaded.emplace_back(oldLoaded.at(i)); //-V106
	//				didPut = true;
	//			}
	//		}

	//		if (!didPut)
	//			loaded.emplace_back(false);

	//		pathes.emplace_back(path);
	//		scripts.emplace_back(filename);

	//		items.emplace_back(std::vector <std::pair <std::string, menu_item>>());
	//		++i;
	//	}
	//}

	auto oldLoaded = loaded;
	auto oldScripts = scripts;

	loaded.clear();
	////pathes.clear();
	scripts.clear();
	////ns_console::convars.clear();
	std::string folder;

	auto get_dir = [&folder]() -> void
	{
		static TCHAR path[MAX_PATH];

		folder = ("darkraihook\\lua\\");

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	scripts.clear();

	std::string path = folder + ("/*.lua");
	WIN32_FIND_DATA fd;

	HANDLE hFind = FindFirstFile(path.c_str(), &fd);
	auto i = 0;
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				//auto path = entry.path();
				auto filename = fd.cFileName;

				auto didPut = false;

				for (auto i = 0; i < oldScripts.size(); i++)
				{
					if (filename == oldScripts.at(i)) //-V106
					{
						loaded.emplace_back(oldLoaded.at(i)); //-V106
						didPut = true;
					}
				}

				if (!didPut)
					loaded.emplace_back(false);

				//pathes.emplace_back(path);
				scripts.emplace_back(filename);

				items.emplace_back(std::vector <std::pair <std::string, menu_item>>());
				++i;

				//scripts.push_back(fd.cFileName);
			}
		} while (FindNextFile(hFind, &fd));

		FindClose(hFind);
	}
}

void c_lua::load_script(int id)
{
	if (id == -1)
		return;

	if (loaded.at(id)) //-V106
		return;

	auto path = get_script_path(id);

	if (path == /*sxor*/(""))
		return;

	auto error_load = false;
	loaded.at(id) = true;
	lua.script_file(path,
		[&error_load](lua_State*, sol::protected_function_result result)
		{
			if (!result.valid())
			{
				sol::error error = result;
				auto log = /*sxor*/("Lua error: ") + (std::string)error.what();

				_events.push_back({ log });
				error_load = true;

			}

			return result;
		}
	);

	if (error_load | loaded.at(id) == false)
	{
		loaded.at(id) = false;
		return;
	}


	//-V106
	//g_ctx.globals.loaded_script = true;
}

void c_lua::unload_script(int id)
{
	if (id == -1)
		return;

	if (!loaded.at(id)) //-V106
		return;

	items.at(id).clear(); //-V106

	if (c_lua::get().events.find(id) != c_lua::get().events.end()) //-V807
		c_lua::get().events.at(id).clear();

	hooks.unregisterHooks(id);
	loaded.at(id) = false; //-V106
}

void c_lua::reload_all_scripts()
{
	for (auto current : scripts)
	{
		if (!loaded.at(get_script_id(current))) //-V106
			continue;

		unload_script(get_script_id(current));
		load_script(get_script_id(current));
	}
}

void c_lua::unload_all_scripts()
{
	for (auto s : scripts)
		unload_script(get_script_id(s));
}

std::string c_lua::get_script_path(const std::string& name)
{
	return get_script_path(get_script_id(name));
}

std::string c_lua::get_script_path(int id)
{
	if (id == -1)
		return /*sxor*/("");

	std::stringstream asd;

	asd << "darkraihook\\lua\\" << scripts.at(id).c_str();

	return asd.str().c_str(); //-V106
}