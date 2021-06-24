#pragma once

#include "sdk.hpp"
#include <props/player.hpp>
#include <array>
#include <ragebot/lag_comp.hpp>
#include <menu/menu/menu.hpp>

namespace Source
{
	extern HWND Window;

	bool Create();
	void Destroy();
	void QueueJobs();

	void* CreateInterface(const std::string& image_name, const std::string& name, bool force = false);
	void* CreateInterface(ULONG64 offset);
}

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define pad(size) char TOKENPASTE2(padding_, __LINE__) [size]

#define XOR_VAL xor_val
#define ADD_VAL 0x54F52D73//0x54F12F43
//#define ADD_VAL2 0x54F52D73

#pragma  optimize( "", off ) 
template<typename T>
class Encrypted_t {
	__forceinline uintptr_t rotate_dec(uintptr_t c) const {
		return c;
	}
public:
	uintptr_t np;
	uintptr_t xor_val;

	__forceinline Encrypted_t(T* ptr) {
		auto p = &ptr;
		xor_val = rotate_dec(CRC32_GetTableEntry(*((uint8_t*)p + 1) + (((uintptr_t(ptr) >> 16) & 7) << 8)));
		np = rotate_dec(rotate_dec(xor_val) ^ (uintptr_t(ptr) + ADD_VAL));
	}

	__forceinline  T* get() const {
		return (T*)((uintptr_t)(rotate_dec(np) ^ rotate_dec(xor_val)) - ADD_VAL);
	}

	__forceinline  T* operator-> () {
		return get();
	}

	__forceinline bool IsValid() const {
		return ((uintptr_t)(rotate_dec(np) ^ rotate_dec(xor_val)) - ADD_VAL) != 0;
	}
};

template <typename t>
class c_interface_base
{
public:
	t* m_interface = nullptr;

	__forceinline t* operator()()
	{
		return get();
	}

	__forceinline virtual t* get()
	{
		return m_interface;
	}

	__forceinline void set(t* ur_shit)
	{
		m_interface = ur_shit;
	}
};
#pragma  optimize( "", on )

class c_variables
{
public:
	bool	aimbot_enabled = false;
	int		aimbot_target_selection = 0;

	struct weapon
	{
		std::vector <int> aimbot_target_hitbox;
		std::vector <int> aimbot_multi_point;
		int		aimbot_multi_point_scale = 0;
		int     aimbot_multi_point_strange = 0;

		bool	aimbot_prefer_safe_point = false;
		bool    aimbot_force_safe_point_on_limbs = false;
		bool    aimbot_automatic_fire = false;
		bool    aimbot_automatic_penetration = false;
		bool    aimbot_silent_aim = false;
		c_keybind aimbot_force_safe_point_key = c_keybind{ 0, 0, false, 0 };


		int      aimbot_minimum_hit_chance = 0;
		int      aimbot_minimum_damage = 0;
		int      aimbot_minimum_autowall_damage = 0;
		int      aimbot_minimum_override_damage = 0;

		bool    aimbot_automatic_scope = false;
		///bool    aimbot_reduce_aim_step = false;

		int     aimbot_maximum_fov = 0;

		bool    aimbot_log_misses_due_to_spread = false;

		bool    aimbot_automatic_stope = false;

		std::vector <int> aimbot_low_fps_mitigations;

		bool    aimbot_remove_recoil = false;
		int     aimbot_accuracy_boost = 0;
		bool    aimbot_delay_shot = false;
		bool    aimbot_quick_stop = false;

		std::vector <int> aimbot_quick_stop_options;

		bool    aimbot_anti_aim_corrections = false;
		bool    aimbot_double_tap = false;

		int     aimbot_double_tap_mode = 0;
		c_keybind aimbot_min_damage_override = c_keybind{ 0, 0, false, 0 };
	} aimbot_weapon[8];


	c_keybind aimbot_double_tap_key = c_keybind{ 0, 0, false, 0 };


	bool	anti_aim_enabled = false;
	int		anti_aim_pitch = 0;
	int		anti_aim_yaw_base = 0;
	int		anti_aim_yaw_type = 0;
	int		anti_aim_yaw_custom = 0;
	int		anti_aim_yaw_jitter_type = 0;
	int		anti_aim_yaw_jitter_custom = 0;
	int		anti_aim_body_yaw = 0;
	bool	anti_aim_freestanding_body_yaw = false;
	int		anti_aim_lby_yaw_target = 0;
	int		anti_aim_fake_limit = 0;
	bool	anti_aim_edge_yaw = 0;
	int		anti_aim_freestanding_fake_type = 0;
	c_keybind anti_aim_freestand_key = c_keybind{ 0, 0, false, 0 };
	int		anti_aim_leg_movement = 0;
	c_keybind anti_aim_fake_switch_key = c_keybind{ 0, 1 , false, 0 }; //key is: Z
	c_keybind anti_aim_yaw_left_switch_key = c_keybind{ 0, 1, false, 0 };
	c_keybind anti_aim_yaw_right_switch_key = c_keybind{ 0, 1, false, 0 };
	c_keybind anti_aim_yaw_backward_switch_key = c_keybind{ 0, 1 , false, 0 };
	c_keybind anti_aim_slowwalk_key = c_keybind{ 0, 1 , false, 0 };


	bool	fake_lag_enabled = false;
	int		fake_lag_type = 0;
	int		fake_lag_value = 0;
	float   fake_lag_variance = 0.f;
	bool	fake_lag_shooting = false;
	c_keybind anti_aim_autopeek_key = c_keybind{ 0, 0, false, 0 };
	c_keybind anti_aim_fakeduck_key = c_keybind{ 0, 0, false, 0 };
	c_keybind aimbot_hideshots_exploit_toggle = c_keybind{ 0, 0, false, 0 };

