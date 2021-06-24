#include "sdk.hpp"
#include <ragebot/autowall.hpp>
#include <ragebot/rage_aimbot.hpp>
#include "source.hpp"
#include <props/entity.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <hooks/hooked.hpp>
#include <sdk/math/math.hpp>
#include <props/displacement.hpp>

#include <thread>

#define SLOBYTE(x)   (*((int8_t*)&(x)))

float penetration::scale(C_BasePlayer* player, float damage, float armor_ratio, int hitgroup) {
	bool  has_heavy_armor;
	int   armor;
	float heavy_ratio, bonus_ratio, ratio, new_damage;

	static auto is_armored = [](C_BasePlayer* player, int armor, int hitgroup) {
		// the player has no armor.
		if (armor <= 0)
			return false;

		// if the hitgroup is head and the player has a helment, return true.
		// otherwise only return true if the hitgroup is not generic / legs / gear.
		if (hitgroup == HITGROUP_HEAD && player->m_bHasHelmet())
			return true;

		else if (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM)
			return true;

		return false;
	};

	// check if the player has heavy armor, this is only really used in operation stuff.
	has_heavy_armor = player->m_bHasHeavyArmor();

	// scale damage based on hitgroup.
	switch (hitgroup) {
	case HITGROUP_HEAD:
		if (has_heavy_armor)
			damage = (damage * 4.f) * 0.5f;
		else
			damage *= 4.f;
		break;

	case HITGROUP_STOMACH:
		damage *= 1.25f;
		break;

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		damage *= 0.75f;
		break;

	default:
		break;
	}

	// grab amount of player armor.
	armor = player->m_ArmorValue();

	// check if the ent is armored and scale damage based on armor.
	if (is_armored(player, armor, hitgroup)) {
		heavy_ratio = 1.f;
		bonus_ratio = 0.5f;
		ratio = armor_ratio * 0.5f;

		// player has heavy armor.
		if (has_heavy_armor) {
			// calculate ratio values.
			bonus_ratio = 0.33f;
			ratio = armor_ratio * 0.25f;
			heavy_ratio = 0.33f;

			// calculate new damage.
			new_damage = (damage * ratio) * 0.85f;
		}

		// no heavy armor, do normal damage calculation.
		else
			new_damage = damage * ratio;

		if (((damage - new_damage) * (heavy_ratio * bonus_ratio)) > armor)
			new_damage = damage - (armor / bonus_ratio);

		damage = new_damage;
	}

	return std::floor(damage);
}
void ClipTraceToPlayers(const Vector& start, const Vector& end, uint32_t mask, ITraceFilter* filter, trace_t* tr) {
	Vector     pos, to, dir, on_ray;
	float      len, range_along, range;
	//CGameTrace new_trace;

	float smallestFraction = tr->fraction;
	constexpr float maxRange = 60.0f;

	//Vector delta(vecAbsEnd - vecAbsStart);
	//const float delta_length = delta.Normalize();

	dir = start - end;
	len = dir.Normalize();

	Ray_t ray;
	ray.Init(start, end);

	for (int i = 1; i <= 64; ++i) {
		C_BasePlayer* ent = csgo.m_entity_list()->GetClientEntity(i);
		if (!ent || ent->IsDormant() || ent->IsDead())
			continue;

		if (filter && !filter->ShouldHitEntity(ent, mask))
			continue;

		// set some local vars.
		pos = ent->m_vecOrigin() + ((ent->OBBMins() + ent->OBBMaxs()) * 0.5f);
		to = pos - start;
		range_along = dir.Dot(to);

		// off start point.
		if (range_along < 0.f)
			range = -(to).Length();

		// off end point.
		else if (range_along > len)
			range = -(pos - end).Length();

		// within ray bounds.
		else {
			on_ray = start + (dir * range_along);
			range = (pos - on_ray).Length();
		}

		if (/*range > 0.0f && */range <= maxRange) {
			trace_t playerTrace;

			//ray.Init(start, end);

			csgo.m_engine_trace()->ClipRayToEntity(ray, 0x4600400B, ent, &playerTrace);

			if (playerTrace.fraction < smallestFraction) {
				// we shortened the ray - save off the trace
				*tr = playerTrace;
				smallestFraction = playerTrace.fraction;
			}
		}
	}
}

