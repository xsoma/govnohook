#pragma once
#include "sdk.hpp"
#include <deque>
#include "source.hpp"
constexpr int size_info = 8;

constexpr int dlinesize = 3, dlinedec = 5;
constexpr int linesize = 5, linedec = 11;
constexpr auto _hk = LIT(("HK"));
constexpr auto _k = LIT(("K"));
constexpr auto _h = LIT(("H"));
constexpr auto _r8 = LIT(("revolver"));
constexpr auto _z = LIT(("ZOOM"));
constexpr auto _pin = LIT(("PIN"));
constexpr auto _c4 = LIT(("C4"));
constexpr auto _vip = LIT(("VIP"));
constexpr auto _hs = LIT(("HEADSHOT"));
struct _event
{
	_event(const std::string& msg, const std::string& secretmsg = "")
	{
		_time = csgo.m_globals()->realtime + 4.f;
		_displayticks = 4.f;
		_msg = msg;
		_displayed = false;
		_secretmsg = secretmsg;
	}

	float _time = 0;
	float _displayticks = 0;
	bool _displayed = false;
	std::string _msg = "";
	std::string _secretmsg = "";
};

extern std::vector<_event> _events;
struct WorldHitmarkerData_t {
	float m_time, m_alpha;
	float m_pos_x, m_pos_y, m_pos_z;

	Vector m_world_pos;
	bool m_world_to_screen;
};

extern std::vector<WorldHitmarkerData_t> world_hitmarker;
class c_bullet_tracer
{
public:
	c_bullet_tracer(const Vector& _src, const Vector& _dst, const float& _time, const Color& _colorLine, const bool& _islocal)
	{
		src = _src;
		dst = _dst;
		time = _time;
		color1 = _colorLine;
		is_local = _islocal;
	}

	Vector src = Vector::Zero, dst = Vector::Zero;
	float time = 0.f;
	Color color1 = Color::White();
	bool is_local = false;
};

extern std::vector<c_bullet_tracer> bullet_tracers;

class c_damage_indicator
{
public:
	c_damage_indicator(const Vector spot, const int tick, const int damage, const bool is_headshot)
	{
		_tick = tick;
		_spot = spot;
		_damage = damage;
		_headshot = is_headshot;
		_time = csgo.m_globals()->realtime + 4.f;
		w2s = Vector::Zero;
	}

	Vector _spot;
	Vector w2s;
	int _damage;
	int _tick;
	float _time;
	bool _headshot;
};

extern std::vector<c_damage_indicator> damage_indicators;


class c_visuals
{
public:
	virtual bool get_espbox(C_BasePlayer* entity, int& x, int& y, int& w, int& h);
	virtual void damage_esp();
	virtual void logs();
	virtual void skeleton(C_BasePlayer* Entity, Color color, matrix3x4_t* pBoneToWorldOut);
	virtual void offscreen_esp(C_BasePlayer* player, float alpha);
	virtual void render(bool reset);
	virtual void other_visuals(bool reset);
	virtual void indicators(bool reset);
	virtual void draw_items(C_BasePlayer* m_entity);

	//float dormant_alpha[129];
	std::array<float, 128u> dormant_alpha = {};
	bool save_pos = false;
	int saved_x = 0;
	int saved_y = 0;
	bool was_moved = false;

	bool save_pos_hotkeys = false;
	int saved_x_hotkeys = 0;
	int saved_y_hotkeys = 0;
	bool was_moved_hotkeys = false;
	//const wchar_t* weapon_names[900] = {};
	std::array<const wchar_t*, 900u> weapon_names = {};

};