	bool	player_esp_dormant = false;
	bool	player_esp_bounding_box = false;
	float   player_esp_bounding_box_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool	player_esp_health_bar = false;
	bool	player_esp_name = false;
	float   player_esp_name_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool player_esp_flags;
	bool	player_esp_weapon_text = false;
	bool	player_esp_weapon_icon = false;
	float   player_esp_weapon_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool	player_esp_ammo = false;
	float   player_esp_ammo_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool	player_esp_distance = false;

	struct glow_esp
	{
		bool enabled = false;
		float color[4] = { 1.f, 1.f, 1.f, 1.f };
		bool m_bFullBloomRender = false;
		bool m_bRenderWhenOccluded = false;
		bool m_bRenderWhenUnoccluded = false;
	} glow[3];

	bool	player_esp_hit_marker = false;
	bool	player_esp_hit_marker_sound = false;
	bool	player_esp_money = false;
	bool	player_esp_skeleton = false;
	float   player_esp_skeleton_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool	player_esp_out_of_fov_arrow = false;
	float   player_esp_out_of_fov_arrow_color[4] = { 1.f, 1.f, 1.f, 1.f };
	int     player_esp_out_of_fov_arrow_size = 0;
	int     player_esp_out_of_fov_arrow_distance = 0;


	bool	colored_models_player = false;
	float   colored_models_player_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool	colored_models_player_behind_wall = false;
	float   colored_models_player_behind_wall_color[4] = { 1.f, 1.f, 1.f, 1.f };
	int     colored_models_player_type = 0;

	bool	colored_models_teammate = false;
	float   colored_models_teammate_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool	colored_models_teammate_behind_wall = false;
	float   colored_models_teammate_behind_wall_color[4] = { 1.f, 1.f, 1.f, 1.f };
	int     colored_models_teammate_type = 0;

	bool	colored_models_local_player = false;
	float   colored_models_local_player_color[4] = { 1.f, 1.f, 1.f, 1.f };
	int     colored_models_local_player_type = 0;
	bool	colored_models_local_player_fake = false;
	float   colored_models_local_player_fake_color[4] = { 1.f, 1.f, 1.f, 1.f };
	int     colored_models_local_player_fake_type = 0;

	bool	colored_models_hit_capsule = false;
	float   colored_models_hit_capsule_color[4] = { 1.f, 1.f, 1.f, 1.f };


	bool    other_esp_radar = false;
	std::vector <int> other_esp_dropped_weapons;
	float   other_esp_dropped_weapons_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool    other_esp_dropped_weapons_ammo = false;
	bool    other_esp_grenades = false;
	float   other_esp_grenades_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool    other_esp_crosshair = false;
	bool    other_esp_bomb = false;
	float   other_esp_bomb_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool    other_esp_grenade_trajectory = false;
	float   other_esp_grenade_trajectory_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool    other_esp_grenade_proximity_warning = false;
	bool    other_esp_grenade_warning_beam_rainbow = false;
	float   other_esp_grenade_warning_beam_color[4] = { 1.f, 1.f, 1.f, 1.f };
	float   other_esp_grenade_warning_timer_color[4] = { 1.f, 1.f, 1.f, 1.f };
	float   other_esp_grenade_warning_beam_width = 0.f;
	float   other_esp_grenade_warning_beam_end_width = 0.f;
	float   other_esp_grenade_warning_beam_halo_scale = 0.f;
	int     other_esp_grenade_warning_beam_segments = 0;

	std::string other_esp_grenade_warning_beam_model_name;
	bool    other_esp_spectators = false;
	bool    other_esp_penetration_reticle = false;

	bool	effects_remove_flashbang_effect = false;
	bool	effects_remove_smoke_grenades = false;
	bool	effects_remove_fog = false;
	bool	effects_remove_skybox = false;
	int     effects_visual_recoil_adjustment = 0;
	int     effects_transparent_walls = 100;
	int     effects_transparent_props = 100;
	int     effects_brightness_adjustment = 0;
	int     effects_night_adjustment = 0;
	bool	effects_remove_scope_overlay = false;
	bool	effects_instant_scope = false;
	bool	effects_disable_post_processing = false;
	bool	effects_force_third_person_alive = false;
	c_keybind effects_force_third_person_key = c_keybind{ 0, 0, false, 0 };
	bool	effects_force_third_person_dead = false;
	bool	effects_disable_rendering_of_teammates = false;
	bool	effects_bullet_tracers = false;
	float   effects_bullet_tracers_color[4] = { 1.f, 1.f, 1.f, 1.f };
	bool	effects_bullet_impact = false;
	bool	effects_override_skybox = false;
	float   effects_override_skybox_color[4] = { 1.f, 1.f, 1.f, 1.f };
	int	    effects_aspect_ratio = 0;
	int	    effects_force_third_person_distance = 0;




	int		miscellaneous_override_fov = 90;
	int		miscellaneous_override_zoom_fov = 100;
	bool	miscellaneous_knifebot = false;
	std::vector <int> miscellaneous_knifebot_knifebot_settings;
	bool	miscellaneous_zeusbot = false;
	bool	miscellaneous_reveal_completitive_ranks = false;
	bool	miscellaneous_reveal_overwatch_players = false;
	bool	miscellaneous_clan_tag_spammer = false;
	bool	miscellaneous_log_weapon_purchases = false;
	bool	miscellaneous_log_damage_dealt = false;
	bool	miscellaneous_persistent_kill_feed = false;
	bool	miscellaneous_disable_sv_pure = false;
	bool	miscellaneous_unlock_hidden_cvars = false;


