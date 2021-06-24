#include "source.hpp"
#include <props/entity.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <hooks/hooked.hpp>
#include <sdk/math/math.hpp>
#include <props/displacement.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <ragebot/prediction.hpp>
#include <algorithm>
#include <iostream>
#include <menu/menu/menu.hpp>
#include <misc/movement.hpp>
#include "usercmd.hpp"
#include <ragebot/autowall.hpp>
#include <ragebot/resolver.hpp>
#include <ragebot/rage_aimbot.hpp>

float c_antiaimbot::get_max_desync_delta(C_BasePlayer* ent) //autistic but w/e
{
	auto animstate = ent->get_animation_state();
	auto speedfraction = max(0.f, min(animstate->m_speed_as_portion_of_walk_top_speed, 1.f));
	auto speedfactor = max(0.f, min(animstate->m_speed_as_portion_of_crouch_top_speed, 1.f));

	auto lol = ((animstate->m_walk_run_transition * -0.30000001f) - 0.19999999f) * speedfraction + 1.f;

	if (animstate->m_anim_duck_amount > 0.0f)
		lol += ((animstate->m_anim_duck_amount * speedfactor) * (0.5f - lol));

	return (animstate->m_aim_yaw_max * lol);
}

bool IsVisible(Vector start, Vector end, C_BasePlayer* skip, C_BasePlayer* ent)
{
	trace_t trace;
	//util_trace_line(start, end, MASK_SHOT_HULL, filter, &trace);
	Ray_t ray;
	ray.Init(start, end);

	if (skip)
	{
		CTraceFilter filter;
		filter.pSkip = skip;
		csgo.m_engine_trace()->TraceRay(ray, MASK_SHOT_HULL, &filter, &trace);
	}
	else
	{
		CTraceFilterWorldOnly filter;
		csgo.m_engine_trace()->TraceRay(ray, MASK_SHOT_HULL, &filter, &trace);
	}

	return trace.m_pEnt == ent;
}

void c_antiaimbot::get_targets()
{
	players.clear();

	C_BasePlayer* target = nullptr;

	QAngle original_viewangles;
	csgo.m_engine()->GetViewAngles(original_viewangles);

	float lowest_distance = 99999.f;
	for (auto i = 1; i < 64; i++)
	{
		origins[i - 1] = Vector::Zero;
		auto player = csgo.m_entity_list()->GetClientEntity(i);

		if (!player || player->IsDead() || player->m_iTeamNum() == ctx.m_local()->m_iTeamNum() || player == ctx.m_local())
			continue;

		const auto idx = player->entindex() - 1;

		const auto& curlog = &feature::lagcomp->records[idx];

		origins[i - 1] = player->m_vecAbsOrigin();
		player->GetWorldSpaceCenter(origins[i - 1]);

		if (!player->IsDormant() && curlog->records_count > 0)// && abs(player->m_flSimulationTime() - TICKS_TO_TIME(csgo.m_globals()->tickcount)) > 5.f)
			curlog->last_scan_time = csgo.m_client_state()->m_clockdrift_manager.m_nServerTick;

		if (std::abs(csgo.m_client_state()->m_clockdrift_manager.m_nServerTick - curlog->last_scan_time) >= 20)
			continue;

		if (IsVisible(ctx.m_local()->GetEyePosition(), origins[i - 1], ctx.m_local(), player))
			players.push_back(player);

		QAngle angle = Math::CalcAngle(ctx.m_local()->GetEyePosition(), origins[i - 1]);
		QAngle delta = angle - original_viewangles;
		delta.Normalize();
		delta.z = 0.f;

		float dist = sqrt(delta.x * delta.x + delta.y * delta.y);

		if (dist < lowest_distance)
		{
			target = player;
			lowest_distance = dist;
		}
	}

	if (!target || *(void**)target == nullptr)
		return;

	players.push_back(target);
}

