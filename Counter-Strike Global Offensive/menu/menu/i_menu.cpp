#include "horizon.hpp"
#include "i_menu.hpp"
#include <features/inventory/parser.h>
#include "source.hpp"
//#include "background_img.hpp"
//#include "imgui_extension.hpp"
#include <sdk/render/rendering.hpp>
#include "menu.hpp"
#include <features/misc/misc.hpp>
#include "menu/setup/settings.h"
#include <features/ragebot/rage_aimbot.hpp>
#include "inventory/inventorychanger.h"
#include "inventory/items.h"
#include <protobuf/Protobuffs.h>
#include "imgui/imgui_internal.h"
#include "lua/Clua.h"
#include <features/visuals/visuals.hpp>

#pragma region "chars"
std::string preview = ("None");
const char* flags[] =
{
	"Money",
	"Armor",
	"Kit",
	"Scoped",
	"FakeDucking",
	"Vulnerable"
};
const char* weapon_types[] =
{
	"text",
	"icon",
};
const char* execution_loc_types[] =
{
	"none (disabled)",
	"create move",
	"frame stage notify",
	"paint traverse"
};
const char* flags_types[] =
{
	"armor",
	"flash",
	"place",
	"pin",
	"bomb/hostage owner",
	"records info",
};

const char* logs_types[] =
{
	"hit log",
	"hurt log",
	"shot log",
	"miss log",
	"buy log",
};
const char* knifebot_types[] =
{
	"Swing",
	"Full stab",
};

const char* airstrafe_types[] =
{
	"View angles",
	"Movement keys",
};
const char* low_fps_mig[] =
{
	"Force low accuracy boost",
	"Limit targets",
	"If hitbox size < spread",
	"If distance too large"
};
const char* quick_stop[] =
{
	"Early",
	"Slow motion",
	"Duck",
	"Fake duck",
	"Move between shots",
	"Ignore Molotov"
};
const char* hitboxes[] =
{
	"Head",
	"Chest",
	"Stomach",
	"Arms",
	"Legs",
	"Feet"
};
const char* dropped_weapons[] =
{
	"Text",
	"Icon",
	"Glow",
	"Distance"
};
const char* weapon_list[] =
{
	"scar/g3sg1",
	"awp",
	"scout",
	"rifle",
	"pitols",
	"r8/deage",
	"other"
};

static const char* zweaponnames(const short id)
{
	switch (id)
	{
	case WEAPON_DEAGLE:
		return "deagle";
	case WEAPON_ELITE:
		return "elite";
	case WEAPON_FIVESEVEN:
		return "fiveseven";
	case WEAPON_GLOCK:
		return "glock";
	case WEAPON_AK47:
		return "ak47";
	case WEAPON_AUG:
		return "aug";
	case WEAPON_AWP:
		return "awp";
	case WEAPON_FAMAS:
		return "famas";
	case WEAPON_G3SG1:
		return "g3sg1";
	case WEAPON_GALILAR:
		return "galilar";
	case WEAPON_M249:
		return "m249";
	case WEAPON_M4A1_SILENCER:
		return "m4a1_silencer";
	case WEAPON_M4A1:
		return "m4a1";
	case WEAPON_MAC10:
		return "mac10";
	case WEAPON_P90:
		return "p90";
	case WEAPON_UMP45:
		return "ump45";
	case WEAPON_XM1014:
		return "xm1014";
	case WEAPON_BIZON:
		return "bizon";
	case WEAPON_MAG7:
		return "mag7";
	case WEAPON_NEGEV:
		return "negev";
	case WEAPON_SAWEDOFF:
		return "sawedoff";
	case WEAPON_TEC9:
		return "tec9";
	case WEAPON_HKP2000:
		return "hkp2000";
	case WEAPON_MP5SD:
		return "mp5sd";
	case WEAPON_MP7:
		return "mp7";
	case WEAPON_MP9:
		return "mp9";
	case WEAPON_NOVA:
		return "nova";
	case WEAPON_P250:
		return "p250";
	case WEAPON_SCAR20:
		return "scar20";
	case WEAPON_SG556:
		return "sg556";
	case WEAPON_SSG08:
		return "ssg08";
	case WEAPON_USP_SILENCER:
		return "usp_silencer";
	case WEAPON_CZ75A:
		return "cz75a";
	case WEAPON_REVOLVER:
		return "revolver";
	case WEAPON_KNIFE:
		return "knife";
	case WEAPON_KNIFE_T:
		return "knife_t";
	case WEAPON_KNIFE_M9_BAYONET:
		return "knife_m9_bayonet";
	case WEAPON_KNIFE_BAYONET:
		return "bayonet";
	case WEAPON_KNIFE_FLIP:
		return "knife_flip";
	case WEAPON_KNIFE_GUT:
		return "knife_gut";
	case WEAPON_KNIFE_KARAMBIT:
		return "knife_karambit";
	case WEAPON_KNIFE_TACTICAL:
		return "knife_tactical";
	case WEAPON_KNIFE_FALCHION:
		return "knife_falchion";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "knife_survival_bowie";
	case WEAPON_KNIFE_BUTTERFLY:
		return "knife_butterfly";
	case WEAPON_KNIFE_PUSH:
		return "knife_push";
	case WEAPON_KNIFE_URSUS:
		return "knife_ursus";
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
		return "knife_gypsy_jackknife";
	case WEAPON_KNIFE_STILETTO:
		return "knife_stiletto";
	case WEAPON_KNIFE_WIDOWMAKER:
		return "knife_widowmaker";
	case WEAPON_KNIFE_SKELETON:
		return "knife_skeleton";
	case WEAPON_KNIFE_OUTDOOR:
		return "knife_outdoor";
	case WEAPON_KNIFE_CANIS:
		return "knife_canis";
	case WEAPON_KNIFE_CORD:
		return "knife_cord";
	case WEAPON_KNIFE_CSS:
		return "knife_css";
	case GLOVE_STUDDED_BLOODHOUND:
		return "studded_bloodhound_gloves";
	case GLOVE_T_SIDE:
		return "t_gloves";
	case GLOVE_CT_SIDE:
		return "ct_gloves";
	case GLOVE_SPORTY:
		return "sporty_gloves";
	case GLOVE_SLICK:
		return "slick_gloves";
	case GLOVE_LEATHER_WRAP:
		return "leather_handwraps";
	case GLOVE_MOTORCYCLE:
		return "motorcycle_gloves";
	case GLOVE_SPECIALIST:
		return "specialist_gloves";
	case GLOVE_HYDRA:
		return "studded_hydra_gloves";
	case GLOVE_BROKENFRAG:
		return "studded_brokenfang_gloves";
	default:
		return "";
	}
}

static const char* normal_weapon_names(const short id)
{
	switch (id)
	{
	case WEAPON_DEAGLE:
		return "Deagle";
	case WEAPON_ELITE:
		return "Elite";
	case WEAPON_FIVESEVEN:
		return "Fiveseven";
	case WEAPON_GLOCK:
		return "Glock";
	case WEAPON_AK47:
		return "AK47";
	case WEAPON_AUG:
		return "Aug";
	case WEAPON_AWP:
		return "AWP";
	case WEAPON_FAMAS:
		return "Famas";
	case WEAPON_G3SG1:
		return "G3SG1";
	case WEAPON_GALILAR:
		return "Galilar";
	case WEAPON_M249:
		return "M249";
	case WEAPON_M4A1_SILENCER:
		return "M4a4s";
	case WEAPON_M4A1:
		return "M4a1";
	case WEAPON_MAC10:
		return "Mac10";
	case WEAPON_P90:
		return "P90";
	case WEAPON_UMP45:
		return "Ump45";
	case WEAPON_XM1014:
		return "Xm1014";
	case WEAPON_BIZON:
		return "Bizon";
	case WEAPON_MAG7:
		return "Mag7";
	case WEAPON_NEGEV:
		return "Negev";
	case WEAPON_SAWEDOFF:
		return "Sawedoff";
	case WEAPON_TEC9:
		return "Tec9";
	case WEAPON_HKP2000:
		return "P2000";
	case WEAPON_MP5SD:
		return "Mp5sd";
	case WEAPON_MP7:
		return "Mp7";
	case WEAPON_MP9:
		return "Mp9";
	case WEAPON_NOVA:
		return "Nova";
	case WEAPON_P250:
		return "P250";
	case WEAPON_SCAR20:
		return "Scar20";
	case WEAPON_SG556:
		return "SG556";
	case WEAPON_SSG08:
		return "SSG08";
	case WEAPON_USP_SILENCER:
		return "Usps";
	case WEAPON_CZ75A:
		return "CZ75a";
	case WEAPON_REVOLVER:
		return "Revolver";
	case WEAPON_KNIFE:
		return "Knife";
	case WEAPON_KNIFE_T:
		return "Knife t";
	case WEAPON_KNIFE_M9_BAYONET:
		return "M9 bayonet knife";
	case WEAPON_KNIFE_BAYONET:
		return "Bayonet knife";
	case WEAPON_KNIFE_FLIP:
		return "Flip knife";
	case WEAPON_KNIFE_GUT:
		return "Gut knife";
	case WEAPON_KNIFE_KARAMBIT:
		return "Karambit knife";
	case WEAPON_KNIFE_TACTICAL:
		return "Tactical knife";
	case WEAPON_KNIFE_FALCHION:
		return "Falchion knife";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "Bowie knife";
	case WEAPON_KNIFE_BUTTERFLY:
		return "Butterfly knife";
	case WEAPON_KNIFE_PUSH:
		return "Push knife";
	case WEAPON_KNIFE_URSUS:
		return "Ursus knife";
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
		return "Navaja knife";
	case WEAPON_KNIFE_STILETTO:
		return "Stiletto knife";
	case WEAPON_KNIFE_WIDOWMAKER:
		return "Widowmaker knife";
	case WEAPON_KNIFE_SKELETON:
		return "Skeleton knife";
	case WEAPON_KNIFE_OUTDOOR:
		return "Outdoor knife";
	case WEAPON_KNIFE_CANIS:
		return "Canis knife";
	case WEAPON_KNIFE_CORD:
		return "Cord knife";
	case WEAPON_KNIFE_CSS:
		return "Css knife";
	case GLOVE_STUDDED_BLOODHOUND:
		return "Bloodhound gloves";
	case GLOVE_T_SIDE:
		return "T gloves";
	case GLOVE_CT_SIDE:
		return "CT gloves";
	case GLOVE_SPORTY:
		return "Sporty gloves";
	case GLOVE_SLICK:
		return "Slick gloves";
	case GLOVE_LEATHER_WRAP:
		return "Leather handwraps";
	case GLOVE_MOTORCYCLE:
		return "Motorcycle gloves";
	case GLOVE_SPECIALIST: return "specialist gloves";
	case GLOVE_HYDRA: return "hydra gloves";
	case GLOVE_BROKENFRAG: return "broken fang gloves";
	case 5105: return "Ground Rebel";
	case 5106: return "Osiris";
	case 5107: return "Prof. Shahmat";
	case 5108: return "The Elite Mr. Muhlik";
	case 5205: return "Soldier Phoenix";
	case 5206: return "Enforcer Phoenix";
	case 5207: return "Slingshot";
	case 5208: return "Street Soldier";
	case 5305: return "Operator FBI";
	case 5306: return "Markus Delrow";
	case 5307: return "Michael Syfers";
	case 5308: return "Special Agent Ava";
	case 5400: return "3rd Commando Company";
	case 5401: return "Seal Team 6 Soldier";
	case 5402: return "BuckShot";
	case 5403: return "`Two Times` McCoy USAF";
	case 5404: return "Lt. Commander Ricksaw";
	case 4619: return "`Blueberries Buckshot`";
	case 4680: return "`Two Times` McCoy TACP";
	case 5500: return "Dragomir";
	case 5501: return "Maximus";
	case 5502: return "Rezan The Ready";
	case 5503: return "Blackwolf";
	case 5504: return "`The Doctor` Romanov";
	case 4718: return "Rezan the Redshirt";
	case 5505: return "Dragomir Sabre";
	case 5601: return "B Squadron Officer";
	case 4711: return "Cmdr. Mae `Dead Cold` Jamison";
	case 4712: return "1st Lieutenant Farlow";
	case 4713: return "John `Van Healen` Kask";
	case 4714: return "Bio-Haz Specialist";
	case 4715: return "Sergeant Bombson";
	case 4716: return "Chem-Haz Specialist";
	case 4726: return "Sir Bloody Miami Darryl";
	case 4733: return "Sir Bloody Silent Darryl";
	case 4734: return "Sir Bloody Skullhead Darryl";
	case 4735: return "Sir Bloody Darryl Royale";
	case 4736: return "Sir Bloody Loudmouth Darryl";
	case 4727: return "SafeCracker Voltzmann";
	case 4728: return "Little Kev";
	case 4732: return "Number K";
	case 4730: return "Getaway Sally";
	default:
		return "unknown";
	}
}

