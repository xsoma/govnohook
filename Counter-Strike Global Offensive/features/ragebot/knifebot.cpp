#include "rage_aimbot.hpp"
#include "source.hpp"
#include <sdk/props/entity.hpp>
#include <sdk/props/player.hpp>
#include <sdk/props/weapon.hpp>
#include <core/hooks/hooked.hpp>
#include <sdk/math/math.hpp>
#include <sdk/props/displacement.hpp>
#include "lag_comp.hpp"
#include <features/antiaim/anti_aimbot.hpp>
#include "resolver.hpp"
#include <features/visuals/visuals.hpp>
#include <menu/menu/menu.hpp>
#include <features/misc/movement.hpp>
#include "prediction.hpp"
#include "autowall.hpp"
#include <features/misc/misc.hpp>
#include "usercmd.hpp"



bool c_aimbot::knife(bool* send_packet, C_BasePlayer* m_entities, CUserCmd* cmd) {

	struct KnifeTarget_t { bool stab; QAngle angle; int entindex; };
	KnifeTarget_t target{};

	target.entindex = 0;

	if (!m_entities || !m_weapon()->is_knife())
		return false;

	C_BasePlayer* entity = m_entities;

	if (!entity ||
		entity->IsDormant() ||
		!entity->IsPlayer() ||
		entity->m_bGunGameImmunity() ||
		entity->m_iTeamNum() == ctx.m_local()->m_iTeamNum() ||
		entity->IsDead() ||
		!entity->GetClientClass())
	{
		return false;
	}

	const auto idx = entity->entindex() - 1;

	if (!entity->get_animation_state())
		return false;

	auto r_log = &feature::resolver->player_records[idx];

	auto data = &feature::lagcomp->records[idx];

	if (!data || data->player != entity || data->records_count < 1) {
		return false;
	}
	feature::lagcomp->records[idx].tick_count = -1;

	const float maxRange = ctx.latest_weapon_data ? ctx.latest_weapon_data->range : 3000;

	int iteration = 0;

	C_Tickrecord* ideal = feature::resolver->find_first_available(entity, data, false);

	// trace with last.
	for (const auto& a : m_knife_ang)
	{
		++iteration;
		// check if we can knife.
		if (!can_knife(entity, ideal, a, target.stab)) {
			continue;
		}

		// set target data.
		target.entindex = entity->entindex();
		target.angle = a;
		break;
	}
	if (target.entindex > 0) {
		C_BasePlayer* entity = csgo.m_entity_list()->GetClientEntity(target.entindex);
		c_player_records* log = &feature::lagcomp->records[target.entindex - 1];

		log->restore_record.apply(entity, true);

		// set target tick.
		if (log->best_record && log->best_record != nullptr)
			cmd->tick_count = TIME_TO_TICKS(log->best_record->simulation_time + ctx.lerp_time);

		// set view angles.
		cmd->viewangles = target.angle;

		// set attack1 or attack2.
		cmd->buttons |= target.stab ? IN_ATTACK2 : IN_ATTACK;

		return true;
	}

	return false;
}

bool c_aimbot::can_knife(C_BasePlayer* m_player, C_Tickrecord* record, QAngle angle, bool& stab) {
	// convert target angle to direction.
	Vector forward;
	Math::AngleVectors(angle, &forward);

	// see if we can hit the player with full range
	// this means no stab.
	CGameTrace trace;
	knife_trace(forward, false, &trace);

	// we hit smthing else than we were looking for.
	if (!trace.m_pEnt || trace.m_pEnt != m_player)
		return false;

	bool armor = m_player->m_ArmorValue() > 0;
	bool first = m_weapon()->m_flNextPrimaryAttack() + 0.4f < TICKS_TO_TIME(ctx.fixed_tickbase_backtrack);
	bool back = knife_is_behind(record);

	int stab_dmg = m_knife_dmg.stab[armor][back];
	int slash_dmg = m_knife_dmg.swing[first][armor][back];
	int swing_dmg = m_knife_dmg.swing[false][armor][back];

	// smart knifebot.
	int health = m_player->m_iHealth();
	if (health <= slash_dmg && ctx.m_settings.miscellaneous_knifebot_knifebot_settings.at(0))
		stab = false;

	else if (health <= stab_dmg && ctx.m_settings.miscellaneous_knifebot_knifebot_settings.at(1))
		stab = true;

	else if (health > (slash_dmg + swing_dmg + stab_dmg) && ctx.m_settings.miscellaneous_knifebot_knifebot_settings.at(1))
		stab = true;

	else
		stab = false;

	// damage wise a stab would be sufficient here.
	if (stab && !knife_trace(forward, true, &trace))
		return false;

	return true;
}

bool c_aimbot::knife_trace(Vector dir, bool stab, CGameTrace* trace) {
	float range = stab ? 32.f : 48.f;

	Vector start = ctx.m_eye_position;
	Vector end = start + (dir * range);

	CTraceFilter filter;
	filter.pSkip = ctx.m_local();
	Ray_t r; r.Init(start, end);
	csgo.m_engine_trace()->TraceRay(r, MASK_SOLID, &filter, trace);

	// if the above failed try a hull trace.
	if (trace->fraction >= 1.f) {
		r.Init(start, end, { -16.f, -16.f, -18.f }, { 16.f, 16.f, 18.f });
		csgo.m_engine_trace()->TraceRay(r, MASK_SOLID, &filter, trace);
		return trace->fraction < 1.f;
	}

	return true;
}

bool c_aimbot::knife_is_behind(C_Tickrecord* record) {
	Vector delta{ record->origin - ctx.m_eye_position };
	delta.z = 0.f;
	delta.Normalize();

	Vector target;
	Math::AngleVectors(record->eye_angles, &target);
	target.z = 0.f;

	return delta.Dot(target) > 0.475f;
}