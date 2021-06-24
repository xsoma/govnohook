#pragma once

#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1	
#define DAMAGE_YES		2
#define DAMAGE_AIM		3

enum TextureCharacters {
	CHAR_TEX_ANTLION = 'A',
	CHAR_TEX_BLOODYFLESH = 'B',
	CHAR_TEX_CONCRETE = 'C',
	CHAR_TEX_DIRT = 'D',
	CHAR_TEX_EGGSHELL = 'E',
	CHAR_TEX_FLESH = 'F',
	CHAR_TEX_GRATE = 'G',
	CHAR_TEX_ALIENFLESH = 'H',
	CHAR_TEX_CLIP = 'I',
	CHAR_TEX_PLASTIC = 'L',
	CHAR_TEX_METAL = 'M',
	CHAR_TEX_SAND = 'N',
	CHAR_TEX_FOLIAGE = 'O',
	CHAR_TEX_COMPUTER = 'P',
	CHAR_TEX_SLOSH = 'S',
	CHAR_TEX_TILE = 'T',
	CHAR_TEX_CARDBOARD = 'U',
	CHAR_TEX_VENT = 'V',
	CHAR_TEX_WOOD = 'W',
	CHAR_TEX_GLASS = 'Y',
	CHAR_TEX_WARPSHIELD = 'Z',
};

#define CHAR_TEX_STEAM_PIPE		11

class c_autowall
{
public:
	//virtual void TraceLine(Vector& absStart, const Vector& absEnd, unsigned int mask, C_BasePlayer* ignore, CGameTrace* ptr);
	//virtual float ScaleDamage(C_BasePlayer* player, float damage, float armor_ratio, int hitgroup);
	//virtual void ScaleDamage(CGameTrace& enterTrace, weapon_info* weaponData, float& currentDamage);
	virtual uint32_t get_filter_simple_vtable();
	//virtual bool TraceToExit(const Vector& start, const Vector dir, Vector& out, trace_t* enter_trace, trace_t* exit_trace);
	//virtual bool HandleBulletPenetration(C_BasePlayer* ignore, weapon_info* weaponData, trace_t& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_bullet_penetration, bool pskip = false);
	//virtual void FixTraceRay(Vector end, Vector start, trace_t* oldtrace, C_BasePlayer* ent);
	//virtual void ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, trace_t* tr);
	//virtual int HitboxToHitgroup(C_BasePlayer* m_player, int ihitbox);
	//virtual bool FireBullet(Vector eyepos, C_WeaponCSBaseGun* pWeapon, Vector& direction, float& currentDamage, C_BasePlayer* ignore, C_BasePlayer* to_who = nullptr, int hitbox = -1, bool* was_viable = nullptr, std::vector<float>* = nullptr);
	//virtual float CanHit(Vector& vecEyePos, Vector& point);
	//virtual float CanHit(const Vector& vecEyePos, Vector& point, C_BasePlayer* ignore_ent, C_BasePlayer* start_ent, int hitbox, bool* was_viable = nullptr);
	//virtual float SimulateShot(Vector& vecEyePos, Vector& point, C_BasePlayer* ignore_ent, C_BasePlayer* to_who, bool* was_viable = nullptr);
};

#pragma once

namespace penetration {
	struct PenetrationInput_t
	{
		C_BasePlayer* m_from;
		C_BasePlayer* m_target;
		Vector  m_pos;
		float	m_damage;
		float   m_damage_pen;
		bool	m_can_pen;
	};

	struct FireBulletData_t
	{
		Vector m_start;
		Vector m_end;
		Vector m_current_position;
		Vector m_direction;

		CTraceFilter* m_filter;
		CGameTrace m_enter_trace;

		float m_thickness;
		float m_current_damage;
		int m_penetration_count;
	};

	struct PenetrationOutput_t {
		C_BasePlayer* m_target;
		float   m_damage;
		int     m_hitgroup;
		bool    m_pen;

		__forceinline PenetrationOutput_t() : m_target{ nullptr }, m_damage{ 0.f }, m_hitgroup{ -1 }, m_pen{ false } {}
	};

	bool HandleBulletPenetration(weapon_info* info, FireBulletData_t& data, bool extracheck, Vector point);
	void TraceLine(Vector& start, Vector& end, unsigned int mask, C_BaseEntity* ignore, CGameTrace* trace);
	bool CanHitFloatingPoint(const Vector& point, const Vector& source);

	float scale(C_BasePlayer* player, float damage, float armor_ratio, int hitgroup);
	bool  TraceToExit(const Vector& start, const Vector& dir, Vector& out, CGameTrace* enter_trace, CGameTrace* exit_trace);
	void  ClipTraceToPlayer(const Vector& start, const Vector& end, uint32_t mask, CGameTrace* tr, C_BasePlayer* player, float min);
	bool  run(PenetrationInput_t* in, PenetrationOutput_t* out);
}