#pragma endregion

#pragma region "Key"
inline std::string get_kname(int key) {
	switch (key) {
	case VK_LBUTTON:
		return "mouse1";
	case VK_RBUTTON:
		return "mouse2";
	case VK_CANCEL:
		return "break";
	case VK_MBUTTON:
		return "mouse3";
	case VK_XBUTTON1:
		return "mouse4";
	case VK_XBUTTON2:
		return "mouse5";
	case VK_BACK:
		return "backspace";
	case VK_TAB:
		return "tab";
	case VK_CLEAR:
		return "clear";
	case VK_RETURN:
		return "enter";
	case VK_SHIFT:
		return "shift";
	case VK_CONTROL:
		return "ctrl";
	case VK_MENU:
		return "alt";
	case VK_PAUSE:
		return "[19]";
	case VK_CAPITAL:
		return "capslock";
	case VK_SPACE:
		return "space";
	case VK_PRIOR:
		return "pgup";
	case VK_NEXT:
		return "pgdown";
	case VK_END:
		return "end";
	case VK_HOME:
		return "home";
	case VK_LEFT:
		return "left";
	case VK_UP:
		return "up";
	case VK_RIGHT:
		return "right";
	case VK_DOWN:
		return "down";
	case VK_SELECT:
		return "select";
	case VK_INSERT:
		return "insert";
	case VK_DELETE:
		return "delete";
	case '0':
		return "0";
	case '1':
		return "1";
	case '2':
		return "2";
	case '3':
		return "3";
	case '4':
		return "4";
	case '5':
		return "5";
	case '6':
		return "6";
	case '7':
		return "7";
	case '8':
		return "8";
	case '9':
		return "9";
	case 'A':
		return "a";
	case 'B':
		return "b";
	case 'C':
		return "c";
	case 'D':
		return "d";
	case 'E':
		return "e";
	case 'F':
		return "f";
	case 'G':
		return "g";
	case 'H':
		return "h";
	case 'I':
		return "i";
	case 'J':
		return "j";
	case 'K':
		return "k";
	case 'L':
		return "l";
	case 'M':
		return "m";
	case 'N':
		return "n";
	case 'O':
		return "o";
	case 'P':
		return "p";
	case 'Q':
		return "q";
	case 'R':
		return "r";
	case 'S':
		return "s";
	case 'T':
		return "t";
	case 'U':
		return "u";
	case 'V':
		return "v";
	case 'W':
		return "w";
	case 'X':
		return "x";
	case 'Y':
		return "y";
	case 'Z':
		return "z";
	case VK_LWIN:
		return "left win";
	case VK_RWIN:
		return "right win";
	case VK_NUMPAD0:
		return "num 0";
	case VK_NUMPAD1:
		return "num 1";
	case VK_NUMPAD2:
		return "num 2";
	case VK_NUMPAD3:
		return "num 3";
	case VK_NUMPAD4:
		return "num 4";
	case VK_NUMPAD5:
		return "num 5";
	case VK_NUMPAD6:
		return "num 6";
	case VK_NUMPAD7:
		return "num 7";
	case VK_NUMPAD8:
		return "num 8";
	case VK_NUMPAD9:
		return "num 9";
	case VK_MULTIPLY:
		return "num mult";
	case VK_ADD:
		return "num add";
	case VK_SEPARATOR:
		return "|";
	case VK_SUBTRACT:
		return "num sub";
	case VK_DECIMAL:
		return "num decimal";
	case VK_DIVIDE:
		return "num divide";
	case VK_F1:
		return "f1";
	case VK_F2:
		return "f2";
	case VK_F3:
		return "f3";
	case VK_F4:
		return "f4";
	case VK_F5:
		return "f5";
	case VK_F6:
		return "f6";
	case VK_F7:
		return "f7";
	case VK_F8:
		return "f8";
	case VK_F9:
		return "f9";
	case VK_F10:
		return "f10";
	case VK_F11:
		return "f11";
	case VK_F12:
		return "f12";
	case VK_NUMLOCK:
		return "num lock";
	case VK_SCROLL:
		return "break";
	case VK_LSHIFT:
		return "shift";
	case VK_RSHIFT:
		return "shift";
	case VK_LCONTROL:
		return "ctrl";
	case VK_RCONTROL:
		return "ctrl";
	case VK_LMENU:
		return "alt";
	case VK_RMENU:
		return "alt";
	case VK_OEM_COMMA:
		return "),";
	case VK_OEM_PERIOD:
		return ".";
	case VK_OEM_1:
		return ";";
	case VK_OEM_MINUS:
		return "-";
	case VK_OEM_PLUS:
		return "=";
	case VK_OEM_2:
		return "/";
	case VK_OEM_3:
		return "grave";
	case VK_OEM_4:
		return "[";
	case VK_OEM_5:
		return "\\";
	case VK_OEM_6:
		return "]";
	case VK_OEM_7:
		return "[222]";
	default:
		return "-";
	}
}
#pragma endregion


std::vector <std::string> files;
namespace menu
{
	static wskin weaponSkin;
	static int index = 0;
	IDirect3DTexture9* m_pTexture;
	void init(const float& alpha)
	{
		auto style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		ImGui::StyleColorsDark(style);

		/*style->AntiAliasedLines = true;
		style->AntiAliasedFill = true;
		style->ScrollbarRounding = 7.f;
		style->ScrollbarSize = 6.f;
		style->WindowRounding = 0.0f;
		style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style->FramePadding = ImVec2(0, 10);
		style->WindowPadding = ImVec2(5, 10);
		style->FrameRounding = 0.f;
		style->ItemSpacing = ImVec2(0, 5);
		style->WindowRounding = 10.f;
		style->ChildBorderSize = 5.f;*/
		//ui::GetIO().Fonts->AddFontDefault();

		/*switch (ctx.m_settings.settings_dpi_scale)
		{
		case 0:
			ImGui::GetIO().FontGlobalScale = 1;
			break;
		case 1:
			ImGui::GetIO().FontGlobalScale = 1.25f;
			break;
		case 2:
			ImGui::GetIO().FontGlobalScale = 1.5f;
			break;
		case 3:
			ImGui::GetIO().FontGlobalScale = 1.75f;
			break;
		case 4:
			ImGui::GetIO().FontGlobalScale = 2.f;
			break;
		}*/
	}
	auto selected_script = 0;