	bool	movement_infinite_duck = false;
	bool	movement_easy_strafe = false;
	bool	movement_bunny_hop = false;
	bool	movement_air_strafe = false;
	std::vector <int> movement_air_strafe_direction;
	int movement_air_strafe_smoothing = 0;
	int     movement_air_duck = 0;
	bool	movement_jump_at_edge = false;
	c_keybind movement_jump_at_edge_key = c_keybind{ 0, 0, false, 0 };

	c_keybind settings_menu_key = c_keybind{ 0, 0, false, 0 };
	int      settings_dpi_scale = 0;
	bool     settings_anti_untrusted = false;
	bool     settings_hide_from_obs = false;
	bool     settings_low_fps_warning = false;
	bool     settings_lock_menu_layout = false;

	float menu_color[4] = { 1.f, 1.f, 1.f, 1.f };
	std::vector <std::string> scripts;
	//c_keybind visuals_tp_key = c_keybind{ 0, 0, false, 0 };

	struct Aimbot {
		bool enabled{ false };
		bool onKey{ false };
		int key{ 0 };
		int keyMode{ 0 };
		bool aimlock{ false };
		bool autodelay{ false };
		bool silent{ false };
		bool friendlyFire{ false };
		bool visibleOnly{ false };
		bool scopedOnly{ false };
		bool ignoreFlash{ false };
		bool ignoreSmoke{ false };
		bool autoShot{ false };
		bool autoScope{ false };
		float fov{ 0.0f };
		float smooth{ 1.0f };
		int bone{ 0 };
		float maxAimInaccuracy{ 1.0f };
		float maxShotInaccuracy{ 1.0f };
		int minDamage{ 1 };
		bool killshot{ false };
		bool betweenShots{ false };
		float killdelay{ 0.0f };
		float rcsfov{ 0.0f };
		float silentfov{ 0.0f };
	};
	std::array<Aimbot, 40> aimbot;
	int	selected_config = 0;

	std::string new_config_name;
};
struct skinInfo
{
	std::string name;
	std::string cdnName;
	std::vector<std::string> weaponName;
	int rarity;
};
class inv_fix
{
public:
	struct xd
	{
		std::unordered_map<int, skinInfo> skinInfo;
		int itemCount = 0;
		bool enabled = false;
	} inventory;
};
struct stickers_oops
{
	std::string name;
	int paintkit;
};

inline std::map<int, stickers_oops> g_Stickers;
inline inv_fix _inv;
class c_csgo
{
public:
	//interfaces
	c_interface_base<IBaseClientDLL>		m_client;
	c_interface_base<ISurface>				m_surface;
	c_interface_base<IClientEntityList>		m_entity_list;
	c_interface_base<IGameMovement>			m_movement;
	c_interface_base<IPrediction>			m_prediction;
	c_interface_base<IMoveHelper>			m_move_helper;
	c_interface_base<IInput>				m_input;
	c_interface_base<CGlobalVarsBase>		m_globals;
	c_interface_base<IVEngineClient>		m_engine;
	c_interface_base<IPanel>				m_panel;
	c_interface_base<IEngineVGui>			m_engine_vgui;
	c_interface_base<CClientState>			m_client_state;
	c_interface_base<ICvar>					m_engine_cvars;
	c_interface_base<IEngineTrace>			m_engine_trace;
	c_interface_base<IVModelInfo>			m_model_info;
	c_interface_base<CCSGameRules>          m_game_rules;
	c_interface_base<InputSystem>			m_input_system;
	c_interface_base<IMaterialSystem>       m_material_system;
	c_interface_base<IVModelRender>			m_model_render;
	c_interface_base<StudioRender>			m_studio_render;
	c_interface_base<IVRenderView>			m_render_view;
	c_interface_base<IPhysicsSurfaceProps>  m_phys_props;//IVDebugOverlay
	c_interface_base<IVDebugOverlay>        m_debug_overlay;
	c_interface_base<IGameEventManager>     m_event_manager;
	c_interface_base<IViewRenderBeams>		m_beams;
	c_interface_base<ILocalize>				m_localize;
	c_interface_base<IBaseFileSystem>		m_filesystem;
	c_interface_base<CGlowObjectManager>	m_glow_object;
	c_interface_base<IMDLCache>             m_mdl_cache;
	c_interface_base<IStaticPropMgr>		m_static_prop;
	c_interface_base<IEngineSound>          m_engine_sound;
	c_interface_base<C_PlayerResource>      m_player_resource;
	c_interface_base< IMemAlloc >           m_mem_alloc;
	c_interface_base< IPhysicsCollision >           m_physcollision;

	/*
		c_interface_base< c_base_client >            m_client;
        c_interface_base< c_client_mode >            m_client_mode;
        c_interface_base< c_entity_list >            m_entity_list;
        c_interface_base< c_var >                    m_engine_cvars;
        c_interface_base< CEngineClient >            m_engine;
        c_interface_base< CEngineTrace >             m_engine_trace;
        c_interface_base< c_global_vars >            m_globals;
        c_interface_base< IVModelInfoClient >        m_model_info;
        c_interface_base< ISurface >                 m_surface;
        c_interface_base< VPanel >                   m_panel;
        c_interface_base< c_input >                  m_input;
        c_interface_base< IGameMovement >            m_movement;
        c_interface_base< IPrediction >              m_prediction;
        c_interface_base< IMaterialSystem >          m_material_system;
        c_interface_base< IVRenderView >             m_render_view;
        c_interface_base< IVModelRender >            m_model_render;
        c_interface_base< c_physics_props >          m_phys_props;
        c_interface_base< c_client_state >           m_client_state;
        c_interface_base< c_localize >               m_localize;
        c_interface_base< IGameEventManager2 >       m_event_manager;
        c_interface_base< void >                     m_event_manager1;
        c_interface_base< IMDLCache >                m_mdl_cache;
        c_interface_base< void >                     m_hdn;
        c_interface_base< void >                     m_file_system;
        c_interface_base< IViewRenderBeams >         m_beams;
        c_interface_base< c_debug_overlay >          m_debug_overlay;
        c_interface_base< CEngineVGui >              m_engine_vgui;
        c_interface_base< engine_sound >             m_engine_sound;
        c_interface_base< c_move_helper >            m_move_helper;
        c_interface_base< void >                     m_soundservices;
        c_interface_base< mem_alloc >                m_mem_alloc;
	*/
};