bool penetration::TraceToExit(const Vector& start, const Vector& dir, Vector& out, CGameTrace* enter_trace, CGameTrace* exit_trace) {
	static CTraceFilterSimple filter{};

	float  dist{};
	Vector new_end;
	int    contents, first_contents{};

	// max pen distance is 90 units.
	while (dist <= 90.f) {
		// step forward a bit.
		dist += 4.f;

		// set out pos.
		out = start + (dir * dist);

		if (!first_contents)
			first_contents = csgo.m_engine_trace()->GetPointContents(out, MASK_SHOT, nullptr);

		contents = csgo.m_engine_trace()->GetPointContents(out, MASK_SHOT, nullptr);

		if ((contents & MASK_SHOT_HULL) && (!(contents & CONTENTS_HITBOX) || (contents == first_contents)))
			continue;

		// move end pos a bit for tracing.
		new_end = out - (dir * 4.f);

		Ray_t ray1;

		ray1.Init(out, new_end);

		// do first trace.
		csgo.m_engine_trace()->TraceRay(ray1, MASK_SHOT, nullptr, exit_trace);

		// note - dex; this is some new stuff added sometime around late 2017 ( 10.31.2017 update? ).
		if (csgo.m_engine_cvars()->FindVar(sxor("sv_clip_penetration_traces_to_players"))->GetInt())
			ClipTraceToPlayers(out, new_end, MASK_SHOT, nullptr, exit_trace);

		// we hit an ent's hitbox, do another trace.
		if (exit_trace->startsolid && (exit_trace->surface.flags & SURF_HITBOX)) {
			filter.SetPassEntity(exit_trace->m_pEnt);

			Ray_t ray2;
			ray2.Init(out, start);

			csgo.m_engine_trace()->TraceRay(ray2, MASK_SHOT_HULL, (ITraceFilter*)&filter, exit_trace);

			if (exit_trace->DidHit() && !exit_trace->startsolid) {
				out = exit_trace->endpos;
				return true;
			}

			continue;
		}

		if (!exit_trace->DidHit() || exit_trace->startsolid) {
			if (enter_trace->m_pEnt->is_breakable()) {
				*exit_trace = *enter_trace;
				exit_trace->endpos = start + dir;
				return true;
			}

			continue;
		}

		if ((exit_trace->surface.flags & SURF_NODRAW)) {
			// note - dex; ok, when this happens the game seems to not ignore world?
			if (exit_trace->m_pEnt->is_breakable() && enter_trace->m_pEnt->is_breakable()) {
				out = exit_trace->endpos;
				return true;
			}

			if (!(enter_trace->surface.flags & SURF_NODRAW))
				continue;
		}

		if (exit_trace->plane.normal.Dot(dir) <= 1.f) {
			out -= (dir * (exit_trace->fraction * 4.f));
			return true;
		}
	}

	return false;
}