	void skins_listbox(const char* label, int height_in_items)
	{
		if (!ImGui::ListBoxHeader(label, 15, height_in_items))
			return;

		for (const auto& weapon : k_inventory_names)
		{
			if (ImGui::Selectable(weapon.second, weaponSkin.wId == weapon.first, 0))
			{
				weaponSkin.wId = weapon.first;
				weaponSkin.paintKit = 0;
				//skinImage = nullptr;
			}
		}
		ImGui::ListBoxFooter();
	}
	void load_config()
	{
		if (cfg_manager->files.empty())
			return;

		cfg_manager->load(cfg_manager->files.at(ctx.m_settings.selected_config), false);

		for (auto& script : ctx.m_settings.scripts)
			c_lua::get().load_script(c_lua::get().get_script_id(script));

		if (selected_script >= c_lua::get().scripts.size())
			selected_script = c_lua::get().scripts.size() - 1; //-V103

		ctx.m_settings.scripts.clear();

		cfg_manager->load(cfg_manager->files.at(ctx.m_settings.selected_config), true);
		cfg_manager->config_files();

		//eventlogs::get().add(crypt_str("Loaded ") + files.at(g_cfg.selected_config) + crypt_str(" config"), false);
	}
	void save_config()
	{
		if (cfg_manager->files.empty())
			return;

		ctx.m_settings.scripts.clear();

		for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
		{
			auto script = c_lua::get().scripts.at(i);

			if (c_lua::get().loaded.at(i))
				ctx.m_settings.scripts.emplace_back(script);
		}

		cfg_manager->save(cfg_manager->files.at(ctx.m_settings.selected_config));
		cfg_manager->config_files();
	}
	void add_config()
	{
		auto empty = true;

		for (auto current : ctx.m_settings.new_config_name)
		{
			if (current != ' ')
			{
				empty = false;
				break;
			}
		}

		if (empty)
			ctx.m_settings.new_config_name = sxor("new config");

		//eventlogs::get().add(crypt_str("Added ") + ctx.m_settings.new_config_name + crypt_str(" config"), false);

		if (ctx.m_settings.new_config_name.find(sxor(".gay")) == std::string::npos)
			ctx.m_settings.new_config_name += sxor(".gay");

		cfg_manager->save(ctx.m_settings.new_config_name);
		cfg_manager->config_files();

		cfg_manager->files = cfg_manager->files;
		ctx.m_settings.selected_config = cfg_manager->files.size() - 1; //-V103
	}
	void draw_combo_lua(const char* name, int& variable, bool (*items_getter)(void*, int, const char**), void* data, int count)
	{
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 9);
		ImGui::Text(name);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
		ImGui::Combo(std::string(("##COMBO__") + std::string(name)).c_str(), &variable, items_getter, data, count);
	}

	void draw(IDirect3DDevice9* device)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos.x = feature::menu->_cursor_position.x;
		io.MousePos.y = feature::menu->_cursor_position.y;

		/*if (!feature::menu->_menu_opened && ui::GetStyle().Alpha > 0.f) {
			float fc = 255.f / 0.2f * ui::GetIO().DeltaTime;
			ui::GetStyle().Alpha = std::clamp(ui::GetStyle().Alpha - fc / 255.f, 0.f, 1.f);
		}

		if (feature::menu->_menu_opened && ui::GetStyle().Alpha < 1.f) {
			float fc = 255.f / 0.2f * ui::GetIO().DeltaTime;
			ui::GetStyle().Alpha = std::clamp(ui::GetStyle().Alpha + fc / 255.f, 0.f, 1.f);
		}*/

		if (!feature::menu->_menu_opened) // && ui::GetStyle().Alpha < 0.1f)
			return;

		init(1.f);

		//ImGui::ShowDemoWindow(0);
		
		ImGui::SetNextWindowSizeConstraints(ImVec2(670, 400), ImVec2(850, 650));
		ImGui::Begin(sxor("imgui menu"), 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
		auto cpos = ImGui::GetCursorPos();
		auto wp = ImGui::GetWindowPos();
		static int tab = 0;

		
		ImGui::TabButton(sxor("legit"), &tab, 0, 9);
		ImGui::TabButton(sxor("rage"), &tab, 1, 9);
		ImGui::TabButton(sxor("anti-aim"), &tab, 2, 9);
		ImGui::TabButton(sxor("player"), &tab, 3, 9);
		ImGui::TabButton(sxor("world"), &tab, 4, 9);
		ImGui::TabButton(sxor("misc"), &tab, 5, 9);
		ImGui::TabButton(sxor("skins"), &tab, 6, 9);
		ImGui::TabButton(sxor("config"), &tab, 7, 9);
		ImGui::TabButton(sxor("lua"), &tab, 8, 9);

		int max_size_y = ImGui::GetWindowSize().y - 40;
		int max_size_x2 = (ImGui::GetWindowSize().x - 110) / 2 - 10;
		int max_size_x = ImGui::GetWindowSize().x - 120;
		int max_size_y2 = (ImGui::GetWindowSize().y - 40) / 2 - 5;

		if (tab == 0)
		{
			static int currentCategory{ 0 };
			static int currentWeapon{ 0 };
			//currentWeapon = 26;
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("aimbot", ImVec2(max_size_x2, max_size_y));
			ImGui::Checkbox("Enabled", &ctx.m_settings.aimbot[currentWeapon].enabled);
			ImGui::Checkbox("Silent", &ctx.m_settings.aimbot[currentWeapon].silent);
			ImGui::Checkbox("Auto Delay", &ctx.m_settings.aimbot[currentWeapon].autodelay);
			ImGui::Checkbox("Friendly fire", &ctx.m_settings.aimbot[currentWeapon].friendlyFire);
			ImGui::Checkbox("Visible only", &ctx.m_settings.aimbot[currentWeapon].visibleOnly);
			ImGui::Checkbox("Scoped only", &ctx.m_settings.aimbot[currentWeapon].scopedOnly);
			ImGui::Checkbox("Ignore flash", &ctx.m_settings.aimbot[currentWeapon].ignoreFlash);
			ImGui::Checkbox("Ignore smoke", &ctx.m_settings.aimbot[currentWeapon].ignoreSmoke);
			ImGui::Combo("Priority", &ctx.m_settings.aimbot[currentWeapon].bone, "Nearest\0Best damage\0Head\0Neck\0Sternum\0Chest\0Stomach\0Pelvis\0");

			ImGui::SliderFloat("Fov", &ctx.m_settings.aimbot[currentWeapon].fov, 0.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Smooth", &ctx.m_settings.aimbot[currentWeapon].smooth, 1.0f, 10.0f, "%.2f");
			//ImGui::SliderFloat("Rcs Fov", &ctx.m_settings.aimbot[currentWeapon].rcsfov, 1.0f, 100.0f, "%.2f");
			ImGui::SliderFloat("Rcs Strange", &ctx.m_settings.aimbot[currentWeapon].rcsfov, 1.0f, 100.0f, "%.2f");
			ImGui::SliderFloat("Silent Fov", &ctx.m_settings.aimbot[currentWeapon].silentfov, 1.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Kill Delay", &ctx.m_settings.aimbot[currentWeapon].killdelay, 0.0f, 5.0f, "%.2f");
			ImGui::EndChild();

			ImGui::SetNextWindowPos(ImVec2(wp.x + 110 + max_size_x2 + 10, wp.y + 30));
			ImGui::BeginChild("weapon", ImVec2(max_size_x2, max_size_y));

			ImGui::SingleSelect("Weapon settings", &currentWeapon, {
				sxor("none"),
				sxor("glock"),
				sxor("p2000"),
				sxor("usp-s"),
				sxor("elite"),
				sxor("p250"),
				sxor("tec9"),
				sxor("five seven"),
				sxor("cz75a"),
				sxor("deagle"),
				sxor("revolver"),
				sxor("nova"),
				sxor("xm1014"),
				sxor("sawedoff"),
				sxor("mag7"),
				sxor("m249"),
				sxor("negev"),
				sxor("mac10"),
				sxor("mp9"),
				sxor("mp7"),
				sxor("mp5sd"),
				sxor("ump45"),
				sxor("p90"),
				sxor("bizon"),
				sxor("galilar"),
				sxor("famas"),
				sxor("ak47"),
				sxor("m4a4"),
				sxor("m4a4s"),
				sxor("ssg08"),
				sxor("sg556"),
				sxor("aug"),
				sxor("awp"),
				sxor("g3sg1"),
				sxor("scar20"),
				sxor("taser")
				});


			ImGui::EndChild();
		}
		if (tab == 1)
		{
			static int settings = 0;
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("Aimbot", ImVec2(max_size_x2, max_size_y));
			ImGui::Checkbox(sxor("Enabled"), &ctx.m_settings.aimbot_enabled);

			ImGui::SingleSelect(sxor("Target selection"), &ctx.m_settings.aimbot_target_selection, { "Cycle","Cycle (2x)","Near crosshair","Highest damage","Lowest ping","Best K/D ration","Best hit chance" });

			ImGui::MultiCombo(sxor("Target hitbox"), ctx.m_settings.aimbot_weapon[settings].aimbot_target_hitbox, hitboxes, ARRAYSIZE(hitboxes), preview);
			ImGui::MultiCombo(sxor("Multi-point"), ctx.m_settings.aimbot_weapon[settings].aimbot_multi_point, hitboxes, ARRAYSIZE(hitboxes), preview);
			ImGui::SingleSelect("Multi-point strange", &ctx.m_settings.aimbot_weapon[settings].aimbot_multi_point_strange, { "Low", "Medium", "High" });

			ImGui::SliderInt(sxor("Mutlti-point scale"), &ctx.m_settings.aimbot_weapon[settings].aimbot_multi_point_scale, 0, 100, "%d%");

			ImGui::Checkbox(sxor("Prefer safe point"), &ctx.m_settings.aimbot_weapon[settings].aimbot_prefer_safe_point);

			ImGui::Text(sxor("Force safe point"));
			ImGui::Keybind(sxor("##force_safe_point"), &ctx.m_settings.aimbot_weapon[settings].aimbot_force_safe_point_key.key, &ctx.m_settings.aimbot_weapon[settings].aimbot_force_safe_point_key.mode);

			ImGui::Text(sxor("Force min damage"));
			ImGui::Keybind(sxor("##force_min_damage"), &ctx.m_settings.aimbot_weapon[settings].aimbot_min_damage_override.key, &ctx.m_settings.aimbot_weapon[settings].aimbot_min_damage_override.mode);

			ImGui::Checkbox(sxor("Force safe point on limbs"), &ctx.m_settings.aimbot_weapon[settings].aimbot_force_safe_point_on_limbs);
			ImGui::Checkbox(sxor("Silent aim"), &ctx.m_settings.aimbot_weapon[settings].aimbot_silent_aim);
			ImGui::Checkbox(sxor("Automatic fire"), &ctx.m_settings.aimbot_weapon[settings].aimbot_automatic_fire);
			ImGui::Checkbox(sxor("Automatic penetration"), &ctx.m_settings.aimbot_weapon[settings].aimbot_automatic_penetration);

			ImGui::SliderInt(sxor("Minimum hit chance"), &ctx.m_settings.aimbot_weapon[settings].aimbot_minimum_hit_chance, 0, 100, "%d%");

			ImGui::SliderInt(sxor("Minimum damage"), &ctx.m_settings.aimbot_weapon[settings].aimbot_minimum_damage, 0, 120, ctx.m_settings.aimbot_weapon[settings].aimbot_minimum_damage < 101 ? "%d" : "HP +%d");
			ImGui::SliderInt(sxor("Minimum autowall damage"), &ctx.m_settings.aimbot_weapon[settings].aimbot_minimum_autowall_damage, 0, 120, ctx.m_settings.aimbot_weapon[settings].aimbot_minimum_autowall_damage < 101 ? "%d" : "HP +%d");
			ImGui::SliderInt(sxor("Minimum damage override"), &ctx.m_settings.aimbot_weapon[settings].aimbot_minimum_override_damage, 0, 120, ctx.m_settings.aimbot_weapon[settings].aimbot_minimum_override_damage < 101 ? "%d" : "HP +%d");

			ImGui::Checkbox(sxor("Automatic scope"), &ctx.m_settings.aimbot_weapon[settings].aimbot_automatic_scope);
			ImGui::SliderInt(sxor("Maximum FOV"), &ctx.m_settings.aimbot_weapon[settings].aimbot_maximum_fov, 0, 180, "%d");

			ImGui::Checkbox(sxor("Missed shots log"), &ctx.m_settings.aimbot_weapon[settings].aimbot_log_misses_due_to_spread);

			ImGui::MultiCombo(sxor("Low FPS mitigations"), ctx.m_settings.aimbot_weapon[settings].aimbot_low_fps_mitigations, low_fps_mig, ARRAYSIZE(low_fps_mig), preview);
			ImGui::EndChild();

			ImGui::SetNextWindowPos(ImVec2(wp.x + 110 + max_size_x2 + 10, wp.y + 30));
			ImGui::BeginChild("Other", ImVec2(max_size_x2, max_size_y));
			ImGui::Checkbox(sxor("Remove recoil"), &ctx.m_settings.aimbot_weapon[settings].aimbot_remove_recoil);

			ImGui::SingleSelect(sxor("Backtrack amount"), &ctx.m_settings.aimbot_weapon[settings].aimbot_accuracy_boost, { "Off", "Low", "Medium", "High", "Maximum" });

			ImGui::Checkbox(sxor("Delay shot"), &ctx.m_settings.aimbot_weapon[settings].aimbot_delay_shot);
			ImGui::Checkbox(sxor("Quick stop"), &ctx.m_settings.aimbot_weapon[settings].aimbot_quick_stop);
			ImGui::MultiCombo(sxor("Quick stop options"), ctx.m_settings.aimbot_weapon[settings].aimbot_quick_stop_options, quick_stop, ARRAYSIZE(quick_stop), preview);

			ImGui::Checkbox(sxor("Anti-aim correction"), &ctx.m_settings.aimbot_weapon[settings].aimbot_anti_aim_corrections);

			ImGui::Checkbox(sxor("Double tap"), &ctx.m_settings.aimbot_weapon[settings].aimbot_double_tap);
			ImGui::Keybind(sxor("##doubletap"), &ctx.m_settings.aimbot_double_tap_key.key, &ctx.m_settings.aimbot_double_tap_key.mode);

			ImGui::SingleSelect(sxor("Double tap mode"), &ctx.m_settings.aimbot_weapon[settings].aimbot_double_tap_mode, { "Defensive", "Offensive" });



			ImGui::SingleSelect(sxor("Global settings"), &settings, { "Auto sniper", "AWP", "Scout", "Rifles", "Pistols", "Heavy pistols", "Taser", "Other" });

			ImGui::EndChild();
		}
		if (tab == 2)
		{
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("Anti-aimbot angles", ImVec2(max_size_x2, max_size_y));
			ImGui::Checkbox(sxor("Enabled"), &ctx.m_settings.anti_aim_enabled);
			ImGui::SingleSelect(sxor("Pitch"), &ctx.m_settings.anti_aim_pitch, { "Off", "Default", "Up", "Down", "Minimal", "Random" });

			ImGui::SingleSelect(sxor("Yaw base"), &ctx.m_settings.anti_aim_yaw_base, { "Local view", "At targets" });

			ImGui::SingleSelect(sxor("Yaw"), &ctx.m_settings.anti_aim_yaw_type, { "Off", "180", "Spin", "Static", "180 Z", "Crosshair" });

			ImGui::SliderInt(sxor("Yaw add"), &ctx.m_settings.anti_aim_yaw_custom, -180, 180);

			ImGui::SingleSelect(sxor("Yaw jitter"), &ctx.m_settings.anti_aim_yaw_jitter_type, { "Off", "Offset", "Centre", "Random" });
			ImGui::SliderInt(sxor("Yaw jitter offset"), &ctx.m_settings.anti_aim_yaw_jitter_custom, -180, 180);

			ImGui::SingleSelect(sxor("Body yaw"), &ctx.m_settings.anti_aim_body_yaw, { "Off", "Opposite", "Jitter", "Static" });
			if (ctx.m_settings.anti_aim_body_yaw > 0)
				ImGui::Checkbox(sxor("Freestanding body yaw"), &ctx.m_settings.anti_aim_freestanding_body_yaw);

			ImGui::SingleSelect(sxor("Lower body yaw target"), &ctx.m_settings.anti_aim_lby_yaw_target, { "Off", "Sway", "Opposite", "Eye yaw" });
			ImGui::SliderInt(sxor("Fake yaw limit"), &ctx.m_settings.anti_aim_fake_limit, -60, 60, "%d", true);
			ImGui::Keybind(sxor("##anti_aim_fake_switch"), &ctx.m_settings.anti_aim_fake_switch_key.key, &ctx.m_settings.anti_aim_fake_switch_key.mode);

			ImGui::Checkbox(sxor("Edge yaw"), &ctx.m_settings.anti_aim_edge_yaw);

			ImGui::SingleSelect(sxor("Freestanding"), &ctx.m_settings.anti_aim_freestanding_fake_type, { "Off", "Default" });
			ImGui::Keybind(sxor("##Freestandingkey"), &ctx.m_settings.anti_aim_freestand_key.key, &ctx.m_settings.anti_aim_freestand_key.mode);

			ImGui::Text(sxor("Right yaw"));
			ImGui::Keybind(sxor("##anti_aim_yaw_right_switch_key"), &ctx.m_settings.anti_aim_yaw_right_switch_key.key, &ctx.m_settings.anti_aim_yaw_right_switch_key.mode);

			ImGui::Text(sxor("Left yaw"));
			ImGui::Keybind(sxor("##anti_aim_yaw_left_switch_key"), &ctx.m_settings.anti_aim_yaw_left_switch_key.key, &ctx.m_settings.anti_aim_yaw_left_switch_key.mode);

			ImGui::Text(sxor("Back yaw"));
			ImGui::Keybind(sxor("##anti_aim_yaw_backward_switch_key"), &ctx.m_settings.anti_aim_yaw_backward_switch_key.key, &ctx.m_settings.anti_aim_yaw_backward_switch_key.mode);


			ImGui::EndChild();

			ImGui::SetNextWindowPos(ImVec2(wp.x + 110 + max_size_x2 + 10, wp.y + 30));
			ImGui::BeginChild("Fake lag", ImVec2(max_size_x2, max_size_y2));
			ImGui::Checkbox(sxor("Enabled"), &ctx.m_settings.fake_lag_enabled);
			ImGui::SingleSelect(sxor("Amount"), &ctx.m_settings.fake_lag_type, { "factor", "break" });
			ImGui::SliderFloat(sxor("Variance"), &ctx.m_settings.fake_lag_variance, 0.f, 100.f);
			ImGui::SliderInt(sxor("Limit"), &ctx.m_settings.fake_lag_value, 0, 14);
			ImGui::Checkbox(sxor("Fakelag while shooting"), &ctx.m_settings.fake_lag_shooting);
			ImGui::EndChild();

			ImGui::SetNextWindowPos(ImVec2(wp.x + 110 + max_size_x2 + 10, wp.y + 30 + max_size_y2 + 10));
			ImGui::BeginChild("Other", ImVec2(max_size_x2, max_size_y2));
			ImGui::Text(sxor("Automatic peek"));
			ImGui::Keybind(sxor("##apkey"), &ctx.m_settings.anti_aim_autopeek_key.key, &ctx.m_settings.anti_aim_autopeek_key.mode);
			ImGui::Text(sxor("Fake duck"));
			ImGui::Keybind(sxor("##fdkey"), &ctx.m_settings.anti_aim_fakeduck_key.key, &ctx.m_settings.anti_aim_fakeduck_key.mode);

			ImGui::SingleSelect(sxor("Leg movement"), &ctx.m_settings.anti_aim_leg_movement, { "Off", "Always slide", "Never slide" });

			ImGui::Text(sxor("On shot aa"));
			ImGui::Keybind(sxor("##aimbot_hideshots_exploit_toggle"), &ctx.m_settings.aimbot_hideshots_exploit_toggle.key, &ctx.m_settings.aimbot_hideshots_exploit_toggle.mode);


			ImGui::EndChild();
		}
		if (tab == 3)
		{
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("Player ESP", ImVec2(max_size_x2, max_size_y));
			ImGui::Checkbox(sxor("Dormant"), &ctx.m_settings.player_esp_dormant);
			ImGui::Checkbox(sxor("Bounding box"), &ctx.m_settings.player_esp_bounding_box);
			ImGui::ColorEdit4(sxor("#bounding_box_color"), ctx.m_settings.player_esp_bounding_box_color);
			ImGui::Checkbox(sxor("Health bar"), &ctx.m_settings.player_esp_health_bar);
			ImGui::Checkbox(sxor("Name"), &ctx.m_settings.player_esp_name);
			ImGui::ColorEdit4(sxor("#name_color"), ctx.m_settings.player_esp_name_color);
			ImGui::Checkbox(sxor("Flags"), &ctx.m_settings.player_esp_flags);
			ImGui::Checkbox(sxor("Weapon text"), &ctx.m_settings.player_esp_weapon_text);
			ImGui::Checkbox(sxor("Weapon icon"), &ctx.m_settings.player_esp_weapon_icon);
			ImGui::ColorEdit4(sxor("#Weapon_color"), ctx.m_settings.player_esp_weapon_color);
			ImGui::Checkbox(sxor("Ammo"), &ctx.m_settings.player_esp_ammo);
			ImGui::ColorEdit4(sxor("#Ammo_color"), ctx.m_settings.player_esp_ammo_color);
			ImGui::Checkbox(sxor("Distance"), &ctx.m_settings.player_esp_distance);
			//ImGui::Checkbox(sxor("Glow"), &ctx.m_settings.player_esp_glow);

			if (ImGui::BeginCombo(sxor("Glow"), "Glow type", 0, 8)) {

				ImGui::Checkbox(sxor("Glow enemy"), &ctx.m_settings.glow[0].enabled);
				ImGui::ColorEdit4(sxor("#player_esp_glow_enemy_color"), ctx.m_settings.glow[0].color);
				ImGui::Checkbox(sxor("Glow enemy m_bFullBloomRender"), &ctx.m_settings.glow[0].m_bFullBloomRender);
				ImGui::Checkbox(sxor("Glow enemy m_bRenderWhenOccluded"), &ctx.m_settings.glow[0].m_bRenderWhenOccluded);
				ImGui::Checkbox(sxor("Glow enemy m_bRenderWhenUnoccluded"), &ctx.m_settings.glow[0].m_bRenderWhenUnoccluded);
				ImGui::Checkbox(sxor("Glow local"), &ctx.m_settings.glow[1].enabled);
				ImGui::ColorEdit4(sxor("#player_esp_glow_local_color"), ctx.m_settings.glow[1].color);
				ImGui::Checkbox(sxor("Glow local m_bFullBloomRender"), &ctx.m_settings.glow[1].m_bFullBloomRender);
				ImGui::Checkbox(sxor("Glow local m_bRenderWhenOccluded"), &ctx.m_settings.glow[1].m_bRenderWhenOccluded);
				ImGui::Checkbox(sxor("Glow local m_bRenderWhenUnoccluded"), &ctx.m_settings.glow[1].m_bRenderWhenUnoccluded);

				ImGui::EndCombo();
			}

			//ImGui::ColorEdit4(sxor("#player_esp_glow_color"), ctx.m_settings.player_esp_glow_color);
			ImGui::Checkbox(sxor("Hit marker"), &ctx.m_settings.player_esp_hit_marker);
			ImGui::Checkbox(sxor("Hit marker sound"), &ctx.m_settings.player_esp_hit_marker_sound);
			ImGui::Checkbox(sxor("Money"), &ctx.m_settings.player_esp_money);
			ImGui::Checkbox(sxor("Skeleton"), &ctx.m_settings.player_esp_skeleton);
			ImGui::ColorEdit4(sxor("#player_esp_skeleton_color"), ctx.m_settings.player_esp_skeleton_color);
			ImGui::Checkbox(sxor("Out of FOV arrow"), &ctx.m_settings.player_esp_out_of_fov_arrow);
			ImGui::ColorEdit4(sxor("#player_esp_out_of_fov_arrow_color"), ctx.m_settings.player_esp_out_of_fov_arrow_color);
			ImGui::SliderInt(sxor("OOF arrow size"), &ctx.m_settings.player_esp_out_of_fov_arrow_size, 0, 32, "%dpx");
			ImGui::SliderInt(sxor("OOF arrow distance"), &ctx.m_settings.player_esp_out_of_fov_arrow_distance, 0, 100, "%d%");

			ImGui::EndChild();

			ImGui::SetNextWindowPos(ImVec2(wp.x + 110 + max_size_x2 + 10, wp.y + 30));
			ImGui::BeginChild("Colored models", ImVec2(max_size_x2, max_size_y));

			ImGui::Checkbox(sxor("Player"), &ctx.m_settings.colored_models_player);
			ImGui::ColorEdit4(sxor("##colored_models_player_color"), ctx.m_settings.colored_models_player_color);
			ImGui::Checkbox(sxor("Player behind wall"), &ctx.m_settings.colored_models_player_behind_wall);
			ImGui::ColorEdit4(sxor("##colored_models_player_behind_wall_color"), ctx.m_settings.colored_models_player_behind_wall_color);

			ImGui::SingleSelect(sxor("Player models type"), &ctx.m_settings.colored_models_player_type, { "Default", "Solid", "Shaded", "Metallic", "Glow", "Bubble" });

			ImGui::Checkbox(sxor("Teammate"), &ctx.m_settings.colored_models_teammate);
			ImGui::ColorEdit4(sxor("##colored_models_teammate_color"), ctx.m_settings.colored_models_teammate_color);
			ImGui::Checkbox(sxor("Teammate behind wall"), &ctx.m_settings.colored_models_teammate_behind_wall);
			ImGui::ColorEdit4(sxor("##colored_models_teammate_behind_wall_color"), ctx.m_settings.colored_models_teammate_behind_wall_color);

			ImGui::SingleSelect(sxor("Teammate models type"), &ctx.m_settings.colored_models_teammate_type, { "Default", "Solid", "Shaded", "Metallic", "Glow", "Bubble" });

			ImGui::Checkbox(sxor("Local player"), &ctx.m_settings.colored_models_local_player);
			ImGui::ColorEdit4(sxor("##colored_models_local_player_color"), ctx.m_settings.colored_models_local_player_color);
			ImGui::SingleSelect(sxor("Local player models type"), &ctx.m_settings.colored_models_local_player_type, { "Default", "Solid", "Shaded", "Metallic", "Glow", "Bubble" });

			ImGui::Checkbox(sxor("Local player fake"), &ctx.m_settings.colored_models_local_player_fake);
			ImGui::ColorEdit4(sxor("##colored_models_local_player_fake_color"), ctx.m_settings.colored_models_local_player_fake_color);
			ImGui::SingleSelect(sxor("Fake models type"), &ctx.m_settings.colored_models_local_player_fake_type, { "Default", "Solid", "Shaded", "Metallic", "Glow", "Bubble" });

			ImGui::Checkbox(sxor("Hit capsule"), &ctx.m_settings.colored_models_hit_capsule);
			ImGui::ColorEdit4(sxor("##colored_models_hit_capsule_color"), ctx.m_settings.colored_models_hit_capsule_color);

			ImGui::EndChild();
		}
		if (tab == 4)
		{
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("Other ESP", ImVec2(max_size_x2, max_size_y));
			ImGui::Checkbox(sxor("Radar"), &ctx.m_settings.other_esp_radar);
			ImGui::MultiCombo(sxor("Dropped weapons"), ctx.m_settings.other_esp_dropped_weapons, dropped_weapons, ARRAYSIZE(dropped_weapons), preview);
			ImGui::ColorEdit4(sxor("##other_esp_dropped_weapons_color"), ctx.m_settings.other_esp_dropped_weapons_color);
			ImGui::Checkbox(sxor("Dropped weapons ammo"), &ctx.m_settings.other_esp_dropped_weapons_ammo);
			ImGui::Checkbox(sxor("Grenades"), &ctx.m_settings.other_esp_grenades);
			ImGui::ColorEdit4(sxor("##other_esp_grenades_color"), ctx.m_settings.other_esp_grenades_color);
			ImGui::Checkbox(sxor("Crosshair"), &ctx.m_settings.other_esp_crosshair);
			ImGui::Checkbox(sxor("Bomb"), &ctx.m_settings.other_esp_bomb);
			ImGui::ColorEdit4(sxor("##other_esp_bomb_color"), ctx.m_settings.other_esp_bomb_color);
			ImGui::Checkbox(sxor("Grenade trajectory"), &ctx.m_settings.other_esp_grenade_trajectory);
			ImGui::ColorEdit4(sxor("##other_esp_grenade_trajectory_color"), ctx.m_settings.other_esp_grenade_trajectory_color);

			ImGui::Checkbox(sxor("Grenade proximity warning"), &ctx.m_settings.other_esp_grenade_proximity_warning);

			if (ImGui::BeginCombo(sxor("Grenade proximity warning settings"), "Grenade proximity warning settings", 0, 8)) {
				static char config_name[64] = "\0";
				ImGui::Checkbox(sxor("Grenade proximity warning rainbow"), &ctx.m_settings.other_esp_grenade_warning_beam_rainbow);
				ImGui::ColorEdit4(sxor("##other_esp_grenade_warning_beam_color"), ctx.m_settings.other_esp_grenade_warning_beam_color);

				if (ImGui::InputText("Sprite name", config_name, sizeof(config_name), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					ctx.m_settings.other_esp_grenade_warning_beam_model_name = config_name;
				}
				ImGui::SliderFloat(sxor("Width"), &ctx.m_settings.other_esp_grenade_warning_beam_width, 0.f, 20.f);
				ImGui::SliderFloat(sxor("End Width"), &ctx.m_settings.other_esp_grenade_warning_beam_end_width, 0.f, 20.f);
				ImGui::SliderFloat(sxor("Halo"), &ctx.m_settings.other_esp_grenade_warning_beam_halo_scale, 0.f, 5.f);
				ImGui::SliderInt(sxor("Segments"), &ctx.m_settings.other_esp_grenade_warning_beam_segments, 0.f, 10.f);

				ImGui::Text(sxor("Warning Timer Color"));
				ImGui::ColorEdit4(sxor("##other_esp_grenade_warning_timer_color"), ctx.m_settings.other_esp_grenade_warning_timer_color);


				ImGui::EndCombo();
			}

			ImGui::Checkbox(sxor("Spectators"), &ctx.m_settings.other_esp_spectators);
			ImGui::Checkbox(sxor("Penetration reticle"), &ctx.m_settings.other_esp_penetration_reticle);
			
			ImGui::EndChild();
			
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110 + max_size_x2 + 10, wp.y + 30));
			ImGui::BeginChild("Effects", ImVec2(max_size_x2, max_size_y));
			ImGui::Checkbox(sxor("Remove flashbang effect"), &ctx.m_settings.effects_remove_flashbang_effect);
			ImGui::Checkbox(sxor("Remove smoke grenades"), &ctx.m_settings.effects_remove_smoke_grenades);
			ImGui::Checkbox(sxor("Remove fog"), &ctx.m_settings.effects_remove_fog);
			ImGui::SingleSelect(sxor("Visual recoil adjustment"), &ctx.m_settings.effects_visual_recoil_adjustment, { "Off", "Remove shake", "Remove all" });
			ImGui::SliderInt(sxor("Transparent walls"), &ctx.m_settings.effects_transparent_walls, 0, 100, "%d%");
			ImGui::SliderInt(sxor("Transparent props"), &ctx.m_settings.effects_transparent_props, 0, 100, "%d%");
			ImGui::SingleSelect(sxor("Brightness adjustment"), &ctx.m_settings.effects_brightness_adjustment, { "Off", "Fullbright", "Night mode" });

			if (ctx.m_settings.effects_brightness_adjustment == 2)
				ImGui::SliderInt(sxor("##effects_night_adjustment"), &ctx.m_settings.effects_night_adjustment, 0, 100, "%d%");

			ImGui::Checkbox(sxor("Remove scope overlay"), &ctx.m_settings.effects_remove_scope_overlay);
			ImGui::Checkbox(sxor("Instant scope"), &ctx.m_settings.effects_instant_scope);
			ImGui::Checkbox(sxor("Disable post processing"), &ctx.m_settings.effects_disable_post_processing);
			ImGui::Checkbox(sxor("Force third person (alive)"), &ctx.m_settings.effects_force_third_person_alive);
			ImGui::Keybind(sxor("##effects_force_third_person_key"), &ctx.m_settings.effects_force_third_person_key.key, &ctx.m_settings.effects_force_third_person_key.mode);
			ImGui::SliderInt(sxor("Third person distance"), &ctx.m_settings.effects_force_third_person_distance, 60, 400, "%d%");

			ImGui::Checkbox(sxor("Force third person (dead)"), &ctx.m_settings.effects_force_third_person_dead);
			ImGui::Checkbox(sxor("Disable rendering of teammates"), &ctx.m_settings.effects_disable_rendering_of_teammates);
			ImGui::Checkbox(sxor("Bullet tracers"), &ctx.m_settings.effects_bullet_tracers);
			ImGui::ColorEdit4(sxor("##effects_bullet_tracers_color"), ctx.m_settings.effects_bullet_tracers_color);

			ImGui::Checkbox(sxor("Bullet impact"), &ctx.m_settings.effects_bullet_impact);
			ImGui::Checkbox(sxor("Override skybox"), &ctx.m_settings.effects_override_skybox);
			ImGui::ColorEdit4(sxor("##effects_override_skybox_color"), ctx.m_settings.effects_override_skybox_color);

			ImGui::SliderInt(sxor("Aspect ration"), &ctx.m_settings.effects_aspect_ratio, 0, 200, "%d%");

			ImGui::EndChild();
		}
		if (tab == 5)
		{
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("Miscellaneous", ImVec2(max_size_x2, max_size_y));
			ImGui::SliderInt(sxor("Override FOV"), &ctx.m_settings.miscellaneous_override_fov, 0, 120, "%d");
			ImGui::SliderInt(sxor("Override zoom FOV"), &ctx.m_settings.miscellaneous_override_zoom_fov, 0, 100, "%d%");


			ImGui::Checkbox(sxor("Knifebot"), &ctx.m_settings.miscellaneous_knifebot);

			ImGui::MultiCombo(sxor("Knife bot settings"), ctx.m_settings.miscellaneous_knifebot_knifebot_settings, knifebot_types, ARRAYSIZE(knifebot_types), preview);

			ImGui::Checkbox(sxor("Zeusbot"), &ctx.m_settings.miscellaneous_zeusbot);
			ImGui::Checkbox(sxor("Reveal completitive ranks"), &ctx.m_settings.miscellaneous_reveal_completitive_ranks);
			ImGui::Checkbox(sxor("Reveal Overwatch players"), &ctx.m_settings.miscellaneous_reveal_overwatch_players);
			ImGui::Checkbox(sxor("Auto-accept mathmaking"), &ctx.m_settings.miscellaneous_reveal_overwatch_players);
			ImGui::Checkbox(sxor("Clan tag spammer"), &ctx.m_settings.miscellaneous_clan_tag_spammer);
			ImGui::Checkbox(sxor("Log weapon purchases"), &ctx.m_settings.miscellaneous_log_weapon_purchases);
			ImGui::Checkbox(sxor("Log damage dealt"), &ctx.m_settings.miscellaneous_log_damage_dealt);
			ImGui::Checkbox(sxor("Persistent kill feed"), &ctx.m_settings.miscellaneous_persistent_kill_feed);

			ImGui::Checkbox(sxor("Unlock hidden cvars"), &ctx.m_settings.miscellaneous_unlock_hidden_cvars);
			static bool unlocked = false;
			if (ctx.m_settings.miscellaneous_unlock_hidden_cvars && !unlocked)
			{
				feature::misc->unlock_cl_cvars();
				unlocked = true;
			}
			ImGui::EndChild();

			ImGui::SetNextWindowPos(ImVec2(wp.x + 110 + max_size_x2 + 10, wp.y + 30));
			ImGui::BeginChild("Movement", ImVec2(max_size_x2, max_size_y2));
			ImGui::Checkbox(sxor("Infinite duck"), &ctx.m_settings.movement_infinite_duck);
			ImGui::Checkbox(sxor("Bunny hop"), &ctx.m_settings.movement_bunny_hop);
			ImGui::Checkbox(sxor("Air strafe"), &ctx.m_settings.movement_air_strafe);
			ImGui::MultiCombo(sxor("Air strafe direction"), ctx.m_settings.movement_air_strafe_direction, airstrafe_types, ARRAYSIZE(airstrafe_types), preview);
			ImGui::SliderInt(sxor("Air strafe smoothing"), &ctx.m_settings.movement_air_strafe_smoothing, 0, 100, "%d%");
			ImGui::SingleSelect(sxor("Air duck"), &ctx.m_settings.movement_air_duck, { "Off", "On" });

			ImGui::Checkbox(sxor("Jump at edge"), &ctx.m_settings.movement_jump_at_edge);
			ImGui::Keybind(sxor("##jump_at_edge_key"), &ctx.m_settings.movement_jump_at_edge_key.key, &ctx.m_settings.movement_jump_at_edge_key.mode);

			ImGui::EndChild();

			ImGui::SetNextWindowPos(ImVec2(wp.x + 110 + max_size_x2 + 10, wp.y + 30 + max_size_y2 + 10));
			ImGui::BeginChild("Settings", ImVec2(max_size_x2, max_size_y2));

			ImGui::Text("Menu key");
			ImGui::Keybind(sxor("##menu_keybind_key"), &ctx.m_settings.settings_menu_key.key, &ctx.m_settings.settings_menu_key.mode);

			ImGui::Text("Menu color");
			ImGui::ColorEdit4("#menucolor", ctx.m_settings.menu_color);

			ImGui::SingleSelect(sxor("DPI scale"), &ctx.m_settings.settings_dpi_scale, { "100%", "125%", "150%", "175%", "200%" });

			ImGui::Checkbox(sxor("Anti-untrusted"), &ctx.m_settings.settings_anti_untrusted);
			ImGui::Checkbox(sxor("Hide from OBS"), &ctx.m_settings.settings_hide_from_obs);
			ImGui::Checkbox(sxor("Low FPS warning"), &ctx.m_settings.settings_low_fps_warning);

			ImGui::Checkbox(sxor("Lock menu layout"), &ctx.m_settings.settings_lock_menu_layout);

			if (ImGui::CustomButton(sxor("Unload"), sxor("Unload"), ImVec2(50, 20)))
				ctx.force_unload = true;

			ImGui::EndChild();
		}
		if (tab == 6)
		{
			if (weaponSkin.wId == WEAPON_NONE)
				weaponSkin.wId = WEAPON_DEAGLE;

			auto weaponName = zweaponnames(weaponSkin.wId);
			int i = 0;

			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("inventory", ImVec2(max_size_x, max_size_y));

			int line_count = 0;
			static bool add_new_tab = false;
			static bool changer_skin_tab = false;

			if (!add_new_tab && !changer_skin_tab)
			{
				for (auto weapon : g_InventorySkins)
				{
					std::string weap = normal_weapon_names(weapon.second.wId);
					std::stringstream whatever;
					if (ImGui::CustomButton(weap.c_str(), weap.c_str(), ImVec2(120, 120)))
					{
						index = weapon.first;
						changer_skin_tab = true;
					}
					if (line_count < round((max_size_x - 125) / 133))
					{
						ImGui::SameLine(0, 5);
						line_count++;
					}
					else
						line_count = 0;

				}

				if (ImGui::CustomButton("+", "+", ImVec2(120, 120)))
				{
					add_new_tab = true;
				}
			}
			else
			{
				if (add_new_tab)
				{
					static char skinname[64] = "\0";

					skins_listbox("##weapons", 15);

					if (ImGui::BeginCombo("Paint kit", weaponSkin.paintKit > 0 ? _inv.inventory.skinInfo[weaponSkin.paintKit].name.c_str() : "none", ImGuiComboFlags_HeightLargest, 15))
					{
						//int lastID = ImGui::getwindow();
						int lastID = ImGui::GetItemID();
						for (auto skin : _inv.inventory.skinInfo)
						{
							for (auto names : skin.second.weaponName)
							{
								if (weaponName != names)
									continue;

								ImGui::PushID(lastID++);

								if (ImGui::Selectable(skin.second.name.c_str(), skin.first == weaponSkin.paintKit, 0, ImVec2()))
									weaponSkin.paintKit = skin.first;

								ImGui::PopID();
							}
						}
						ImGui::EndCombo();
					}
					ImGui::Text("statrack");
					ImGui::InputInt("##statrack", &weaponSkin.stattrak);
					ImGui::Text("wear");
					ImGui::InputFloat("##wear", &weaponSkin.wear);
					ImGui::Text("seed");
					ImGui::InputInt("##seed", &weaponSkin.seed);

					ImGui::Text("custom name");
					ImGui::InputText("##name", skinname, sizeof(skinname), 0);


					static int stickerkit[4] = { 0,0,0,0 };

					if (weaponSkin.wId <= 100 && weaponSkin.wId != 42 && weaponSkin.wId != 59)
					{

						if (ImGui::BeginCombo("Sticker 1", g_Stickers[stickerkit[0]].name.c_str(), ImGuiComboFlags_HeightLargest, g_Stickers.size()))
						{
							for (auto skin : fosso)
							{
								{
									//ImGui::PushID(lastID++);
									if (ImGui::Selectable(skin.second.name.c_str(), skin.second.paintKit == stickerkit[0], 0, ImVec2()))
										stickerkit[0] = skin.second.paintKit;
									//ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}

						if (ImGui::BeginCombo("Sticker 2", g_Stickers[stickerkit[1]].name.c_str(), ImGuiComboFlags_HeightLargest, g_Stickers.size()))
						{
							for (auto skin : fosso)
							{
								{
									//ImGui::PushID(lastID++);
									if (ImGui::Selectable(skin.second.name.c_str(), skin.second.paintKit == stickerkit[1], 0, ImVec2()))
										stickerkit[1] = skin.second.paintKit;
									//ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}

						if (ImGui::BeginCombo("Sticker 3", g_Stickers[stickerkit[2]].name.c_str(), ImGuiComboFlags_HeightLargest, g_Stickers.size()))
						{
							int lastID = ImGui::GetItemID();

							for (auto skin : fosso)
							{
								{
									ImGui::PushID(lastID++);
									if (ImGui::Selectable(skin.second.name.c_str(), skin.second.paintKit == stickerkit[2], 0, ImVec2()))
										stickerkit[2] = skin.second.paintKit;
									ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}

						if (ImGui::BeginCombo("Sticker 4", g_Stickers[stickerkit[3]].name.c_str(), ImGuiComboFlags_HeightLargest, g_Stickers.size()))
						{
							int lastID = ImGui::GetItemID();

							for (auto skin : fosso)
							{
								{
									ImGui::PushID(lastID++);
									if (ImGui::Selectable(skin.second.name.c_str(), skin.second.paintKit == stickerkit[3], 0, ImVec2()))
										stickerkit[3] = skin.second.paintKit;
									ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}


					}
					if (ImGui::CustomButton("Add", "Add", ImVec2(50, 20)))
					{
						weaponSkin.sicker[0] = stickerkit[0];
						weaponSkin.sicker[1] = stickerkit[1];
						weaponSkin.sicker[2] = stickerkit[2];
						weaponSkin.sicker[3] = stickerkit[3];

						std::string str(skinname);
						if (str.length() > 0)
							weaponSkin.name = str;
						g_InventorySkins.insert({ RandomInt(20000, 200000), weaponSkin });
						_inv.inventory.itemCount = g_InventorySkins.size();
						stickerkit[0] = 0;
						stickerkit[1] = 0;
						stickerkit[2] = 0;
						stickerkit[3] = 0;
						//index = 0;


						csgo.m_engine()->ExecuteClientCmd("econ_clear_inventory_images");
						write.SendClientHello();
						write.SendMatchmakingClient2GCHello();

						add_new_tab = false;
					}
					if (ImGui::CustomButton("back", "back", ImVec2(50, 20)))
					{
						add_new_tab = false;
					}
				}
				if (changer_skin_tab)
				{
					//for (auto& weapon : g_InventorySkins)
					//{
					static char skinname[64] = "\0";

					if (g_InventorySkins.at(index).wId < 4000)
					{
						if (ImGui::BeginCombo("Paint Kit", g_InventorySkins.at(index).paintKit > 0 ? _inv.inventory.skinInfo[g_InventorySkins.at(index).paintKit].name.c_str() : "none", ImGuiComboFlags_HeightLargest, 15))
						{
							//int lastID = ImGui::getwindow();
							int lastID = ImGui::GetItemID();
							for (auto skin : _inv.inventory.skinInfo)
							{
								for (auto names : skin.second.weaponName)
								{
									if (zweaponnames(g_InventorySkins.at(index).wId) != names)
										continue;

									ImGui::PushID(lastID++);

									if (ImGui::Selectable(skin.second.name.c_str(), skin.first == g_InventorySkins.at(index).paintKit, 0, ImVec2()))
										g_InventorySkins.at(index).paintKit = skin.first;

									ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}


						ImGui::Text("statrack");
						ImGui::InputInt("##statrack", &weaponSkin.stattrak);
						ImGui::Text("wear");
						ImGui::InputFloat("##wear", &weaponSkin.wear);
						ImGui::Text("seed");
						ImGui::InputInt("##seed", &weaponSkin.seed);

						ImGui::Text("custom name");
						ImGui::InputText("##name", skinname, sizeof(skinname), 0);
					}

					static int stickerkit[4] = { 0,0,0,0 };

					if (g_InventorySkins.at(index).wId <= 100 && g_InventorySkins.at(index).wId != 42 && g_InventorySkins.at(index).wId != 59)
					{

						if (ImGui::BeginCombo("Sticker 1", g_Stickers[stickerkit[0]].name.c_str(), ImGuiComboFlags_HeightLargest, g_Stickers.size()))
						{
							for (auto skin : fosso)
							{
								{
									//ImGui::PushID(lastID++);
									if (ImGui::Selectable(skin.second.name.c_str(), skin.second.paintKit == stickerkit[0], 0, ImVec2()))
										stickerkit[0] = skin.second.paintKit;
									//ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}

						if (ImGui::BeginCombo("Sticker 2", g_Stickers[stickerkit[1]].name.c_str(), ImGuiComboFlags_HeightLargest, g_Stickers.size()))
						{
							for (auto skin : fosso)
							{
								{
									//ImGui::PushID(lastID++);
									if (ImGui::Selectable(skin.second.name.c_str(), skin.second.paintKit == stickerkit[1], 0, ImVec2()))
										stickerkit[1] = skin.second.paintKit;
									//ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}

						if (ImGui::BeginCombo("Sticker 3", g_Stickers[stickerkit[2]].name.c_str(), ImGuiComboFlags_HeightLargest, g_Stickers.size()))
						{
							int lastID = ImGui::GetItemID();

							for (auto skin : fosso)
							{
								{
									ImGui::PushID(lastID++);
									if (ImGui::Selectable(skin.second.name.c_str(), skin.second.paintKit == stickerkit[2], 0, ImVec2()))
										stickerkit[2] = skin.second.paintKit;
									ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}

						if (ImGui::BeginCombo("Sticker 4", g_Stickers[stickerkit[3]].name.c_str(), ImGuiComboFlags_HeightLargest, g_Stickers.size()))
						{
							int lastID = ImGui::GetItemID();

							for (auto skin : fosso)
							{
								{
									ImGui::PushID(lastID++);
									if (ImGui::Selectable(skin.second.name.c_str(), skin.second.paintKit == stickerkit[3], 0, ImVec2()))
										stickerkit[3] = skin.second.paintKit;
									ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}


					}
					if (ImGui::CustomButton("change", "change", ImVec2(ImGui::GetWindowSize().x / 2 - 32, 25)))
					{
						g_InventorySkins.at(index).sicker[0] = stickerkit[0];
						g_InventorySkins.at(index).sicker[1] = stickerkit[1];
						g_InventorySkins.at(index).sicker[2] = stickerkit[2];
						g_InventorySkins.at(index).sicker[3] = stickerkit[3];

						std::string str(skinname);
						if (str.length() > 0)
							g_InventorySkins.at(index).name = str;
						//g_InventorySkins.insert({ RandomInt(20000, 200000), g_InventorySkins.at(index) });
						_inv.inventory.itemCount = g_InventorySkins.size();
						//index = 0;


						csgo.m_engine()->ExecuteClientCmd("econ_clear_inventory_images");
						write.SendClientHello();
						write.SendMatchmakingClient2GCHello();

						changer_skin_tab = false;
					}
					ImGui::SameLine();

					if (ImGui::CustomButton("delete", "delete", ImVec2(ImGui::GetWindowSize().x / 2 - 32, 25)))
					{
						changer_skin_tab = false;
						// if (g_InventorySkins[index] != NULL)
						g_InventorySkins.erase(index);
						_inv.inventory.itemCount = g_InventorySkins.size();
						index--;
						csgo.m_engine()->ExecuteClientCmd("econ_clear_inventory_images");
						write.SendClientHello();
						write.SendMatchmakingClient2GCHello();
					}

					if (ImGui::CustomButton("back", "back", ImVec2(50, 20)))
					{
						changer_skin_tab = false;
					}
					//}
				}
			}
			i = 0;
			ImGui::EndChild();
		}
		if (tab == 7)
		{
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("config list", ImVec2(max_size_x, max_size_y));

			static auto should_update = true;

			if (should_update)
			{
				should_update = false;

				cfg_manager->config_files();
				cfg_manager->files = cfg_manager->files;
			}

			//ImGui::InputText("config name", );

			static int operation = 0;
			int line_count = 0;
			int last_pos_y = wp.y + 55;
			for (int i = 0; i < cfg_manager->files.size(); i++)
			{
				std::stringstream asdsss;
				asdsss << "##" << cfg_manager->files.at(i).c_str();
				ImGui::SetNextWindowPos(ImVec2(wp.x + 120, last_pos_y));
				ImGui::BeginChild(asdsss.str().c_str(), ImVec2(max_size_x - 20, 60), true, ImGuiWindowFlags_NoName);
				auto winpos = ImGui::GetWindowPos();
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
				ImGui::GetWindowDrawList()->AddText(ImVec2(winpos.x + 10, winpos.y + 7), ImColor(200, 200, 200, 255), cfg_manager->files.at(i).c_str());
				ImGui::PopFont();

				std::stringstream last_update;
				last_update << "Updated: 14.88.1337";
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
				ImGui::GetWindowDrawList()->AddText(ImVec2(winpos.x + 10, winpos.y + ImGui::CalcTextSize(cfg_manager->files.at(i).c_str()).y + 22), ImColor(150, 150, 150, 255), last_update.str().c_str());
				ImGui::PopFont();


				std::stringstream creator;
				creator << "Creator: Dlj";
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
				ImGui::GetWindowDrawList()->AddText(ImVec2(winpos.x + 10 + ImGui::CalcTextSize(last_update.str().c_str()).x + 15, winpos.y + ImGui::CalcTextSize(cfg_manager->files.at(i).c_str()).y + 22), ImColor(150, 150, 150, 255), creator.str().c_str());
				ImGui::PopFont();

				static bool loaded_config[128];

				ImGui::SameLine(ImGui::GetWindowSize().x - 120);

				std::stringstream config_sl;
				std::stringstream config_sls;

				config_sl << "Save##" << cfg_manager->files.at(i).c_str();

				if (ImGui::CustomButton(config_sl.str().c_str(), config_sl.str().c_str(), ImVec2(100, 40)))
				{
					ctx.m_settings.selected_config = i;
					save_config();
				}
				ImGui::SameLine(ImGui::GetWindowSize().x - 240);

				config_sls << "Load##" << cfg_manager->files.at(i).c_str();
				if (ImGui::CustomButton(config_sls.str().c_str(), config_sls.str().c_str(), ImVec2(100, 40)))
				{
					ctx.m_settings.selected_config = i;
					load_config();
				}
				ImGui::EndChild();

				last_pos_y += 70;
			}
			ImGui::SetNextWindowPos(ImVec2(wp.x + 120, last_pos_y));
			ImGui::BeginChild("createnewconfig", ImVec2(max_size_x - 20, 60), true, ImGuiWindowFlags_NoName);
			auto winpos = ImGui::GetWindowPos();
			static bool settings_acc = false;
			static char config_name[64] = "\0";
			if (!settings_acc)
			{
				ImGui::SameLine(ImGui::GetWindowSize().x - max_size_x + 24);
				if (ImGui::CustomButton("Refresh Configs", "Refresh Configs", ImVec2((ImGui::GetWindowSize().x - 15) / 3, 40)))
					should_update = true; ImGui::SameLine();
				if (ImGui::CustomButton("Create New", "Create New", ImVec2((ImGui::GetWindowSize().x - 15) / 3, 40)))
					settings_acc = true; ImGui::SameLine();
				if (ImGui::CustomButton("Open Configs folder", "Open Configs folder", ImVec2((ImGui::GetWindowSize().x - 15) / 3, 40)))
					ShellExecute(NULL, "open", "darkraihook\\", NULL, NULL, SW_RESTORE);

			}
			else
			{
				ImGui::SameLine(ImGui::GetWindowSize().x - max_size_x + 12);
				if (ImGui::InputText("##config_new", config_name, sizeof(config_name), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					ctx.m_settings.new_config_name = config_name;
					settings_acc = false;
					add_config();


					should_update = true;
				}

				if (GetAsyncKeyState(VK_ESCAPE))
				{
					should_update = true;
					settings_acc = false;
				}
			}

			ImGui::EndChild();

			ImGui::EndChild();
		}
		if (tab == 8)
		{
			ImGui::SetNextWindowPos(ImVec2(wp.x + 110, wp.y + 30));
			ImGui::BeginChild("scripts list", ImVec2(max_size_x, max_size_y));
			int line_count = 0;
			int last_pos_y = wp.y + 55;
			static bool need_edit = false;
			static int selected_script = 0;
			if (!need_edit)
			{

				for (int i = 0; i < c_lua::get().scripts.size(); i++)
				{
					std::stringstream asdsss;
					asdsss << "##" << c_lua::get().scripts.at(i).c_str();
					ImGui::SetNextWindowPos(ImVec2(wp.x + 120, last_pos_y));
					ImGui::BeginChild(asdsss.str().c_str(), ImVec2(max_size_x - 20, 60), true, ImGuiWindowFlags_NoName);
					auto winpos = ImGui::GetWindowPos();
					ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
					ImGui::GetWindowDrawList()->AddText(ImVec2(winpos.x + 10, winpos.y + 7), ImColor(200, 200, 200, 255), c_lua::get().scripts.at(i).c_str());
					ImGui::PopFont();

					std::stringstream last_update;
					last_update << "Updated: 14.88.1337";
					ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
					ImGui::GetWindowDrawList()->AddText(ImVec2(winpos.x + 10, winpos.y + ImGui::CalcTextSize(c_lua::get().scripts.at(i).c_str()).y + 22), ImColor(150, 150, 150, 255), last_update.str().c_str());
					ImGui::PopFont();


					std::stringstream creator;
					creator << "Creator: Dlj";
					ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
					ImGui::GetWindowDrawList()->AddText(ImVec2(winpos.x + 10 + ImGui::CalcTextSize(last_update.str().c_str()).x + 15, winpos.y + ImGui::CalcTextSize(c_lua::get().scripts.at(i).c_str()).y + 22), ImColor(150, 150, 150, 255), creator.str().c_str());
					ImGui::PopFont();

					ImGui::SameLine(ImGui::GetWindowSize().x - 140);

					ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[5]);
					ImGui::GetWindowDrawList()->AddText(ImVec2(winpos.x + ImGui::GetWindowSize().x - 130, winpos.y + ImGui::CalcTextSize(c_lua::get().scripts.at(i).c_str()).y + 8), ImColor(150, 150, 150, 255), "d");
					ImGui::PopFont();

					if (ImGui::InvisibleButton("edit", ImVec2(20, 20)))
					{
						selected_script = i;
						need_edit = true;
					}


					ImGui::SameLine(ImGui::GetWindowSize().x - 110);

					std::stringstream config_sl;
					if (!c_lua::get().loaded.at(c_lua::get().get_script_id(c_lua::get().scripts.at(i))))
						config_sl << "Load##" << c_lua::get().scripts.at(i).c_str();
					else
						config_sl << "Unload##" << c_lua::get().scripts.at(i).c_str();

					if (ImGui::CustomButton(config_sl.str().c_str(), config_sl.str().c_str(), ImVec2(100, 40)))
					{
						if (c_lua::get().loaded.at(c_lua::get().get_script_id(c_lua::get().scripts.at(i))))
						{
							c_lua::get().unload_script(c_lua::get().get_script_id(c_lua::get().scripts.at(i)));
						}
						else
						{
							c_lua::get().load_script(c_lua::get().get_script_id(c_lua::get().scripts.at(i)));
						}
					}
					ImGui::EndChild();
					last_pos_y += 70;
				}
				ImGui::SetNextWindowPos(ImVec2(wp.x + 120, last_pos_y));
				ImGui::BeginChild("settings context", ImVec2(max_size_x - 20, 60), true, ImGuiWindowFlags_NoName);

				ImGui::SameLine(ImGui::GetWindowSize().x - max_size_x + 12);
				if (ImGui::CustomButton("Refresh scripts", "Refresh scripts", ImVec2((ImGui::GetWindowSize().x - 15) / 3, 40)))
					c_lua::get().refresh_scripts(); ImGui::SameLine();
				if (ImGui::CustomButton("Reload active scripts", "Reload active scripts", ImVec2((ImGui::GetWindowSize().x - 15) / 3, 40)))
					c_lua::get().reload_all_scripts(); ImGui::SameLine();
				if (ImGui::CustomButton("Unload all", "Unload all", ImVec2((ImGui::GetWindowSize().x - 15) / 3, 40)))
					c_lua::get().unload_all_scripts();

				ImGui::EndChild();
			}
			else
			{
				auto previous_check_box = false;
				auto& items = c_lua::get().items.at(selected_script);

				for (auto& item : items)
				{
					std::string item_name;

					auto first_point = false;
					auto item_str = false;

					for (auto& c : item.first)
					{
						if (c == '.')
						{
							if (first_point)
							{
								item_str = true;
								continue;
							}
							else
								first_point = true;
						}

						if (item_str)
							item_name.push_back(c);
					}

					switch (item.second.type)
					{
					case NEXT_LINE:
						previous_check_box = false;
						break;
					case CHECK_BOX:
						previous_check_box = true;
						ImGui::Checkbox(item_name.c_str(), &item.second.check_box_value);
						break;
					case COMBO_BOX:
						previous_check_box = false;
						draw_combo_lua(item_name.c_str(), item.second.combo_box_value, [](void* data, int idx, const char** out_text)
							{
								auto labels = (std::vector <std::string>*)data;
								*out_text = labels->at(idx).c_str(); //-V106
								return true;
							}, &item.second.combo_box_labels, item.second.combo_box_labels.size());
						break;
					case SLIDER_INT:
						previous_check_box = false;
						ImGui::SliderInt(item_name.c_str(), &item.second.slider_int_value, item.second.slider_int_min, item.second.slider_int_max);
						break;
					case SLIDER_FLOAT:
						previous_check_box = false;
						ImGui::SliderFloat(item_name.c_str(), &item.second.slider_float_value, item.second.slider_float_min, item.second.slider_float_max);
						break;
					case COLOR_PICKER:
						if (previous_check_box)
							previous_check_box = false;
						else
							ImGui::Text((item_name + ' ').c_str());

						ImGui::SameLine();
						ImGui::ColorEdit4((("##") + item_name).c_str(), item.second.color_picker_value);
						break;

					case KEY_BIND:
						if (previous_check_box)
							previous_check_box = false;
						else
							ImGui::Text((item_name + ' ').c_str());

						ImGui::SameLine();
						ImGui::Keybind((("##") + item_name).c_str(), &item.second.keybind_key_value, &item.second.keybind_key_mode);
						break;
					}
				}

				if (ImGui::CustomButton(("<- Back"), ("<- Back"), ImVec2(50, 25)))
					need_edit = false;
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}
}