class c_vmthooks
{
public:
	////interfaces
	//c_interface_base<IBaseClientDLL>		m_client;
	//c_interface_base<ISurface>				m_surface;
	//c_interface_base<IClientEntityList>		m_entity_list;
	//c_interface_base<IGameMovement>			m_movement;
	//c_interface_base<IPrediction>			m_prediction;
	//c_interface_base<IMoveHelper>			m_move_helper;
	//c_interface_base<IInput>				m_input;
	//c_interface_base<CGlobalVarsBase>		m_globals;
	//c_interface_base<IVEngineClient>		m_engine;
	//c_interface_base<IPanel>				m_panel;
	//c_interface_base<IEngineVGui>			m_engine_vgui;
	//c_interface_base<CClientState>			m_client_state;
	//c_interface_base<ICvar>					m_engine_cvars;
	//c_interface_base<IEngineTrace>			m_engine_trace;
	//c_interface_base<IVModelInfo>			m_model_info;
	//c_interface_base<CCSGameRules>          m_game_rules;
	//c_interface_base<InputSystem>			m_input_system;
	//c_interface_base<IMaterialSystem>       m_material_system;
	//c_interface_base<IVModelRender>			m_model_render;
	//c_interface_base<IVRenderView>			m_render_view;
	//c_interface_base<IPhysicsSurfaceProps>  m_phys_props;//IVDebugOverlay
	//c_interface_base<IVDebugOverlay>        m_debug_overlay;
	//c_interface_base<IGameEventManager>     m_event_manager;
	//c_interface_base<IViewRenderBeams>		m_beams;
	//c_interface_base<ILocalize>				m_localize;
	//c_interface_base<CGlowObjectManager>	m_glow_object;
	//c_interface_base<IMDLCache>             m_mdl_cache;
	//c_interface_base<IStaticPropMgr>		m_static_prop;
	//c_interface_base<IEngineSound>          m_engine_sound;
	//c_interface_base<C_PlayerResource>      m_player_resource;
	//c_interface_base< IMemAlloc >           m_mem_alloc;
	//c_interface_base< IPhysicsCollision >           m_physcollision;

	Memory::VmtSwap::Shared m_material_system;
	Memory::VmtSwap::Shared m_material;
	Memory::VmtSwap::Shared m_engine_trace;
	Memory::VmtSwap::Shared m_bsp_tree_query;
	Memory::VmtSwap::Shared m_client;
	Memory::VmtSwap::Shared m_clientstate;
	Memory::VmtSwap::Shared m_clientmode;
	Memory::VmtSwap::Shared m_surface;
	Memory::VmtSwap::Shared m_prediction;
	Memory::VmtSwap::Shared m_movement;
	Memory::VmtSwap::Shared m_panel;
	Memory::VmtSwap::Shared m_render_view;
	Memory::VmtSwap::Shared m_engine;
	Memory::VmtSwap::Shared m_fire_bullets;
	Memory::VmtSwap::Shared m_engine_vgui;
	Memory::VmtSwap::Shared m_model_render;
	Memory::VmtSwap::Shared m_net_channel;
	Memory::VmtSwap::Shared m_show_impacts;
	Memory::VmtSwap::Shared m_device;
	Memory::VmtSwap::Shared m_cl_clock_correction;
	Memory::VmtSwap::Shared m_cl_grenadepreview;
	Memory::VmtSwap::Shared cl_smooth;
	Memory::VmtSwap::Shared m_engine_sound;
};

static DWORD client = 0;
static DWORD m_surface;
static DWORD m_entity_list;
static DWORD m_movement;
static DWORD m_prediction;
static DWORD m_move_helper;
static DWORD m_input;
static DWORD m_globals;
static DWORD m_engine;
static DWORD m_panel;
static DWORD m_engine_vgui;
static DWORD m_client_state;
static DWORD m_engine_cvars;
static DWORD m_engine_trace;
static DWORD m_model_info;
static DWORD m_game_rules;
static DWORD m_input_system;
static DWORD m_material_system;
static DWORD m_model_render;
static DWORD m_render_view;
static DWORD m_phys_props;//IVDebugOverlay
static DWORD m_debug_overlay;
static DWORD m_event_manager;
static DWORD m_beams;
static DWORD m_localize;
static DWORD m_glow_object;
static DWORD m_mdl_cache;
static DWORD m_static_prop;

extern c_csgo	 csgo;
extern c_vmthooks	 vmt;

class C_WeaponCSBaseGun;
class C_BasePlayer;

class CCSGO_HudDeathNotice;

#define ANGLES_TOTAL 4
#define ANGLE_POSDELTA 3
#define ANGLE_SHOOTANGLE 2
#define ANGLE_FAKE 1
#define ANGLE_REAL 0