void penetration::ClipTraceToPlayer(const Vector& start, const Vector& end, uint32_t mask, CGameTrace* tr, C_BasePlayer* player, float min) {
	Vector     pos, to, dir, on_ray;
	float      len, range_along, range;
	Ray_t        ray;
	CGameTrace new_trace;

	// reference: https://github.com/alliedmodders/hl2sdk/blob/3957adff10fe20d38a62fa8c018340bf2618742b/game/shared/util_shared.h#L381

	// set some local vars.
	pos = player->m_vecOrigin() + ((player->OBBMins() + player->OBBMaxs()) * 0.5f);
	to = pos - start;
	dir = start - end;
	len = dir.Normalize();
	range_along = dir.Dot(to);

	// off start point.
	if (range_along < 0.f)
		range = -(to).Length();

	// off end point.
	else if (range_along > len)
		range = -(pos - end).Length();

	// within ray bounds.
	else {
		on_ray = start + (dir * range_along);
		range = (pos - on_ray).Length();
	}

	if ( /*min <= range &&*/ range <= 60.f) {
		// clip to player.

		Ray_t ray4;
		ray4.Init(start, end);
		csgo.m_engine_trace()->ClipRayToEntity(ray4, mask, player, &new_trace);

		if (tr->fraction > new_trace.fraction)
			*tr = new_trace;
	}
}
bool IsValidHitgroup(int index) {
	if ((index >= HITGROUP_HEAD && index <= HITGROUP_RIGHTLEG) || index == HITGROUP_GEAR)
		return true;

	return false;
}
bool penetration::run(PenetrationInput_t* in, PenetrationOutput_t* out) {
	static CTraceFilterSimple filter{};

	int			  pen{ 4 }, enter_material, exit_material;
	float		  damage, penetration, penetration_mod, player_damage, remaining, trace_len{}, total_pen_mod, damage_mod, modifier, damage_lost;
	surface_data_t* enter_surface, * exit_surface;
	bool		  nodraw, grate;
	Vector		  start, dir, end, pen_end;
	CGameTrace	  trace, exit_trace;
	C_BaseCombatWeapon* weapon;
	weapon_info* weapon_info;

	// if we are tracing from our local player perspective.
	if (in->m_from == ctx.m_local()) {
		weapon = ctx.m_local()->get_weapon();
		weapon_info = ctx.latest_weapon_data;
		start = ctx.m_local()->Weapon_ShootPosition();
	}

	// not local player.
	else {
		weapon = in->m_from->get_weapon();
		if (!weapon)
			return false;

		// get weapon info.
		weapon_info = weapon->GetWpnData();
		if (!weapon_info)
			return false;

		// set trace start.

		start = in->m_from->Weapon_ShootPosition();
	}

	// get some weapon data.
	damage = (float)weapon_info->damage;
	penetration = weapon_info->penetration;

	// used later in calculations.
	penetration_mod = max(0.f, (3.f / penetration) * 1.25f);

	// get direction to end point.
	dir = (in->m_pos - start).Normalized();

	// setup trace filter for later.
	filter.SetPassEntity(in->m_from);
	// filter.SetPassEntity2( nullptr );

	while (damage > 0.f) {
		// calculating remaining len.
		remaining = weapon_info->range - trace_len;

		// set trace end.
		end = start + (dir * remaining);

		// setup ray and trace.
		// TODO; use UTIL_TraceLineIgnoreTwoEntities?
		Ray_t ray3;
		ray3.Init(start, end);

		csgo.m_engine_trace()->TraceRay(ray3, MASK_SHOT | CONTENTS_HITBOX, (ITraceFilter*)&filter, &trace);

		// we didn't hit anything.
		if (trace.fraction == 1.f)
			return false;

		// check for player hitboxes extending outside their collision bounds.
		// if no target is passed we clip the trace to a specific player, otherwise we clip the trace to any player.
		if (in->m_target)
			ClipTraceToPlayer(start, end + (dir * 40.f), MASK_SHOT | CONTENTS_HITBOX, &trace, in->m_target, -60.f);
		else
			ClipTraceToPlayers(start, end + (dir * 40.f), MASK_SHOT | CONTENTS_HITBOX, (ITraceFilter*)&filter, &trace);

		// calculate damage based on the distance the bullet traveled.
		trace_len += trace.fraction * remaining;
		damage *= std::pow(weapon_info->range_modifier, trace_len / 500.f);

		// if a target was passed.
		if (in->m_target) {

			// validate that we hit the target we aimed for.
			if (trace.m_pEnt && trace.m_pEnt == in->m_target && IsValidHitgroup(trace.hitgroup)) {
				int group = (weapon->m_iItemDefinitionIndex() == WEAPON_TASER) ? HITGROUP_GENERIC : trace.hitgroup;

				// scale damage based on the hitgroup we hit.
				player_damage = scale(in->m_target, damage, weapon_info->armor_ratio, group);

				// set result data for when we hit a player.
				out->m_pen = pen != 4;
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = in->m_target;

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}
		}

		// no target was passed, check for any player hit or just get final damage done.
		else {
			out->m_pen = pen != 4;

			// todo - dex; team checks / other checks / etc.
			if (trace.m_pEnt && trace.m_pEnt->IsPlayer() && IsValidHitgroup(trace.hitgroup)) {
				int group = (weapon->m_iItemDefinitionIndex() == WEAPON_TASER) ? HITGROUP_GENERIC : trace.hitgroup;

				player_damage = scale((C_BasePlayer*)trace.m_pEnt, damage, weapon_info->armor_ratio, group);

				// set result data for when we hit a player.
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = (C_BasePlayer*)trace.m_pEnt;

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}

			// if we've reached here then we didn't hit a player yet, set damage and hitgroup.
			out->m_damage = damage;
		}

		// don't run pen code if it's not wanted.
		if (!in->m_can_pen)
			return false;

		// get surface at entry point.
		enter_surface = csgo.m_phys_props()->GetSurfaceData(trace.surface.surfaceProps);

		// this happens when we're too far away from a surface and can penetrate walls or the surface's pen modifier is too low.
		if ((trace_len > 3000.f && penetration) || enter_surface->game.penetrationmodifier < 0.1f)
			return false;

		// store data about surface flags / contents.
		nodraw = (trace.surface.flags & SURF_NODRAW);
		grate = (trace.contents & CONTENTS_GRATE);

		// get material at entry point.
		enter_material = enter_surface->game.material;

		// note - dex; some extra stuff the game does.
		if (!pen && !nodraw && !grate && enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS)
			return false;

		// no more pen.
		if (penetration <= 0.f || pen <= 0)
			return false;

		// try to penetrate object.
		if (!TraceToExit(trace.endpos, dir, pen_end, &trace, &exit_trace)) {
			if (!(csgo.m_engine_trace()->GetPointContents(pen_end, MASK_SHOT_HULL) & MASK_SHOT_HULL))
				return false;
		}

		// get surface / material at exit point.
		exit_surface = csgo.m_phys_props()->GetSurfaceData(exit_trace.surface.surfaceProps);
		exit_material = exit_surface->game.material;

		// todo - dex; check for CHAR_TEX_FLESH and ff_damage_bullet_penetration / ff_damage_reduction_bullets convars?
		//             also need to check !isbasecombatweapon too.
		if (enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS) {
			total_pen_mod = 3.f;
			damage_mod = 0.05f;
		}

		else if (nodraw || grate) {
			total_pen_mod = 1.f;
			damage_mod = 0.16f;
		}

		else {
			total_pen_mod = (enter_surface->game.penetrationmodifier + exit_surface->game.penetrationmodifier) * 0.5f;
			damage_mod = 0.16f;
		}

		// thin metals, wood and plastic get a penetration bonus.
		if (enter_material == exit_material) {
			if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
				total_pen_mod = 3.f;

			else if (exit_material == CHAR_TEX_PLASTIC)
				total_pen_mod = 2.f;
		}

		// set some local vars.
		trace_len = (exit_trace.endpos - trace.endpos).LengthSquared();
		modifier = fmaxf(1.f / total_pen_mod, 0.f);

		// this calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
		damage_lost = fmaxf(
			((modifier * trace_len) / 24.f)
			+ ((damage * damage_mod)
				+ (fmaxf(3.75 / penetration, 0.f) * 3.f * modifier)), 0.f);

		// subtract from damage.
		damage -= max(0.f, damage_lost);
		if (damage < 1.f)
			return false;

		// set new start pos for successive trace.
		start = exit_trace.endpos;

		// decrement pen.
		--pen;
	}

	return false;
}
uint32_t c_autowall::get_filter_simple_vtable()
{
	return *reinterpret_cast<uint32_t*>(Engine::Displacement::Signatures[c_signatures::TRACEFILTER_SIMPLE] + 0x3d);
}