void c_antiaimbot::run_at_target(float& yaw)
{
	auto GetFOV = [](const QAngle& view_angles, const Vector& start, const Vector& end) -> float {
		Vector dir, fw;

		// get direction and normalize.
		dir = (end - start).Normalized();

		// get the forward direction vector of the view angles.
		Math::AngleVectors(view_angles, &fw);

		// get the angle between the view angles forward directional vector and the target location.
		return max(RAD2DEG(std::acos(fw.Dot(dir))), 0.f);
	};

	Vector best_origin = Vector::Zero;
	float best_distance = 9999.f;
	float best_fov = 9999.f;
	int best_idx = -1;
	bool best_dormant = false;

	QAngle viewangles;
	csgo.m_engine()->GetViewAngles(viewangles);

	for (auto i = 1; i < 64; i++)
	{
		if (origins[i - 1].IsZero())
			continue;

		auto player = csgo.m_entity_list()->GetClientEntity(i);

		if (!player || player->IsDead() || player->m_iTeamNum() == ctx.m_local()->m_iTeamNum() || player == ctx.m_local())
			continue;

		const auto idx = player->entindex() - 1;

		const auto& curlog = &feature::lagcomp->records[idx];

		if (abs(csgo.m_client_state()->m_clockdrift_manager.m_nServerTick - curlog->last_scan_time) >= 20)
			continue;

		//Vector forward;
		//Math::AngleVectors(Math::CalcAngle(ctx.m_eye_position, origins[i-1]), &forward);

		const auto dist = (origins[i - 1] - ctx.m_local()->GetEyePosition()).LengthSquared();
		Vector wsc = origins[i - 1];
		/*const*/ auto fov = GetFOV(viewangles, ctx.m_local()->GetEyePosition(), wsc);

		if (dist < best_distance)
		{
			best_origin = origins[i - 1];
			best_distance = dist;
			best_dormant = !player->IsDormant();
			best_fov = fov;
			best_idx = i;
		}
	}

	if (feature::ragebot->m_target != nullptr && !feature::ragebot->m_target->IsDormant() && !feature::ragebot->m_target->IsDead() && feature::ragebot->m_target->entindex() != best_idx) {
		best_origin = feature::ragebot->m_target->m_vecOrigin() + feature::ragebot->m_target->m_vecVelocity() * csgo.m_globals()->interval_per_tick;
	}

	if (best_origin.IsZero())
		return;

	const auto angle = Math::CalcAngle(ctx.m_local()->GetEyePosition(), best_origin);
	yaw = Math::normalize_angle(angle.y);
}