enum cycle_update_flags
{
	CYCLE_NONE = 0,
	CYCLE_PRE_UPDATE = 1 << 1,
	CYCLE_UPDATE = 1 << 2
};

enum fakeduck_flags
{
	FD_NONE = 0,
	FD_NEED_A_FIX = 1 << 2,
};

enum packet_flags
{
	PACKET_NONE = 0,
	PACKET_CHOKE = 1 << 1,
	PACKET_SEND = 1 << 2
};

struct timing_data
{
	timing_data(int _t, int _s, int _cm, int _tc)
	{
		tickbase = _t;
		shifted_shit = _s;
		cmd_num = _cm;
		tick_count = _tc;
	}
	int tickbase;
	int shifted_shit;
	int cmd_num;
	int tick_count;
};

struct c_keybindinfo
{
	c_keybindinfo()
	{
		index = 0;
		sort_index = 0;
		mode = 0;
		prev_state = false;
		name = "";
		type = "";
	}

	c_keybindinfo(int _i, const char* _n, const char* _t, int _si)
	{
		index = _i;
		name = _n;
		type = _t;
		sort_index = _si;
	}
	int index;
	int mode;
	const char* name;
	const char* type;
	bool prev_state;
	int sort_index;
};

struct s_local_damage
{
	s_local_damage(int _d, float _t)
	{
		time = _t;
		damage = _d;
	}
	s_local_damage()
	{
		damage = 0;
		time = 0.f;
	}
	int damage;
	float time;
};

struct multipoint_info_s
{
	Vector point;
	int damage;
};

struct shot_pre_event_info_s
{
	Vector point;
	bool hit;
	bool find;
	int entindex;
	int resolver_index;
	float last_time_hit;
};

struct COutgoingData {
	int command_nr;
	int prev_command_nr;

	bool is_outgoing;
	bool is_used;
};

enum m_eflags
{
	hook_should_return_cl_smooth = 0x4,
};

struct server_delay_s
{
	int tick_count;
	int cmd_num;
};