//
//void c_autowall::TraceLine(Vector& absStart, const Vector& absEnd, unsigned int mask, C_BasePlayer* ignore, CGameTrace* ptr)
//{
//	Ray_t ray;
//	ray.Init(absStart, absEnd);
//	CTraceFilter filter;
//	filter.pSkip = ignore;
//
//	csgo.m_engine_trace()->TraceRay(ray, mask, &filter, ptr);
//}
//
//float c_autowall::ScaleDamage(C_BasePlayer* player, float damage, float armor_ratio, int hitgroup) {
//	if (!player || !player->GetClientClass() || !ctx.m_local()->get_weapon() || player->GetClientClass()->m_ClassID != class_ids::CCSPlayer)
//		return 0.f;
//
//	auto new_damage = damage;
//
//	const auto is_zeus = ctx.m_local()->get_weapon()->m_iItemDefinitionIndex() == WEAPON_TASER;
//
//	static auto is_armored = [](C_BasePlayer* player, int armor, int hitgroup) {
//		if (player && player->m_ArmorValue() > 0)
//		{
//			if (player->m_bHasHelmet() && hitgroup == HITGROUP_HEAD || (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM || hitgroup == 8 || hitgroup == 0))
//				return true;
//		}
//		return false;
//	};
//
//	if (!is_zeus) {
//		switch (hitgroup)
//		{
//		case HITGROUP_HEAD:
//			new_damage *= 4.f;
//			break;
//		case HITGROUP_STOMACH:
//			new_damage *= 1.25f;
//			break;
//		case HITGROUP_LEFTLEG:
//		case HITGROUP_RIGHTLEG:
//			new_damage *= .75f;
//			break;
//		default:
//			break;
//			/*4.0; 1
//			1.0; 2
//			1.25; 3
//			1.0; 4
//			1.0; 5
//			0.75; 6
//			0.75; 7
//			1.0; 8*/
//		}
//	}
//	else
//		new_damage *= 0.92f;
//
//	if (is_armored(player, player->m_ArmorValue(), hitgroup))
//	{
//		float flHeavyRatio = 1.0f;
//		float flBonusRatio = 0.5f;
//		float flRatio = armor_ratio * 0.5f;
//		float flNewDamage;
//
//		if (!player->m_bHasHeavyArmor())
//		{
//			flNewDamage = damage * flRatio;
//		}
//		else
//		{
//			flBonusRatio = 0.33f;
//			flRatio = armor_ratio * 0.25f;
//			flHeavyRatio = 0.33f;
//			flNewDamage = (damage * flRatio) * 0.85f;
//		}
//
//		int iArmor = player->m_ArmorValue();
//
//		if (((damage - flNewDamage) * (flHeavyRatio * flBonusRatio)) > iArmor)
//			flNewDamage = damage - (iArmor / flBonusRatio);
//
//		new_damage = flNewDamage;
//	}
//
//	return floorf(new_damage);
//}
//bool EntityHasArmor(C_BasePlayer* Entity, int hitgroup)
//{
//	if (Entity->m_ArmorValue() > 0)
//	{
//		if ((hitgroup == HITGROUP_HEAD && Entity->m_bHasHelmet()) || (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM))
//			return true;
//	}
//	return false;
//}
//void c_autowall::ScaleDamage(CGameTrace& enterTrace, weapon_info* weaponData, float& currentDamage)
//{
//	if (!enterTrace.m_pEnt || !enterTrace.m_pEnt->GetClientClass() || !ctx.m_local()->get_weapon() || enterTrace.m_pEnt->GetClientClass()->m_ClassID != class_ids::CCSPlayer)
//		return;
//
//	C_BasePlayer* target = (C_BasePlayer*)enterTrace.m_pEnt;
//
//	bool bHasHeavyArmor = false;// player->HasHeavyArmor();
//
//	switch (enterTrace.hitgroup)
//	{
//	case HITGROUP_HEAD:
//		if (!bHasHeavyArmor)
//			currentDamage *= 4.0f;
//		else
//			currentDamage = (currentDamage * 4.0f) * 0.5f;
//		break;
//	case HITGROUP_STOMACH:
//		currentDamage *= 1.25f;
//		break;
//	case HITGROUP_LEFTLEG:
//	case HITGROUP_RIGHTLEG:
//		currentDamage *= 0.75f;
//		break;
//	}
//
//	if (target && EntityHasArmor(target, enterTrace.hitgroup))
//	{
//		float flHeavyRatio = 1.0f;
//		float flBonusRatio = 0.5f;
//		float flRatio = weaponData->armor_ratio * 0.5f;
//		float flNewDamage;
//
//		if (!bHasHeavyArmor)
//		{
//			flNewDamage = currentDamage * flRatio;
//		}
//		else
//		{
//			flBonusRatio = 0.33f;
//			flRatio = weaponData->armor_ratio * 0.25f;
//			flHeavyRatio = 0.33f;
//			flNewDamage = (currentDamage * flRatio) * 0.85f;
//		}
//
//		int iArmor = target->m_ArmorValue();
//
//		if (((currentDamage - flNewDamage) * (flHeavyRatio * flBonusRatio)) > iArmor)
//			flNewDamage = currentDamage - (iArmor / flBonusRatio);
//
//		currentDamage = flNewDamage;
//	}
//
//	currentDamage = floorf(currentDamage);
//}
//
//uint32_t c_autowall::get_filter_simple_vtable()
//{
//	return *reinterpret_cast<uint32_t*>(Engine::Displacement::Signatures[c_signatures::TRACEFILTER_SIMPLE] + 0x3d);
//}
//
//bool c_autowall::TraceToExit(const Vector& start, const Vector dir, Vector& out, trace_t* enter_trace, trace_t* exit_trace)
//{
//	static ConVar* sv_clip_penetration_traces_to_players = csgo.m_engine_cvars()->FindVar(sxor("sv_clip_penetration_traces_to_players"));
//
//	Vector          new_end;
//	float           dist = 0.0f;
//	int				iterations = 23;
//	int				first_contents = 0;
//	int             contents;
//	Ray_t r{};
//
//	while (1)
//	{
//		iterations--;
//
//		if (iterations <= 0 || dist > 90.f)
//			break;
//
//		dist += 4.0f;
//		out = start + (dir * dist);
//
//		contents = csgo.m_engine_trace()->GetPointContents(out, 0x4600400B, nullptr);
//
//		if (first_contents == -1)
//			first_contents = contents;
//
//		if (contents & 0x600400B && (!(contents & CONTENTS_HITBOX) || first_contents == contents))
//			continue;
//
//		new_end = out - (dir * 4.f);
//
//		TraceLine(out, new_end, 0x4600400B, nullptr, exit_trace);
//
//		if (exit_trace->startsolid && (exit_trace->surface.flags & SURF_HITBOX) != 0)
//		{
//			TraceLine(out, start, MASK_SHOT_HULL, (C_BasePlayer*)exit_trace->m_pEnt, exit_trace);
//
//			if (exit_trace->DidHit() && !exit_trace->startsolid)
//			{
//				out = exit_trace->endpos;
//				return true;
//			}
//			continue;
//		}
//
//		if (!exit_trace->DidHit() || exit_trace->startsolid)
//		{
//			if (enter_trace->m_pEnt != csgo.m_entity_list()->GetClientEntity(0)) {
//				if (exit_trace->m_pEnt && exit_trace->m_pEnt->is_breakable())
//				{
//					exit_trace->surface.surfaceProps = enter_trace->surface.surfaceProps;
//					exit_trace->endpos = start + dir;
//					return true;
//				}
//			}
//
//			continue;
//		}
//
//		if ((exit_trace->surface.flags & 0x80u) != 0)
//		{
//			if (enter_trace->m_pEnt && enter_trace->m_pEnt->is_breakable()
//				&& exit_trace->m_pEnt && exit_trace->m_pEnt->is_breakable())
//			{
//				out = exit_trace->endpos;
//				return true;
//			}
//
//			if (!(enter_trace->surface.flags & 0x80u))
//				continue;
//		}
//
//		if (exit_trace->plane.normal.Dot(dir) <= 1.f) // exit nodraw is only valid if our entrace is also nodraw
//		{
//			out -= dir * (exit_trace->fraction * 4.0f);
//			return true;
//		}
//	}
//
//	return false;
//}
//
//bool c_autowall::HandleBulletPenetration(C_BasePlayer* ignore, weapon_info* weaponData, trace_t& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_bullet_penetration, bool pskip)
//{
//	int is_nodraw = (enterTrace.surface.flags) & SURF_NODRAW;
//	bool cannotpenetrate = false;
//
//	Vector end;
//	trace_t exit_trace;
//	bool entry_is_grate = (enterTrace.contents & CONTENTS_GRATE);
//
//	const auto enter_surface_data = csgo.m_phys_props()->GetSurfaceData(enterTrace.surface.surfaceProps);
//	const int enter_material = enter_surface_data->game.material;
//	auto enemy = reinterpret_cast<C_BasePlayer*>(enterTrace.m_pEnt);
//
//	if (!possibleHitsRemaining)
//	{
//		if (!is_nodraw && !entry_is_grate)
//		{
//			if (enter_material != CHAR_TEX_GLASS)
//				cannotpenetrate = enter_material != CHAR_TEX_GRATE;
//		}
//	}
//
//	if (penetrationPower <= 0.0f || possibleHitsRemaining <= 0)
//		cannotpenetrate = true;
//
//	if (!TraceToExit(enterTrace.endpos, direction, end, &enterTrace, &exit_trace)) {
//		if (!(csgo.m_engine_trace()->GetPointContents(end, 0x600400B) & 0x600400B))
//			return false;
//	}
//
//
//	auto exit_surface_data = csgo.m_phys_props()->GetSurfaceData(exit_trace.surface.surfaceProps);
//	float damage_modifier;
//	float average_penetration_modifier;
//	unsigned short exit_material = exit_surface_data->game.material;
//
//	damage_modifier = 0.16f;
//	if (!entry_is_grate && !is_nodraw)
//	{
//		if (enter_material == CHAR_TEX_GLASS)
//			goto LABEL_51;
//
//		if (enter_material != CHAR_TEX_GRATE)
//		{
//			const float ff_damage_reduction_bullets = csgo.m_engine_cvars()->FindVar("ff_damage_reduction_bullets") != nullptr ? csgo.m_engine_cvars()->FindVar("ff_damage_reduction_bullets")->GetFloat() : 0.1f;
//			const float ff_damage_bullet_penetration = csgo.m_engine_cvars()->FindVar("ff_damage_bullet_penetration") != nullptr ? csgo.m_engine_cvars()->FindVar("ff_damage_bullet_penetration")->GetFloat() : 0.f;
//
//			if (enter_material != CHAR_TEX_FLESH
//				|| ff_damage_reduction_bullets != 0.0f
//				|| !enemy
//				|| !enemy->IsPlayer()
//				|| (enemy->m_iTeamNum() != ctx.m_local()->m_iTeamNum()))
//			{
//				average_penetration_modifier = (exit_surface_data->game.penetrationmodifier + enter_surface_data->game.penetrationmodifier) * 0.5f;
//				goto LABEL_46;
//			}
//
//			if (ff_damage_bullet_penetration == 0.0f)
//				return false;
//
//		LABEL_45:
//			average_penetration_modifier = enter_surface_data->game.penetrationmodifier;
//		LABEL_46:
//			damage_modifier = 0.16f;
//			goto LABEL_52;
//		}
//	}
//
//	if (enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS)
//	{
//		average_penetration_modifier = 1.0f;
//		goto LABEL_52;
//	}
//
//LABEL_51:
//	damage_modifier = 0.05f;
//	average_penetration_modifier = 3.0f;
//LABEL_52:
//
//	if (enter_material == exit_material)
//	{
//		if (exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD)
//			average_penetration_modifier = 3.0f;
//		else if (exit_material == CHAR_TEX_PLASTIC)
//			average_penetration_modifier = 2.0f;
//	}
//
//	float penetration_length = (exit_trace.endpos - enterTrace.endpos).Length();
//	float modifier = fmaxf(1.0f / average_penetration_modifier, 0.0f);
//	float taken_first = (fmaxf(((3.0f / penetrationPower) * 1.25f), 0.0f) * (modifier * 3.0f) + (currentDamage * damage_modifier));
//	float taken_damage = (((penetration_length * penetration_length) * modifier) / 24.0f) + taken_first;
//
//	currentDamage -= fmaxf(0.0f, taken_damage);
//
//	if (currentDamage < 1.0f)
//		return false;
//
//	eyePosition = exit_trace.endpos;
//
//	--possibleHitsRemaining;
//	return true;
//}
//
//void c_autowall::FixTraceRay(Vector end, Vector start, trace_t* oldtrace, C_BasePlayer* ent) {
//	if (!ent)
//		return;
//
//	const auto mins = ent->OBBMins();
//	const auto maxs = ent->OBBMaxs();
//
//	auto dir(end - start);
//	auto len = dir.Normalize();
//
//	const auto center = (mins + maxs) / 2;
//	const auto pos(ent->m_vecOrigin() + center);
//
//	auto to = pos - start;
//	const float range_along = dir.Dot(to);
//
//	float range;
//	if (range_along < 0.f) {
//		range = -(to).Length();
//	}
//	else if (range_along > len) {
//		range = -(pos - end).Length();
//	}
//	else {
//		auto ray(pos - (start + (dir * range_along)));
//		range = ray.Length();
//	}
//
//	if (range <= 30.f) {
//
//		Ray_t ray;
//		ray.Init(start, end);
//
//		trace_t trace;
//		csgo.m_engine_trace()->ClipRayToEntity(ray, 0x4600400B, ent, &trace);
//
//		if (oldtrace->fraction > trace.fraction)
//			*oldtrace = trace;
//	}
//}
//
//void c_autowall::ClipTraceToPlayers(const Vector& start, const Vector& end, uint32_t mask, ITraceFilter* filter, trace_t* tr) {
//	Vector     pos, to, dir, on_ray;
//	float      len, range_along, range;
//	//CGameTrace new_trace;
//
//	float smallestFraction = tr->fraction;
//	constexpr float maxRange = 60.0f;
//
//	//Vector delta(vecAbsEnd - vecAbsStart);
//	//const float delta_length = delta.Normalize();
//
//	dir = start - end;
//	len = dir.Normalize();
//
//	Ray_t ray;
//	ray.Init(start, end);
//
//	for (int i = 1; i <= 64; ++i) {
//		C_BasePlayer* ent = csgo.m_entity_list()->GetClientEntity(i);
//		if (!ent || ent->IsDormant() || ent->IsDead())
//			continue;
//
//		if (filter && !filter->ShouldHitEntity(ent, mask))
//			continue;
//
//		// set some local vars.
//		pos = ent->m_vecOrigin() + ((ent->OBBMins() + ent->OBBMaxs()) * 0.5f);
//		to = pos - start;
//		range_along = dir.Dot(to);
//
//		// off start point.
//		if (range_along < 0.f)
//			range = -(to).Length();
//
//		// off end point.
//		else if (range_along > len)
//			range = -(pos - end).Length();
//
//		// within ray bounds.
//		else {
//			on_ray = start + (dir * range_along);
//			range = (pos - on_ray).Length();
//		}
//
//		if (/*range > 0.0f && */range <= maxRange) {
//			trace_t playerTrace;
//
//			csgo.m_engine_trace()->ClipRayToEntity(ray, 0x4600400B, ent, &playerTrace);
//
//			if (playerTrace.fraction < smallestFraction) {
//				// we shortened the ray - save off the trace
//				*tr = playerTrace;
//				smallestFraction = playerTrace.fraction;
//			}
//		}
//	}
//}
//
//int c_autowall::HitboxToHitgroup(C_BasePlayer* m_player, int ihitbox)
//{
//	switch (ihitbox)
//	{
//	case HITBOX_HEAD:
//	case HITBOX_NECK:
//		return HITGROUP_HEAD;
//	case HITBOX_UPPER_CHEST:
//	case HITBOX_CHEST:
//	case HITBOX_THORAX:
//	case HITBOX_LEFT_UPPER_ARM:
//	case HITBOX_RIGHT_UPPER_ARM:
//		return HITGROUP_CHEST;
//	case HITBOX_PELVIS:
//	case HITBOX_LEFT_THIGH:
//	case HITBOX_RIGHT_THIGH:
//	case HITBOX_BODY:
//		return HITGROUP_STOMACH;
//	case HITBOX_LEFT_CALF:
//	case HITBOX_LEFT_FOOT:
//		return HITGROUP_LEFTLEG;
//	case HITBOX_RIGHT_CALF:
//	case HITBOX_RIGHT_FOOT:
//		return HITGROUP_RIGHTLEG;
//	case HITBOX_LEFT_FOREARM:
//	case HITBOX_LEFT_HAND:
//		return HITGROUP_LEFTARM;
//	case HITBOX_RIGHT_FOREARM:
//	case HITBOX_RIGHT_HAND:
//		return HITGROUP_RIGHTARM;
//	default:
//		return HITGROUP_STOMACH;
//	}
//}
//
//bool c_autowall::FireBullet(Vector eyepos, C_WeaponCSBaseGun* pWeapon, Vector& direction, float& currentDamage, C_BasePlayer* ignore, C_BasePlayer* to_who, int hitbox, bool* was_viable, std::vector<float>* power)
//{
//	if (!pWeapon || !ignore)
//		return false;
//
//	float currentDistance = 0;
//
//	static ConVar* damageBulletPenetration = csgo.m_engine_cvars()->FindVar(sxor("ff_damage_bullet_penetration"));
//
//	const float ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();
//
//	weapon_info* weaponData = ctx.latest_weapon_data;
//	CGameTrace enterTrace;
//
//	CTraceFilter filter;
//	filter.pSkip = ignore;
//
//	if (!weaponData)
//		return false;
//
//	currentDamage = float(weaponData->damage);
//	auto maxRange = weaponData->range;
//	auto penetrationDistance = weaponData->range;
//	auto penetrationPower = weaponData->penetration;
//	auto RangeModifier = weaponData->range_modifier;
//
//	ctx.last_penetrated_count = 4;
//	ctx.last_hitgroup = -1;
//
//	int penetrated = 0;
//
//	while (currentDamage > 0)
//	{
//		Vector end = eyepos + direction * (maxRange - currentDistance);
//
//		TraceLine(eyepos, end, MASK_SHOT, ignore, &enterTrace);
//
//		if (to_who || enterTrace.contents & CONTENTS_HITBOX && enterTrace.m_pEnt) {
//			FixTraceRay(eyepos + (direction * 40.f), eyepos, &enterTrace, (to_who != nullptr ? to_who : (C_BasePlayer*)enterTrace.m_pEnt));
//		}
//		else
//			ClipTraceToPlayers(eyepos, eyepos + (direction * 40.f), 0x4600400B, &filter, &enterTrace);
//
//		if (enterTrace.fraction == 1.0f)
//			return false;
//
//		maxRange -= currentDistance;
//		currentDistance += maxRange * enterTrace.fraction;
//
//		//Let's make our damage drops off the further away the bullet is.
//		currentDamage *= powf(RangeModifier, (currentDistance * 0.002f));
//
//		if (!(enterTrace.contents & CONTENTS_HITBOX))
//			enterTrace.hitgroup = 1;
//
//		//This looks gay as fuck if we put it into 1 long line of code.
//		const bool canDoDamage = enterTrace.hitgroup > 0 && enterTrace.hitgroup <= 8 || enterTrace.hitgroup == HITGROUP_GEAR;
//		const bool isPlayer = enterTrace.m_pEnt != nullptr
//			&& enterTrace.m_pEnt->GetClientClass()
//			&& enterTrace.m_pEnt->GetClientClass()->m_ClassID == class_ids::CCSPlayer
//			&& (!ctx.m_local() || ctx.m_local()->IsDead() || ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() != ctx.m_local()->m_iTeamNum());
//
//		if (to_who)
//		{
//			if (enterTrace.m_pEnt && to_who == enterTrace.m_pEnt && canDoDamage && isPlayer) {
//				const int group = (pWeapon->m_iItemDefinitionIndex() == WEAPON_TASER) ? HITGROUP_GENERIC : enterTrace.hitgroup;
//				ctx.last_hitgroup = group;
//
//				ScaleDamage(enterTrace, weaponData, currentDamage);
//
//				if (was_viable != nullptr)
//					*was_viable = (penetrated == 0);
//
//				ctx.force_hitbox_penetration_accuracy = false;
//
//				return true;
//			}
//		}
//		else
//		{
//			if (enterTrace.m_pEnt && canDoDamage && isPlayer) {
//				const int group = (pWeapon->m_iItemDefinitionIndex() == WEAPON_TASER) ? HITGROUP_GENERIC : enterTrace.hitgroup;
//				ctx.last_hitgroup = group;
//
//				ScaleDamage(enterTrace, weaponData, currentDamage);
//
//				if (was_viable != nullptr)
//					*was_viable = (penetrated == 0);
//
//				ctx.force_hitbox_penetration_accuracy = false;
//
//				return true;
//			}
//		}
//
//		//Sanity checking / Can we actually shoot through?
//		if (currentDistance > maxRange && penetrationPower
//			|| csgo.m_phys_props()->GetSurfaceData(enterTrace.surface.surfaceProps)->game.penetrationmodifier < 0.1f) {
//			break;
//		}
//
//		const auto prev = ctx.last_penetrated_count;
//
//		//Calling HandleBulletPenetration here reduces our penetrationCounter, and if it returns true, we can't shoot through it.
//		if (!HandleBulletPenetration(ignore, weaponData, enterTrace, eyepos, direction, ctx.last_penetrated_count, currentDamage, penetrationPower, ff_damage_bullet_penetration)) {
//			break;
//		}
//		if (prev != ctx.last_penetrated_count)
//			penetrated++;
//	}
//
//	return false;
//}
//
//////////////////////////////////////// Usage Calls //////////////////////////////////////
//float c_autowall::CanHit(Vector& vecEyePos, Vector& point)
//{
//	Vector angles, direction;
//	Vector tmp = point - vecEyePos;
//	float currentDamage = 0;
//
//	Math::VectorAngles(tmp, angles);
//	Math::AngleVectors(angles, &direction);
//	direction.Normalize();
//
//	if (m_weapon() != nullptr && m_weapon()->IsGun() && FireBullet(vecEyePos, m_weapon(), direction, currentDamage, ctx.m_local()))
//		return currentDamage;
//
//	return -1; //That wall is just a bit too thick buddy
//}
//
//float c_autowall::CanHit(const Vector& vecEyePos, Vector& point, C_BasePlayer* ignore_ent, C_BasePlayer* to_who, int hitbox, bool* was_viable)
//{
//	if (ignore_ent == nullptr || to_who == nullptr)
//		return 0;
//
//	Vector direction;
//	Vector tmp = point - vecEyePos;
//	float currentDamage = 0;
//
//	//Math::VectorAngles(tmp, angles);
//	//Math::AngleVectors(angles, &direction);
//	direction = tmp.Normalized();
//
//	if (m_weapon() != nullptr)
//	{
//		if (m_weapon()->IsGun() && FireBullet(vecEyePos, m_weapon(), direction, currentDamage, ignore_ent, to_who, hitbox, was_viable))
//			return currentDamage;
//		else
//			return -1;
//	}
//
//	return -1; //That wall is just a bit too thick buddy
//}
//
//float c_autowall::SimulateShot(Vector& vecEyePos, Vector& point, C_BasePlayer* ignore_ent, C_BasePlayer* to_who, bool* was_viable)
//{
//	if (ignore_ent == nullptr || to_who == nullptr)
//		return 0;
//
//	Vector angles, direction;
//	const Vector tmp(point - vecEyePos);
//	float currentDamage = 0.f;
//
//	Math::VectorAngles(tmp, angles);
//	Math::AngleVectors(angles, &direction);
//	direction.Normalize();
//
//	/*
//		maxRange = power[0];
//		penetrationDistance = power[1];
//		penetrationPower = power[2];
//		currentDamage = power[3];
//	*/
//
//	static std::vector<float> power = { 4000.f, 4000.f, 2.50f, 80.f, 1.f };
//
//	if (m_weapon() != nullptr)
//	{
//		if (FireBullet(vecEyePos, m_weapon(), direction, currentDamage, ignore_ent, to_who, -1, was_viable, &power))
//			return currentDamage;
//		else
//			return -1;
//	}
//
//	return -1; //That wall is just a bit too thick buddy
//}