void c_antiaimbot::auto_direction() {
	// constants.
	static constexpr float STEP{ 4.f };
	static constexpr float RANGE{ 32.f };

	if (!ctx.m_local() || ctx.m_local()->IsDead())
		return;

	AutoTarget_t target = AutoTarget_t{ 180.f - 1.f, nullptr };

	// iterate players.
	for (int i{ 0 }; i <= csgo.m_globals()->maxClients; ++i) {
		C_BasePlayer* player = csgo.m_entity_list()->GetClientEntity(i);

		if (!player || player->IsDead() || player->entindex() < 0 || player->entindex() > 64 || player->m_iTeamNum() == ctx.m_local()->m_iTeamNum() || player == ctx.m_local())
			continue;

		const auto idx = player->entindex() - 1;

		auto* curlog = &feature::lagcomp->records[idx];

		if (!curlog || abs(csgo.m_client_state()->m_clockdrift_manager.m_nServerTick - curlog->last_scan_time) >= 20)
			continue;

		auto absorg = player->m_vecAbsOrigin();
		player->GetWorldSpaceCenter(absorg);

		// get best target based on fov.
		float fov = Math::GetFov(Engine::Movement::Instance()->m_qRealAngles, Math::CalcAngle(ctx.m_local()->GetEyePosition(), absorg));

		if (fov < target.fov) {
			target.fov = fov;
			target.player = player;
		}
	}

	if (!target.player || !target.player->IsPlayer()) {
		// we have a timeout.
		/*if (m_auto_last > 0.f && m_auto_time > 2.5f && csgo.m_globals()->curtime < (m_auto_last + 2.5f))
			return;*/

			// set angle to backwards.
		m_auto = Math::normalize_angle(Engine::Movement::Instance()->m_qRealAngles.y - 179.f);
		m_auto_dist = -1.f;
		return;
	}

	// construct vector of angles to test.
	std::vector<AdaptiveAngle> angles = {};
	angles.emplace_back(Engine::Movement::Instance()->m_qRealAngles.y - 179.f);
	angles.emplace_back(Engine::Movement::Instance()->m_qRealAngles.y + 90.f);
	angles.emplace_back(Engine::Movement::Instance()->m_qRealAngles.y - 90.f);

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid = false;

	const auto start = target.player->Weapon_ShootPosition();

	// iterate vector of angles.
	for (auto i = 0; i < (int)angles.size(); ++i) {

		auto it = &angles.at(i);

		// compute the 'rough' estimation of where our head will be.
		Vector end{ ctx.m_local()->GetEyePosition().x + std::cos(DEG2RAD(it->m_yaw)) * RANGE,
					ctx.m_local()->GetEyePosition().y + std::sin(DEG2RAD(it->m_yaw)) * RANGE,
					ctx.m_local()->GetEyePosition().z };

		// draw a line for debugging purposes.
		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		auto dir = end - start;
		float len = dir.Normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			const auto point = start + (dir * i);

			// get the contents at this point.
			const int contents = csgo.m_engine_trace()->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid || angles.empty()) {
		// set angle to backwards.
		m_auto = Math::normalize_angle(Engine::Movement::Instance()->m_qRealAngles.y - 179.f);
		m_auto_dist = -1.f;
		return;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();

	// check if we are not doing a useless change.
	if (best->m_dist != m_auto_dist) {
		// set yaw to the best result.
		m_auto = Math::normalize_angle(best->m_yaw);
		m_auto_dist = best->m_dist;
		m_auto_last = csgo.m_globals()->curtime;
	}
}

void Normalize3(QAngle& angles) {
	while (angles.x < -180.0f) angles.x += 360.0f;
	while (angles.x > 180.0f) angles.x -= 360.0f;

	while (angles.y < -180.0f) angles.y += 360.0f;
	while (angles.y > 180.0f) angles.y -= 360.0f;
	angles.z = 0.f;
}

void ClampAngles(QAngle& angles) {
	if (angles.x > 89.0f) angles.x = 89.0f;
	else if (angles.x < -89.0f) angles.x = -89.0f;

	if (angles.y > 180.0f) angles.y = 180.0f;
	else if (angles.y < -180.0f) angles.y = -180.0f;

	angles.z = 0;
}
void c_antiaimbot::change_angles(CUserCmd* cmd, bool* send_packet)
{
	VIRTUALIZER_START;

	auto animstate = ctx.m_local()->get_animation_state();

	get_targets();

	const auto at_target = ctx.m_settings.anti_aim_yaw_base == 1;

	if (at_target)
		run_at_target(cmd->viewangles.y);

	auto_direction();

	max_delta = get_max_desync_delta(ctx.m_local());

	switch (ctx.m_settings.anti_aim_pitch)
	{
	case 1:
		cmd->viewangles.x = 175.0f;
		break;
	case 2:
		cmd->viewangles.x = -179.0f;
		break;
	case 3:
		cmd->viewangles.x = 177.0f;
		break;
	case 4:
		cmd->viewangles.x = 179.0f;
		break;
	case 5:
		cmd->viewangles.x = RandomInt(-179.0f, 179.0f);
		break;
	}

	float f_max_limit = ctx.m_settings.anti_aim_fake_limit;

	if (ctx.side != -1)
	{
		if (!ctx.side)
			cmd->viewangles.y += 90.f;
		else if (ctx.side == 1)
			cmd->viewangles.y -= 90.f;
		else if (ctx.side == 2)
			cmd->viewangles.y += 178.f;
	}
	else {
		switch (ctx.m_settings.anti_aim_yaw_type)
		{
		case 1:
			cmd->viewangles.y += 180.0f;
			break;
		case 2:
			cmd->viewangles.y += Math::normalize_angle(csgo.m_globals()->curtime * 250.f);
			break;
		case 3:
			cmd->viewangles.y += RandomInt(-179.0f, 179.0f);
			break;
		case 4:
			//cmd->viewangles.y += ctx.m_settings.anti_aim_custom_yaw;
			break;
		}
	}

	const auto v81 = !ctx.m_local()->m_bIsScoped();
	float v90;

	if (v81)
		v90 = ctx.latest_weapon_data->max_speed * 0.25f;
	else
		v90 = ctx.latest_weapon_data->max_speed_alt * 0.25f;

	static float previous_hp = ctx.m_local()->m_iHealth();

	previous_hp = ctx.m_local()->m_iHealth();

	static bool force_choke = false;
	const auto move_speed = sqrtf((cmd->forwardmove * cmd->forwardmove) + (cmd->sidemove * cmd->sidemove));
	const auto vel_speed = sqrtf((ctx.m_local()->m_vecVelocity().y * ctx.m_local()->m_vecVelocity().y) + (ctx.m_local()->m_vecVelocity().x * ctx.m_local()->m_vecVelocity().x));
	const auto new_max_delta = max_delta * (f_max_limit / 60.f);

	RandomSeed(cmd->command_number & 255);

	static auto is_inverted = false;
	is_inverted = (ctx.fside > 0);

	cmd->viewangles.y = Math::normalize_angle(cmd->viewangles.y);

	float modifer = 0.f;
	static bool jitter_flip = false;

	jitter_flip = !jitter_flip;

	modifer += ctx.m_settings.anti_aim_yaw_custom;

	if (ctx.m_settings.anti_aim_freestanding_body_yaw && *send_packet) {
		if (m_auto_dist != -1) {
			ctx.fside = Math::AngleDiff(Engine::Movement::Instance()->m_qRealAngles.y, m_auto) < 0 ? -1 : 1;
		}
	}

	if (ctx.m_settings.anti_aim_freestanding_fake_type > 0 && ctx.allow_freestanding) {
		if ((m_auto_dist != -1 && at_target) || !at_target)
			cmd->viewangles.y = m_auto;
	}

	switch (ctx.m_settings.anti_aim_yaw_jitter_type)
	{
	case 1:
		modifer = jitter_flip ? ctx.m_settings.anti_aim_yaw_jitter_custom : -ctx.m_settings.anti_aim_yaw_jitter_custom;
		break;
	case 2:
		modifer = ctx.m_settings.anti_aim_yaw_jitter_custom / 2;
		break;
	case 3:
		modifer = RandomFloat(0, ctx.m_settings.anti_aim_yaw_jitter_custom);
		break;
	}

	last_real_angle = cmd->viewangles;

	auto inv_body_lean = new_max_delta;
	auto view_yaw = last_real_angle.y;
	float leaned_yaw = 0.f;
	float abs_lean = 0.f;
	float j = 0.f;
	float k = 0.f;
	static bool broke_lby = false;

	float v8, chto_blyad, i;
	bool v12;

	if (ctx.m_settings.anti_aim_body_yaw == 1)
	{
		if (!is_inverted)
			inv_body_lean *= -1.f;

		chto_blyad = *(float*)(uintptr_t(csgo.m_globals()) + 0x20) * 102.0f;

		if (abs(abs_lean) <= chto_blyad && new_max_delta < max_delta)
			if (!*send_packet && csgo.m_client_state()->m_iChockedCommands < 14u)
			{
				cmd->viewangles.y = Math::normalize_angle(view_yaw);
			}
		if (abs(abs_lean) <= (180.0f - (max_delta + chto_blyad)))
		{
			if (abs_lean <= 0.0)
				view_yaw = max_delta + leaned_yaw;
			else
				view_yaw = leaned_yaw - max_delta;
		}
		else
		{
			view_yaw = leaned_yaw;
		}
		v12 = 1;
		i = ctx.angles[ANGLE_REAL];
		if (new_max_delta >= max_delta)
		{
			if (is_inverted)
			{
				view_yaw = last_real_angle.y + 120.0f;
			}
			else
			{
				view_yaw = last_real_angle.y - 120.0f;
			}
			if (!*send_packet && csgo.m_client_state()->m_iChockedCommands < 14u)
			{
				cmd->viewangles.y = Math::normalize_angle(view_yaw);
			}
			if (*send_packet)
				view_yaw = last_real_angle.y;
		}
	}
	else if (ctx.m_settings.anti_aim_body_yaw == 2)
	{
		static bool invert_jitter = false;

		if (invert_jitter) {
			cmd->viewangles.y = last_real_angle.y + 180.0f;
			last_real_angle.y = Math::normalize_angle(cmd->viewangles.y);
		}

		if (*send_packet) {
			invert_jitter = !invert_jitter;

			if (!invert_jitter)
				cmd->viewangles.y = Math::normalize_angle(last_real_angle.y + (new_max_delta * 0.5f) * ctx.fside);
			else
				cmd->viewangles.y = Math::normalize_angle(last_real_angle.y - (new_max_delta * 0.5f) * ctx.fside);
		}
		else /*if (!flick_lby)*/ {
			if (invert_jitter)
				cmd->viewangles.y = Math::normalize_angle(last_real_angle.y - (new_max_delta + 5.0f) * ctx.fside);
			else
				cmd->viewangles.y = Math::normalize_angle(last_real_angle.y + (new_max_delta + 5.0f) * ctx.fside);
		}
	}
	else if (ctx.m_settings.anti_aim_body_yaw == 3)
	{

		if (!is_inverted)
			inv_body_lean *= -1.f;

		// current abs rotation
		for (leaned_yaw = inv_body_lean + last_real_angle.y; leaned_yaw > 180.0; leaned_yaw = leaned_yaw - 360.0)
			;
		for (; leaned_yaw < -180.0; leaned_yaw = leaned_yaw + 360.0)
			;
		v8 = ctx.angles[ANGLE_REAL];
		if (v8 > 180.0)
		{
			do
				v8 = v8 - 360.0;
			while (v8 > 180.0);
		}
		for (; v8 < -180.0; v8 = v8 + 360.0)
			;
		for (abs_lean = v8 - leaned_yaw; abs_lean > 180.0; abs_lean = abs_lean - 360.0)
			;
		for (; abs_lean < -180.0; abs_lean = abs_lean + 360.0)
			;
		chto_blyad = *(float*)(uintptr_t(csgo.m_globals()) + 0x20) * 102.0f;
		// max body yaw * yaw modifier
		if (abs(abs_lean) <= chto_blyad && new_max_delta < max_delta)
			goto LABEL_57;
		if (abs(abs_lean) <= (180.0f - (max_delta + chto_blyad)))
		{
			if (abs_lean <= 0.0)
				view_yaw = max_delta + leaned_yaw;
			else
				view_yaw = leaned_yaw - max_delta;
		}
		else
		{
			view_yaw = leaned_yaw;
		}
		v12 = 1;
		i = ctx.angles[ANGLE_REAL];
		if (new_max_delta >= max_delta)
		{
			for (j = last_real_angle.y; j > 180.0; j = j - 360.0)
				;
			for (; j < -180.0; j = j + 360.0)
				;
			for (; i > 180.0; i = i - 360.0)
				;
			for (; i < -180.0; i = i + 360.0)
				;
			for (k = i - j; k > 180.0; k = k - 360.0)
				;
			for (; k < -180.0; k = k + 360.0)
				;
			if (is_inverted)
			{
				if (k < new_max_delta || k >= 179.0)
					goto LABEL_46;
			}
			else if ((new_max_delta * -1.f) < k || k <= -179.0f)
			{
				goto LABEL_50;
			}
			v12 = 0;
			if (*send_packet)
				goto LABEL_59;
		LABEL_46:
			if (is_inverted)
			{
				if (k > 0.0 && k < 179.0)
					view_yaw = last_real_angle.y + 120.0f;
				goto LABEL_53;
			}
		LABEL_50:
			if (k < 0.0 && k > -179.0)
				view_yaw = last_real_angle.y - 120.0f;
		LABEL_53:
			if (!v12)
				goto LABEL_57;
		}
	LABEL_57:
		if (!*send_packet && csgo.m_client_state()->m_iChockedCommands < 14u)
		{
			goto LABEL_63;
		}
	LABEL_59:
		view_yaw = last_real_angle.y;
	LABEL_63:

		cmd->viewangles.y = Math::normalize_angle(view_yaw);
	}

	cmd->viewangles.y = std::remainderf(cmd->viewangles.y, 360.0f);
	cmd->viewangles.Clamp();

	VIRTUALIZER_END;
}

bool c_antiaimbot::peek_fake_lag(CUserCmd* cmd, bool* send_packet)
{
	auto choked = 15;

	if (!m_weapon())
		return false;

	//const auto weapon_info = ctx.latest_weapon_data;

	//if (!weapon_info)
	//	return false;

	//auto simulated_origin = ctx.m_local()->m_vecOrigin();
	//auto move_per_tick = ctx.m_local()->m_vecVelocity() * csgo.m_globals()->interval_per_tick;

	//int when_started_to_penetrate = 0;

	//Vector vDuckHullMin = csgo.m_movement()->GetPlayerMins(true);
	//Vector vStandHullMin = csgo.m_movement()->GetPlayerMins(false);

	//float fMore = (vDuckHullMin.z - vStandHullMin.z);

	//Vector vecDuckViewOffset = csgo.m_movement()->GetPlayerViewOffset(true);
	//Vector vecStandViewOffset = csgo.m_movement()->GetPlayerViewOffset(false);
	//float duckFraction = min(1.f, ctx.m_local()->m_flDuckAmount() + 0.06f);

	//float tempz = ((vecDuckViewOffset.z - fMore) * duckFraction) +
	//	(vecStandViewOffset.z * (1 - duckFraction));

	///*
	//csgo.m_engine()->GetViewAngles(ang);
	//*/

	//Vector direction;
	//Math::AngleVectors(Engine::Movement::Instance()->m_qRealAngles, &direction);
	//auto max_range = weapon_info->range * 2;
	//auto dmg = (float)weapon_info->damage;
	//CTraceFilter filter;
	//filter.pSkip = ctx.m_local();
	//CGameTrace enterTrace;


	//for (int i = 0; i < choked; i++) {
	//	simulated_origin += move_per_tick;

	//	Vector start = simulated_origin + Vector(0, 0, tempz);
	//	Vector end = start + (direction * max_range);
	//	auto currentDistance = 0.f;

	//	//feature::autowall->TraceLine(start, end, MASK_SHOT | CONTENTS_GRATE, ctx.m_local(), &enterTrace);

	//	if (enterTrace.fraction == 1.0f)
	//		dmg = 0.f;
	//	else
	//		//calculate the damage based on the distance the bullet traveled.
	//		currentDistance += enterTrace.fraction * max_range;

	//	//Let's make our damage drops off the further away the bullet is.
	//	dmg *= pow(weapon_info->range_modifier, (currentDistance / 500.f));

	//	auto enterSurfaceData = csgo.m_phys_props()->GetSurfaceData(enterTrace.surface.surfaceProps);
	//	float enterSurfPenetrationModifier = enterSurfaceData->game.penetrationmodifier;

	//	if (currentDistance > 3000.0 && weapon_info->penetration > 0.f || enterSurfPenetrationModifier < 0.1f)
	//		dmg = -1.f;

	//	if (enterTrace.m_pEnt != nullptr)
	//	{
	//		//This looks gay as fuck if we put it into 1 long line of code.
	//		bool canDoDamage = (enterTrace.hitgroup - 1) <= 7;
	//		bool isPlayer = (enterTrace.m_pEnt->GetClientClass() && enterTrace.m_pEnt->GetClientClass()->m_ClassID == class_ids::CCSPlayer);
	//		//bool isEnemy = (ctx.m_local()->m_iTeamNum() != ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum());
	//		bool onTeam = (((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() == 2 || ((C_BasePlayer*)enterTrace.m_pEnt)->m_iTeamNum() == 3);

	//		//TODO: Team check config
	//		if (canDoDamage && isPlayer && onTeam)
	//			feature::autowall->ScaleDamage(enterTrace, weapon_info, dmg);

	//		if (!canDoDamage && isPlayer)
	//			dmg = -1.f;
	//	}

	//	auto penetrate_count = 4;

	//	ctx.force_low_quality_autowalling = true;
	//	if (!feature::autowall->HandleBulletPenetration(ctx.m_local(), weapon_info, enterTrace, start, direction, penetrate_count, dmg, weapon_info->penetration, 0.f, true))
	//		dmg = -1.f;
	//	ctx.force_low_quality_autowalling = false;

	//	if (penetrate_count <= 0)
	//		dmg = -1.f;

	//	auto can_penetrate = dmg > 0.f;

	//	if (can_penetrate)
	//		when_started_to_penetrate = i;
	//}

	//if (when_started_to_penetrate == 4) {
	//	*send_packet = true;
	//	return true;
	//}
	//else {
	//	*send_packet = false;
	//	return true;
	//}
}

void c_antiaimbot::fake_lagv2(CUserCmd* cmd, bool* send_packet)
{
	VIRTUALIZER_START;

	if (!cmd || cmd == nullptr || ctx.m_local() == nullptr)
		return;

	if (cmd->command_number == ctx.m_ragebot_shot_nr)
	{
		*send_packet = true;
		return;
	}

	unchocking = false;

	bool lag = false; static float lag_timer = 0.f; static float old_delta = 0.f;

	bool force_nade_choke = false;

	static bool is_throwing = false;
	if (m_weapon() != nullptr && ctx.latest_weapon_data != nullptr && m_weapon()->IsGrenade() && m_weapon()->IsBeingThrowed())
	{
		if (!(cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2) && throw_nade || is_throwing) {
			is_throwing = true;
			enable_delay = csgo.m_globals()->realtime + csgo.m_globals()->interval_per_tick;
		}
		else
		{
			//maximal_choke = max(maximal_choke, 10);
			force_nade_choke = true;
			lag = true;

			throw_nade = true;
			is_throwing = false;
		}
	}
	else {
		throw_nade = false;
		is_throwing = false;
	}

	if (ctx.fakeducking)
		return;



	const auto chocked_ticks = csgo.m_client_state()->m_iChockedCommands;
	const auto new_velo = (animation_speed > 2.f ? animation_speed : 0.f);

	RandomSeed(cmd->random_seed & 255);

	//const auto origin_delta = sent_data.m_vecOrigin.IsZero() ? 0.f : sent_data.m_vecOrigin.Distance(ctx.m_local()->m_vecOrigin());
	auto choke_value = (int)ctx.m_settings.fake_lag_value;

	auto v6 = RandomInt(0, choke_value / 4);

	if (choke_value > 2 && chocked_ticks != 15)
		choke_value -= v6;// v6 * (2 * (csgo.m_globals()->tickcount & 1) - 1);

	auto v7 = 0;
	auto v8 = 2;
	auto v9 = new_velo * csgo.m_globals()->interval_per_tick;

	switch (ctx.m_settings.fake_lag_type)
	{
	case 0:
		if (chocked_ticks < choke_value)
			lag = 1;
		else
			lag = false;
		break;
	case 1:
		while (float(v7 * v9) <= 68.0f)
		{
			if (float((v8 - 1) * v9) > 68.0f)
			{
				++v7;
				break;
			}
			if (float(v8 * v9) > 68.0f)
			{
				v7 += 2;
				break;
			}
			if (float(float(v8 + 1) * v9) > 68.0f)
			{
				v7 += 3;
				break;
			}
			if (float(float(v8 + 2) * v9) > 68.0f)
			{
				v7 += 4;
				break;
			}
			v8 += 5;
			v7 += 5;
			if (v8 > 16)
				break;
		}
		if (chocked_ticks < choke_value)
			lag = 1;
		else
			lag = false;
		break;
	default:
		break;
	}

	if (!ctx.m_settings.fake_lag_enabled)
		lag = false;

	if (lag)
	{
		if (csgo.m_engine()->IsVoiceRecording() && chocked_ticks > 3)
			lag = false;

		if (chocked_ticks > 0 && (ctx.has_exploit_toggled && ctx.main_exploit > 0 && ctx.exploit_allowed && ctx.charged_commands > 4 || ctx.allow_shooting > cmd->command_number
			|| enable_delay > csgo.m_globals()->realtime))
			lag = false;
	}
	else
	{
		if (ctx.m_settings.anti_aim_enabled && !ctx.m_settings.fake_lag_enabled)
			lag = (cmd->command_number % 2) == 0;
	}

	if (lag)
		*send_packet = false;

	VIRTUALIZER_END;
}


void c_antiaimbot::work(CUserCmd* cmd, bool* send_packet)
{
	if (cmd == nullptr || ctx.m_local() == nullptr)
		return;

	VIRTUALIZER_START;

	if (ctx.m_settings.anti_aim_enabled && m_weapon()->m_reloadState() == 0 && sqrtf((cmd->forwardmove * cmd->forwardmove) + (cmd->sidemove * cmd->sidemove)) < 0.1f)
	{
		if (sqrtf((ctx.m_local()->m_vecVelocity().y * ctx.m_local()->m_vecVelocity().y) + (ctx.m_local()->m_vecVelocity().x * ctx.m_local()->m_vecVelocity().x)) < 0.1f)
		{
			auto v12 = 1.01f;/* : -1.01f;*/

			if (ctx.m_local()->m_bDucking() || ctx.m_local()->m_fFlags() & 2)
				v12 = v12 / (((ctx.m_local()->m_flDuckAmount() * 0.34f) + 1.0f) - ctx.m_local()->m_flDuckAmount());

			if (!(cmd->command_number & 1))
				v12 *= -1;

			cmd->forwardmove = v12;
		}
	}

	if (ctx.fside == 0)
		previous_side = ctx.fside = 1;

	ctx.changed_fake_side = false;

	const bool can_show = !(enable_delay > csgo.m_globals()->realtime || !m_weapon() || !ctx.m_settings.anti_aim_enabled || ctx.m_local()->m_MoveType() == 9 || ctx.m_local()->m_MoveType() == 8 || (cmd->buttons & IN_USE && !(m_weapon()->m_iItemDefinitionIndex() == WEAPON_C4 || ctx.m_local()->m_bIsDefusing())));

	ctx.allow_freestanding = ctx.side == 0;

	if (ctx.get_key_press(ctx.m_settings.anti_aim_fake_switch_key.key)) {
		ctx.fside *= -1;

		previous_side = ctx.fside;

		if (can_show) {
			ctx.changed_fake_side = true;
			ctx.active_keybinds[7].mode = 4;
		}
	}

	if (ctx.get_key_press(ctx.m_settings.anti_aim_yaw_left_switch_key.key)) {
		ctx.side = (ctx.side == 0 ? -1 : 0);

		if (can_show)
			ctx.active_keybinds[10].mode = 4;
	}

	if (ctx.get_key_press(ctx.m_settings.anti_aim_yaw_right_switch_key.key)) {
		ctx.side = (ctx.side == 1 ? -1 : 1);

		if (can_show)
			ctx.active_keybinds[9].mode = 4;
	}

	if (ctx.get_key_press(ctx.m_settings.anti_aim_yaw_backward_switch_key.key)) {
		ctx.side = (ctx.side == 2 ? -1 : 2);

		if (can_show)
			ctx.active_keybinds[8].mode = 4;
	}

	if (!can_show || ctx.onshot_aa_cmd == cmd->command_number)
		return;

	static bool force_choke = false;
	if (cmd->buttons & IN_USE)
	{
		if (m_weapon()->m_iItemDefinitionIndex() == WEAPON_C4 || ctx.m_local()->m_bIsDefusing())
		{

			if (TICKS_TO_TIME(ctx.m_local()->m_nTickBase()) > feature::anti_aim->lby_timer && csgo.m_client_state()->m_iChockedCommands == 0)
			{
				cmd->viewangles.y = Math::normalize_angle(ctx.m_local()->m_angEyeAngles().y - (180.f - ctx.fside));
				*send_packet = false;
				force_choke = true;

				cmd->viewangles.Clamp();

				return;
			}

			if (force_choke)
			{
				*send_packet = false;
				force_choke = false;
			}

			return;
		}
	}

	//#TODO: planting & defusing anti-aim to be less hittable

	change_angles(cmd, send_packet);

	VIRTUALIZER_END;
}