class c_context
{
public:
	c_variables m_settings;
	_MANUAL_INJECTEX32* data;
	FORCEINLINE C_BasePlayer* m_local()
	{
		//if (!csgo.m_engine()->IsInGame())
		//	return nullptr;
		
		auto local_player = csgo.m_engine()->GetLocalPlayer();

		//if (client == nullptr || *(void**)client == nullptr || !client->IsPlayer())
		//	return nullptr;

		return csgo.m_entity_list()->GetClientEntity(local_player);
	}
	FORCEINLINE Color flt2color(float c[])
	{
		return Color(c[0] * 255, c[1] * 255, c[2] * 255, c[3] * 255);
	}
	/*FORCEINLINE*/ CClientEffectRegistration* m_effects_head()
	{
		static CClientEffectRegistration* effcts = **reinterpret_cast<CClientEffectRegistration* **>(Memory::Scan(
			sxor("client.dll"), sxor("8B 35 ? ? ? ? 85 F6 0F 84 ? ? ? ? 0F 1F ? 8B 3E")) + 2);
		return effcts;
	}
	std::vector<std::string> get_all_files_names_within_folder(std::string folder, std::string fmt = "*.2k17")
	{
		std::vector<std::string> names = {};
		char search_path[200] = { 0 };
		sprintf_s(search_path, "%s/%s", folder.c_str(), fmt.c_str());
		WIN32_FIND_DATAA fd;
		HANDLE hFind = ::FindFirstFileA(search_path, &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				// read all (real) files in current folder
				// , delete '!' read other 2 default folder . and ..
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					names.emplace_back(fd.cFileName);
				}
			} while (::FindNextFileA(hFind, &fd));
			::FindClose(hFind);
		}
		return names;

	}
	bool pressed_keys[256] = {};
	Vector2D screen_size = Vector2D(0, 0);
	int side = -1; //manual anti-aim side
	int fside = -1; //anti-aim fake yaw side
	int previous_buttons = 0; 
	Vector old_move = Vector::Zero;
	bool pressed_before = false;
	bool fakeducking = false;
	bool fakeducking_prev_state = false;
	int fakeduck_will_choke = 0;
	bool in_tp = false;
	float fov = 0.f;
	float hurt_time = 0.f;
	QAngle last_angles = QAngle::Zero;
	CUserCmd* last_usercmd;
	bool setup_bones = false;
	bool allow_attachment_helper = false;
	bool start_switching = false;
	float lerp_time = 0.f;
	bool applied_tickbase = false;
	float latency[MAX_FLOWS] = { 0.f , 0.f };
	float avglatency[MAX_FLOWS] = { 0.f , 0.f };
	bool updating_anims = false;
	bool updating_resolver = false;
	bool force_unload = false;
	int last_penetrated_count = 4;
	int last_hitgroup = -1;
	float angles[ANGLES_TOTAL] = { 0.f, 0.f, 0.f, 0.f };
	Vector abs_origin[ANGLES_TOTAL] = { Vector::Zero, Vector::Zero, Vector::Zero, Vector::Zero };

	Vector local_origin[128] = { Vector::Zero, Vector::Zero, Vector::Zero, Vector::Zero };
	Vector desync_origin[128] = { Vector::Zero, Vector::Zero, Vector::Zero, Vector::Zero };

	std::array<float, 24> poses[ANGLES_TOTAL];
	//C_AnimationLayer layers[ANGLES_TOTAL][14];
	matrix3x4_t fake_matrix[128];
	//matrix3x4_t zero_matrix[128];
	server_delay_s m_arr_latency[150];
	std::array< std::array<std::vector<Vector>, 20u>, 64u> points;
	bool do_autostop = false;
	int onshot_desync = 0;
	bool allows_aimbot = false;
	int onshot_aa_cmd = 0;
	int original_model_index = 0;
	void* update_hud_weapons = nullptr;
	CCSGO_HudDeathNotice* hud_death_notice = nullptr;
	CUserCmd* last_predicted_command = nullptr;
	//std::deque<_shotinfo> fired_shot;
	int update_cycle = CYCLE_NONE;
	bool was_fakeducking_before = false;
	bool changed_fake_side = false;
	int last_frame_stage = 0;
	//bool fix_velocity_modifier = false;
	//float old_velocity_modifier = 0.f;
	bool is_predicting = false;
	std::deque<timing_data> m_corrections_data;
	float last_velocity_modifier = -1.f;
	int last_velocity_modifier_tick = -1;
	bool g_bOverrideModifier = false;
	int last_velocity_modifier_update_tick = -1;
	bool fix_modify_eye_pos = false;
	bool fix_runcommand = false;
	bool can_aimbot = true;
	float max_weapon_speed;
	int last_command_number = -1;
	int last_time_command_arrived = -1;
	int last_cmd_delta = 0;
	std::deque<int> last_4_deltas;
	int ticks_allowed = 0;
	int out_sequence_nr;
	bool skip_communication = false;
	bool did_communicate = false;
	bool can_call_senddatagram = false;
	int did_recharge = false;
	bool can_store_netvars = false;
	bool run_cmd_got_called = false;
	int last_netvars_update_tick = -1;
	int send_next_tick = PACKET_NONE;
	bool left_side = false;
	float last_shot_time_clientside = 0.f;
	bool autopeek_back = false;
	bool is_in_teleport = false;
	float fps = 0.f;
	bool boost_fps = false;
	int scroll_value = 0;
	bool is_local_defusing = false;
	int shift_amount = 0;
	bool doubletap_now = false;
	bool doubletap_charged = 0;
	int last_sent_tick = 0;
	bool buy_weapons = false;
	QAngle shot_angles = QAngle::Zero;
	bool did_shot = false;
	CCSGOPlayerAnimState fake_state;
	bool in_hbp = false;
	int next_shift_amount = 0;
	int shifting_amount = 0;
	int shifted_teleport_amount = 0;
	int double_tapped = 0;
	bool return_ishltv = false;
	bool force_next_packet_choke = false;
	int speed_hack = 0;
	int allow_shooting = 0;
	int charged_commands = 0;
	int charged_tickbase = 0;
	bool is_able_to_shoot = false;
	int started_speedhack = 0;
	int speedhack_choke = 0;
	int shifted_cmd = 0;
	bool next_run_cmd_fix_tickbase = false;
	int max_shift_cmd = 0;
	int tickbase_started_teleport = 0;
	bool speed_hacking = false;
	bool was_teleporting = false;
	bool has_scar = false;
	int estimated_shift_amount = 0;
	bool init_finished = false;
	C_AnimationLayer local_layers[4][14];
	matrix3x4_t local_matrix[128];
	int original_tickbase = 0;
	bool did_set_shift = false;
	int exploit_tickbase_shift = 0;
	int fixed_tickbase_backtrack = 0;
	int tickbase_shift_nr = 0;
	int fixed_tickbase_time = 0;
	bool exploit_allowed = false;
	std::deque<COutgoingData> command_numbers = {};
	int fix_senddatagram[150] = {};
	int shifted_command[150] = { 0 };
	bool in_send_datagram = false;
	bool air_stuck = false;
	float last_speedhack_time = 0.f;
	int last_aim_index = 0;
	int last_aim_state = 0;
	std::vector<std::string> knifes;
	std::vector<std::string> skins;
	bool updated_skin = false;
	int knife_model_index = 0;
	std::vector<C_BasePlayer*> m_player_entities;
	//bool can_hit[65] = {};
	//matrix3x4_t matrix[128];
	int m_ragebot_shot_nr = 0;
	int m_last_shot_index = 0;
	float m_last_shot_time = 0;
	matrix3x4_t* last_rage_matrix;
	int force_aimbot = 0;
	QAngle m_ragebot_shot_ang = QAngle(0,0,0);
	//std::vector<std::string> lua_scripts_count;
	float time_to_reset_sound = 0.f;
	std::vector<std::string> music_found;
	bool sound_valid = false;
	//std::vector<multipoint_info_s> multi_points[64][20] = {};
	float local_spawntime;
	c_keybindinfo active_keybinds[15] = { };
	int active_keybinds_visible = 0;
	float last_shot_time_fakeduck;
	bool is_updating_fake = false;
	int current_tickcount = 0;
	int previous_tickcount = 0;
	int tickrate = 0;
	int cmd_tickcount = 0;
	CUserCmd* cmd;
	bool did_stop_before = false;
	int last_autostop_tick = 0;
	int accurate_max_previous_chocked_amt = 0;
	float current_realtime = 0.f;
	bool should_rotate_camera = false;
	Vector m_eye_position = Vector::Zero;
	Vector m_pred_eye_pos = Vector::Zero;
	int command_number = 0;
	//int prediction_tickbase = 0;
	ConVar* clantag_cvar = nullptr;
	ConVar* cv_console_window_open = NULL;
	//Vector abs_origin = Vector::Zero;
	bool force_hitbox_penetration_accuracy = false;
	weapon_info* latest_weapon_data = nullptr;
	bool force_low_quality_autowalling = false;
	Vector auto_peek_spot = Vector::Zero;
	bool breaks_lc = false;
	s_local_damage local_damage[64] = {};
	bool first_run_since_init[65] = {};
	float current_spread = 0.0f;
	bool process_movement_sound_fix = false;
	bool optimized_point_search = false;
	bool allow_freestanding = false;
	bool is_charging = false;
	float last_time_charged = 0.f;
	bool is_cocking = false;
	float r8_timer = 0.f;
	QAngle cmd_original_angles;
	int cmd_original_buttons = 0;
	bool has_exploit_toggled = false;
	shot_pre_event_info_s last_shot_info;
	int main_exploit = 0;
	bool prev_exploit_states[2] = { false, false };
	int cheat_option_flags = 0;
	int ticks_to_stop = 0;
	float last_time_layers_fixed = 0.f;

	bool hold_aim = false;
	QAngle hold_angles = QAngle::Zero;
	int hold_aim_ticks = 0;

	bool m_revolver_fire;
	int weapon = 0;

	bool bomb_planting = false;
	int bomb_site = 0;
	bool is_defuse = false;
	float plant_time = 0.f;
	//FORCEINLINE bool check_crc(std::string file, CRC32_t original)
	//{
	//	auto hFile = CreateFileA(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); // Open the DLL

	//	if (hFile == INVALID_HANDLE_VALUE)
	//		return 0;

	//	auto FileSize = GetFileSize(hFile, NULL);
	//	auto buffer = VirtualAlloc(NULL, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	//	if (!buffer)
	//	{
	//		CloseHandle(hFile);
	//		return 0;
	//	}

	//	// Read the DLL
	//	DWORD read;

	//	if (!ReadFile(hFile, buffer, FileSize, &read, NULL))
	//	{
	//		VirtualFree(buffer, 0, MEM_RELEASE);
	//		CloseHandle(hFile);

	//		return 0;
	//	}

	//	CloseHandle(hFile);

	//	CRC32_t crc;

	//	CRC32_Init(&crc);
	//	CRC32_ProcessBuffer(&crc, &buffer, sizeof(unsigned char) * FileSize);
	//	CRC32_Final(&crc);

	//	VirtualFree(buffer, 0, MEM_RELEASE);
	//	CloseHandle(hFile);

	//	return (crc == original);
	//}
	/*FORCEINLINE*/ bool get_key_press(int key, int zticks = 1) const
	{
		static int ticks[256];
		bool returnvalue = false;



		if (pressed_keys[key] && !csgo.m_client()->IsChatRaised() && ((DWORD)cv_console_window_open < 0x200 || cv_console_window_open->GetInt() == 0))
		{
			ticks[key]++;

			if (ticks[key] <= zticks)
			{
				returnvalue = true;
			}
		}
		else
			ticks[key] = 0;

		return returnvalue;
	}
	/*FORCEINLINE*/ bool get_key_press(c_keybind &key, int zticks = 1) const
	{
		bool returnvalue = false;

		if (key.key > 255 || key.mode > 5)
			return false;

		if (key.mode == 3 && key.key <= 0)
			return true;

		if (key.key <= 0)
			return false;

		switch (key.mode)
		{
		case 0:
			if (!csgo.m_client()->IsChatRaised() && ((DWORD)cv_console_window_open < 0x200 || cv_console_window_open->GetInt() == 0) && pressed_keys[key.key])
			{
				key.time++;

				if (key.time <= zticks) {
					key.toggled = !key.toggled;
				}
			}
			else
				key.time = 0;

			returnvalue = key.toggled;
			break;
		case 1:

			if (csgo.m_client()->IsChatRaised() || (DWORD)cv_console_window_open > 0x200 && cv_console_window_open->GetInt() == 1)
				return false;

			returnvalue = pressed_keys[key.key];
			
			break;
		case 2:

			if (csgo.m_client()->IsChatRaised() || (DWORD)cv_console_window_open > 0x200 && cv_console_window_open->GetInt() == 1)
				return false;

			returnvalue = !pressed_keys[key.key];

			break;
		case 3:
			if (key.key != 0)
				returnvalue = true;
			break;
		}

		return returnvalue;
	}
	int host_frameticks()
	{
		static auto host_frameticks = *(int**)(Memory::Scan("engine.dll", "03 05 ? ? ? ? 83 CF 10") + 2);

		if (host_frameticks)
			return *host_frameticks;
		else
			return 1;
	}
	const char* base64_encode(const std::string& in)
	{
		std::string out = "";

		int val = 0, valb = -6;
		for (unsigned char c : in) {
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				out.push_back(sxor("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6) out.push_back(sxor("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")[((val << 8) >> (valb + 8)) & 0x3F]);
		while (out.size() % 4) out.push_back('=');
		return out.c_str();
	}
	std::string base64_decode(const std::string& in)
	{
		std::string out = "";

		std::vector<int> T(256, -1);
		for (int i = 0; i < 64; i++) T[sxor("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")[i]] = i;

		int val = 0, valb = -8;
		for (unsigned char c : in) {
			if (T[c] == -1) break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(char((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}
	void ForceTermination()
	{
		char buffer[256] = { 0 };
		GetModuleFileNameA(NULL, buffer, 256);


		int random1 = RandomInt(15, 40);
		DWORD random2 = RandomInt(0x11000000, 0x12000000);


		std::string msg = sxor(
			"This application has encountered a critical error:"
			"\n\n"
			"FATAL ERROR!"
			"\n\n"
			"Program: %s"
			"\n"
			"Exception:        0xC0000005 (ACCESS_VIOLATION) at 00%d:%X"
			"\n\n"
			"The instruction at '0x%X' referenced memory at '0x%X'."
			"\n"
			"The memory could not be 'read'."
			"\n\n"
			"Press OK to terminate the application."

		);

		char formatted[512] = { 0 };

		sprintf_s(formatted, 512, msg.c_str(), buffer, random1, random2, random2, random2);

		MessageBoxA(0, formatted, sxor("csgo.exe"), MB_OK | MB_ICONERROR);
		exit(0xffffff);
		terminate();
	}
	std::array<int,128u> shots_fired;
	std::array<int, 128u> shots_total;
	int padd[128];

};

#include <visuals/grenade_warning.h>

#include <visuals/chams_hit.h>

class c_menu;
class c_antiaimbot;
class c_misc;
class c_resolver;
class c_visuals;
class c_usercmd;
class c_lagcomp;
class c_chams;
class c_autowall;
class c_aimbot;
class c_dormant_esp;
class c_music_player;
class c_grenade_tracer;
class c_weather_controller;
class c_legitaimbot;
class c_visuals;
class c_hit_chams;
class c_grenade_prediction;
class c_grenade_tracer;
class c_net_channel;
namespace feature
{
	extern Encrypted_t<c_menu> menu;
	extern Encrypted_t< c_misc> misc;
	extern Encrypted_t < c_antiaimbot > anti_aim;
	extern Encrypted_t < c_resolver > resolver;
	extern Encrypted_t < c_visuals > visuals;
	extern Encrypted_t < c_usercmd > usercmd;
	extern Encrypted_t < c_lagcomp > lagcomp;
	extern Encrypted_t < c_chams > chams;
	extern Encrypted_t < c_autowall > autowall;
	extern Encrypted_t < c_aimbot > ragebot;
	extern Encrypted_t < c_dormant_esp > sound_parser;
	extern Encrypted_t < c_music_player > music_player;
	extern Encrypted_t < c_weather_controller > weather;
	extern Encrypted_t < c_legitaimbot > legitbot;
	extern Encrypted_t < c_hit_chams > hitchams;
	extern Encrypted_t < c_grenade_prediction > grenade_prediction;
	extern Encrypted_t < c_grenade_tracer > grenade_tracer;
	extern Encrypted_t < c_net_channel > net_channel;

	template <class T>
	T find_hud_element(const char* name)
	{
		static auto pThis = *reinterpret_cast<DWORD * *>(Memory::Scan("client.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

		static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Memory::Scan("client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));

		if (find_hud_element && pThis)
			return (T)find_hud_element(pThis, name);
		else
			return (T)nullptr;
	}
}

extern C_WeaponCSBaseGun* m_weapon();

extern c_context ctx;

#define MEMEBR_FUNC_ARGS(...) ( this, __VA_ARGS__ ); }
#define FUNCARGS(...) ( __VA_ARGS__ ); }
#define VFUNC( index, func, sig ) auto func { return util::get_vfunc< sig >( this, index ) MEMEBR_FUNC_ARGS
#define MFUNC(func, sig, offset) auto func { return reinterpret_cast< sig >( offset ) MEMEBR_FUNC_ARGS
#define FUNC(func, sig, offset) auto func { return reinterpret_cast< sig >( offset ) FUNCARGS

inline uint8_t* find_sig_ext(uint32_t offset, const char* signature, uint32_t range = 0u)
{
	static auto pattern_to_bytes = [](const char* pattern) -> std::vector<int>
	{
		auto bytes = std::vector<int32_t>{};
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current)
		{
			if (*current == '?')
			{
				current++;

				if (*current == '?')
					current++;

				bytes.push_back(-1);
			}
			else
				bytes.push_back(static_cast<int32_t>(strtoul(current, &current, 0x10)));
		}

		return bytes;
	};

	const auto scan_bytes = reinterpret_cast<std::uint8_t*>(offset);
	auto pattern_bytes = pattern_to_bytes(signature);
	const auto s = pattern_bytes.size();
	const auto d = pattern_bytes.data();

	for (auto i = 0ul; i < range - s; ++i)
	{
		auto found = true;

		for (auto j = 0ul; j < s; ++j)
			if (scan_bytes[i + j] != d[j] && d[j] != -1)
			{
				found = false;
				break;
			}

		if (found)
			return &scan_bytes[i];
	}

	return nullptr;
}

__forceinline void erase_function(uint8_t * function)
{
	if (*function == 0xE9)
	{
		auto pdFollow = (PDWORD)(function + 1);
		function = ((PBYTE)(*pdFollow + (DWORD)function + 5));
	}
	else if (*function == 0xEB)
	{
		auto pbFollow = (PDWORD)(function + 1);
		function = ((PBYTE)((DWORD)* pbFollow + (DWORD)function + 2));
	}

	static const auto current_process = reinterpret_cast<HANDLE>(-1);

	const auto end = find_sig_ext(reinterpret_cast<uint32_t>(function), "90 90 90 90 90", 0x2000);
	size_t bytes = reinterpret_cast<DWORD>(end) - reinterpret_cast<DWORD>(function) + 6;

	void* fn = function;
	size_t size = bytes;
	DWORD old;
	VirtualProtect(fn, size, PAGE_EXECUTE_READWRITE, &old);
	fn = function;
	size = bytes;
	memset(fn, 0, size);
	VirtualProtect(fn, size, old, &old);
}

#define concat_impl(x, y) x##y
#define concat(x, y) concat_impl(x, y)

// NOLINTNEXTLINE
#define erase_fn(a) constexpr auto concat(w, __LINE__) = &a;\
    erase_function(reinterpret_cast<uint8_t*>((void*&)concat(w, __LINE__)))
#define erase_end  __asm _emit 0x90 __asm _emit 0x90 __asm _emit 0x90 __asm _emit 0x90 __asm _emit 0x90 

#include <parser.h>