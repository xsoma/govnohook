#include <ragebot/resolver.hpp>
#include "sdk.hpp"
#include "source.hpp"
#include <ragebot/rage_aimbot.hpp>
#include <ragebot/autowall.hpp>
#include <visuals/visuals.hpp>
#include <misc/misc.hpp>
#include <props/displacement.hpp>
#include <hooks/hooked.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <ragebot/prediction.hpp>
#include <props/weapon.hpp>
float AngleDiff(float angle_1, float angle_2)
{
	float delta = 0.f; // xmm1_4

	for (; angle_1 > 180.0f; angle_1 = angle_1 - 360.0f)
		;
	for (; angle_1 < -180.0f; angle_1 = angle_1 + 360.0f)
		;
	for (; angle_2 > 180.0f; angle_2 = angle_2 - 360.0f)
		;
	for (; angle_2 < -180.0f; angle_2 = angle_2 + 360.0f)
		;
	for (delta = (angle_2 - angle_1); delta > 180.0f; delta = (delta - 360.0f))
		;
	for (; delta < -180.0f; (delta = delta + 360.0f))
		;
	return delta;
}

bool c_resolver::add_shot(const Vector& shotpos, const Vector& shotpoint, C_Tickrecord* record, const int damage, const int hitgroup, const int& enemy_index)
{
	bool result = false;

	static auto last_tick = 0;

	const auto outgoing = ctx.latency[FLOW_OUTGOING];
	const auto incoming = ctx.latency[FLOW_INCOMING];
	const auto serverTickcount = csgo.m_globals()->tickcount + (ctx.fakeducking ? 15 - csgo.m_client_state()->m_iChockedCommands : 0) + TIME_TO_TICKS(outgoing + incoming);

	if (serverTickcount != last_tick)
	{
		shots.emplace_back(shotpos, shotpoint, serverTickcount, csgo.m_globals()->realtime, hitgroup, damage, enemy_index, record);

		result = true;
		last_tick = serverTickcount;
	}

	return result;
}

void c_resolver::update_missed_shots(const ClientFrameStage_t& stage)
{
	if (stage != FRAME_NET_UPDATE_START)
		return;

	auto it = shots.begin();
	while (it != shots.end())
	{
		const auto& shot = *it;
		if (abs(shot.tick - csgo.m_globals()->tickcount) > 40)
		{
			it = shots.erase(it);
		}
		else
		{
			++it;
		}
	}

	auto it2 = current_shots.begin();
	while (it2 != current_shots.end())
	{
		const auto& shot = *it2;
		if (abs(shot.tick - csgo.m_globals()->tickcount) > 40)
		{
			it2 = current_shots.erase(it2);
		}
		else
		{
			++it2;
		}
	}
}

std::deque<shot_t>& c_resolver::get_shots()
{
	return shots;
}

void c_resolver::hurt_listener(IGameEvent* game_event)
{
	if (shots.empty() || !ctx.m_local() || ctx.m_local()->IsDead())
		return;

	//_(attacker_s, "attacker");
	//_(userid_s, "userid");
	//_(hitgroup_s, "hitgroup");
	//_(dmg_health_s, "dmg_health");

	const auto attacker = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("attacker")));
	const auto victim = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));
	const auto hitgroup = game_event->GetInt(sxor("hitgroup"));
	const auto damage = game_event->GetInt(sxor("dmg_health"));

	if (attacker != csgo.m_engine()->GetLocalPlayer())
		return;

	if (victim == csgo.m_engine()->GetLocalPlayer())
		return;

	auto player = csgo.m_entity_list()->GetClientEntity(victim);
	if (!player || player->m_iTeamNum() == ctx.m_local()->m_iTeamNum())
		return;

	if (unapproved_shots.empty())
		return;

	for (auto& shot : unapproved_shots)
	{
		if (!shot.hurt)
		{
			shot.hurt = true;
			shot.hitinfo.victim = victim;
			shot.hitinfo.hitgroup = hitgroup;
			shot.hitinfo.damage = damage;
			return;
		}
	}
}

shot_t* c_resolver::closest_shot(int tickcount)
{
	shot_t* closest_shot = nullptr;
	auto closest_diff = 64;
	for (auto& shot : shots)
	{
		const auto diff = abs(tickcount - shot.tick);
		if (diff <= closest_diff)
		{
			closest_shot = &shot;
			closest_diff = diff;
			continue;
		}


		break;
	}

	return closest_shot;
}

bool c_resolver::is_record_equal(C_Tickrecord* a1, C_Tickrecord* a2)
{
	if (a1->shot_this_tick != a2->shot_this_tick)
		return 0;
	else if (a1->left_side != a2->left_side || a1->right_side != a2->right_side)
		return 0;
	if (a1->origin.x != a2->origin.x
		|| a1->origin.y != a2->origin.y
		|| a1->origin.z != a2->origin.z
		|| a1->eye_angles.x != a2->eye_angles.x
		|| a1->eye_angles.y != a2->eye_angles.y
		|| a1->eye_angles.z != a2->eye_angles.z
		|| a1->entity_flags != a2->entity_flags
		|| a1->animations_updated != a2->animations_updated
		|| a1->object_maxs.z != a2->object_maxs.z)
	{
		return 0;
	}
	return 1;
}


C_Tickrecord* c_resolver::find_shot_record(C_BasePlayer* player, c_player_records* data) {
	resolver_records* r_log = &feature::resolver->player_records[player->entindex() - 1];

	if (data->records_count < 1)
		return nullptr;

	C_Tickrecord* previous = 0;

	// iterate records.
	for (auto iter = 0; iter <= min(50, data->records_count); iter++) {

		auto it = &data->tick_records[iter & 63];

		if (!it
			|| !it->data_filled
			|| !it->animated
			|| !it->valid
			//|| it->first_after_dormancy
			|| feature::lagcomp->is_time_delta_too_large(it))
			continue;

		if (it->lc_exploit || r_log->breaking_lc && iter >= 1)
			return nullptr;

		if (previous && is_record_equal(previous, it)) {
			previous = it;
			continue;
		}

		if (it->dormant)
			break;

		// try to find a record with a shot, walking or no anti-aim.
		if (it->shot_this_tick || (fabs(it->eye_angles.x) < 25 || it->animations_updated) /*&& (player->get_weapon() == nullptr || fabs(player->get_weapon()->m_flLastShotTime() - it->simulation_time) > TICKS_TO_TIME(6))*/)
			return it;

		previous = it;
		//max_velocity_modifier = it->velocity_modifier;
	}

	// none found above, return the first valid record if possible.
	return nullptr;
}

C_Tickrecord* c_resolver::find_first_available(C_BasePlayer* player, c_player_records* data, bool oldest) {
	resolver_records* r_log = &feature::resolver->player_records[player->entindex() - 1];

	if (data->records_count < 1)
		return nullptr;

	if (!oldest) {
		// iterate records.
		for (auto iter = min(50, data->records_count); iter > 1; --iter) {

			auto it = &data->tick_records[iter & 63];

			if (!it
				|| !it->data_filled
				|| !it->animated
				|| !it->valid
				|| r_log->breaking_lc && iter > 0
				|| feature::lagcomp->is_time_delta_too_large(it))
				continue;

			if (it->dormant)
				break;

			return it;
		}
	}
	else
	{
		// iterate records.
		for (auto iter = 0; iter <= min(50, data->records_count); ++iter) {

			auto it = &data->tick_records[iter & 63];

			if (it->breaking_lc && iter >= 1)
				break;

			if (!it
				|| it->dormant
				|| !it->data_filled
				|| !it->animated
				|| !it->valid
				//|| it->first_after_dormancy 
				|| feature::lagcomp->is_time_delta_too_large(it))
				continue;

			return it;
		}
	}

	return nullptr;
}

void c_resolver::record_shot(IGameEvent* game_event)
{
	//_(userid_s, "userid");

	const auto userid = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));
	auto player = csgo.m_entity_list()->GetClientEntity(userid);

	if (!player)
		return;

	if (player != ctx.m_local())
		return;

	const auto shot = closest_shot(csgo.m_globals()->tickcount);

	if (!shot)
		return;

	current_shots.push_back(*shot);
}

void c_resolver::listener(IGameEvent* game_event)
{
	constexpr auto weap_fire = hash_32_fnv1a_const("weapon_fire");

	if (hash_32_fnv1a_const(game_event->GetName()) == weap_fire)
	{
		record_shot(game_event);
		return;
	}

	if (current_shots.empty())
		return;

	const auto userid = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));
	auto player = csgo.m_entity_list()->GetClientEntity(userid);

	if (!player || player != ctx.m_local())
		return;

	const Vector pos(game_event->GetFloat(sxor("x")), game_event->GetFloat(sxor("y")), game_event->GetFloat(sxor("z")));

	//if (vars.misc.impacts.get<bool>())
	//	g_pDebugOverlay->AddBoxOverlay(pos, Vector(-2, -2, -2), Vector(2, 2, 2), QAngle(0, 0, 0), 0, 0, 155, 127, 4);

	auto shot = &current_shots[0];

	static int last_tick = 0;
	static auto last_length = 0.f;
	static auto counter = 0;

	if (last_tick == csgo.m_globals()->tickcount)
		counter++;
	else
	{
		counter = 0;
		last_length = 0.f;
	}

	if (last_tick == csgo.m_globals()->tickcount && shot->shotpos.Distance(pos) < last_length)
		return;

	last_length = shot->shotpos.Distance(pos);

	if (counter)
		unapproved_shots.pop_front();

	shot->hitpos = pos;

	unapproved_shots.push_back(*shot);

	last_tick = csgo.m_globals()->tickcount;
}

bool c_resolver::select_next_side(C_BasePlayer* m_player, C_Tickrecord* record)
{
	resolver_records* r_log = &player_records[m_player->entindex() - 1];
	c_player_records* log = &feature::lagcomp->records[m_player->entindex() - 1];

	int next_resolving_method = 0;
	int next_resolving_type = 0;

	r_log->next_resolving_index.clear();

	auto was_triggered = false;

	const auto base_attarg = Math::CalcAngle(record->origin, ctx.m_local()->get_abs_origin()).y;
	auto at_target_2 = (int)Math::angle_diff(base_attarg, record->eye_angles.y);
	auto opposite_angle = 0;

	if (record->shot_this_tick)
	{
		auto left_side_diff = fabsf(Math::AngleDiff(record->left_side, record->original_abs_yaw));
		auto right_side_diff = fabsf(Math::AngleDiff(record->right_side, record->original_abs_yaw));

		//opposite lby detection
		if (fabsf(left_side_diff - right_side_diff) > 30.f/*desync_diff > (record->desync_delta * 1.1f)*/)
		{
			opposite_angle = (left_side_diff > right_side_diff ? 1 : 2);
			was_triggered = true;
		}
	}



	if (r_log->missed_shots[0] > 0)
	{
		switch (r_log->missed_shots[0] % 3)
		{
		case 0:
			record->resolver_index = r_log->last_resolved_side;
			record->resolver_type = 4;
			break;
		case 1:
			record->resolver_index = -r_log->last_resolved_side;
			record->resolver_type = 5;
			break;
		case 2:
			record->resolver_index = 0;
			record->resolver_type = 6;
			break;
		}
	}
	else
	{
		if (!record->shot_this_tick && record->animations_index != 0) {

			record->resolver_index = record->animations_updated ? record->animations_index : r_log->anims_pre_resolving;
			record->resolver_type = 1;
			r_log->last_resolved_side = record->resolver_index;
		}
		else if (!record->shot_this_tick && record->animations_index == 0) {

			record->resolver_index = record->freestanding_index;
			record->resolver_type = 3;
			r_log->last_resolved_side = record->resolver_index;
		}
	}

	return true;
}
std::string side_so_string(int h)
{
	switch (h)
	{
	case -1:
		return sxor("negative");
		break;
	case 1:
		return sxor("positive");
		break;
	default:
		return sxor("unresolved");
		break;
	}
}
std::string mode_so_string(int h)
{
	switch (h)
	{
	case -1:
		return sxor("unresolved");
		break;
	case 0:
		return sxor("unresolved");
		break;
	case 1:
		return sxor("freestand");
		break;
	case 2:
		return sxor("weight");
		break;
	case 3:
		return sxor("layer pbk");
		break;
	case 4:
		return sxor("pre/layer weight");
		break;
	case 5:
		return sxor("lby");
		break;
	case 10:
	case 11:
	case 12:
		return sxor("bruteforce");
		break;
	}
}
void c_resolver::approve_shots(const ClientFrameStage_t& stage)
{
	if (stage != FRAME_NET_UPDATE_START)
		return;

	for (auto& shot : unapproved_shots)
	{
		if (!shot.record.data_filled)
		{
			if (shot.hurt)
			{
				const auto player = csgo.m_entity_list()->GetClientEntity(shot.hitinfo.victim);
				if (player)
				{
					continue;
				}

			}

			continue;
		}

		auto player = csgo.m_entity_list()->GetClientEntity(shot.enemy_index);
		if (!player || player->IsDormant() || player->IsDead())
		{
			continue;
		}

		const auto r_log = &player_records[shot.enemy_index - 1];
		const auto resolver_log = &feature::resolver->player_records[shot.enemy_index - 1];
		const auto log = &feature::lagcomp->records[shot.enemy_index - 1];

		shot.backup.store(player, true);
		shot.record.apply(player, false);

		bool didhit[4];
		//bool didawall[4] = {};
		auto did_intersect_anything = false;

		bool occulusion_miss = false;

		const auto impact_dist = (shot.hitpos - shot.shotpos).Length();
		const auto hitbox_dist = (shot.shotpoint - shot.shotpos).Length();
		const auto origin_dist = (shot.record.origin - shot.shotpos).Length();

		QAngle shot_angle = Math::CalcAngle(shot.shotpos, shot.shotpoint);
		QAngle impact_angle = Math::CalcAngle(shot.shotpos, shot.hitpos);

		auto shot_impact_delta = float(fabs(Math::angle_diff(shot_angle.y, impact_angle.y)) + fabs(Math::angle_diff(shot_angle.x, impact_angle.x)));/*fabs((shot_angle.x + shot_angle.y) - (impact_angle.x + impact_angle.y))*/;

		auto forward = shot.hitpos - shot.shotpos;
		forward.Normalize();

		auto lol = shot.shotpos + (forward * 8192.f);

		trace_t trace, trace_zero, trace_first, trace_second;
		Ray_t ray;

		ray.Init(shot.shotpos, shot.hitpos);
		csgo.m_engine_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, player, &trace);

		auto occulusion = (fmaxf(origin_dist, hitbox_dist) - 41.f) > impact_dist && int(fabs(hitbox_dist - impact_dist)) > 41;

		if (!occulusion)
			shot.hit = trace.m_pEnt == player;
		else
			shot.hit = false;

		if (shot.hurt)
			shot.hit = true;
		static auto nospread = csgo.m_engine_cvars()->FindVar(sxor("weapon_accuracy_nospread"));

		char text[255];
		if (shot.hit || nospread->GetBool())
		{
			sprintf_s(text, sxor("missed shot due to resolver [side: %s | mode: %s | delta: %.0f | low delta: %s | layer0_playback: %.5f | layer1_playback: %.5f | layer2_playback: %.5f | layer_real_playback: %.5f]\0"),
				side_so_string(shot.record.resolver_index).c_str(),
				mode_so_string(shot.record.resolver_type).c_str(),
				shot.record.resolver_delta_multiplier,
				resolver_log->had_low_delta ? "true" : "false",
				shot.record.m_LayerData[0].m_flWeight,
				shot.record.m_LayerData[1].m_flWeight,
				shot.record.m_LayerData[2].m_flWeight,
				shot.record.anim_layers[6].m_flWeight
			);
		}
		else
		{
			sprintf_s(text, sxor("missed shot due to spread [occulusion: %s | delta: %.2f]\0"),
				occulusion ? "true" : "false",
				shot_impact_delta
			);
		}
		if (!shot.hurt) {
			if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_log_misses_due_to_spread)
				_events.emplace_back(text);
		}

		//if (!log->saved_info.fakeplayer)
		collect_and_correct_info(&shot);

		shot.backup.apply(player, true);
	}

	current_shots.clear();
	unapproved_shots.clear();
}

bool c_resolver::hurt_resolver(shot_t* shot)
{
	auto record = &shot->record;
	auto player = csgo.m_entity_list()->GetClientEntity(shot->enemy_index);

	if (record == nullptr || player == nullptr || !record->data_filled || record->dormant || ctx.m_local() == nullptr || ctx.m_local()->IsDead() || player->IsDead() || player->IsDormant())
		return false;

	auto start = shot->shotpos;
	auto end = shot->hitpos;

	if (!shot->hurt || !shot->hit) //lol
		return false;

	/*for (auto imp : record->_impacts)
	{
		if (point_in_bbox(imp, record->_tickrecord.origin, record->_tickrecord.object_mins, record->_tickrecord.object_maxs)) {
			end = imp;
			break;
		}
	}*/

	//const auto angle_impact = Math::CalcAngle(start, end);

	C_Tickrecord backup = C_Tickrecord(player, true);
	record->apply(player, false, true);

	//CTraceFilter _filter;
	//_filter.pSkip = ctx.m_local();
	trace_t tr;
	Ray_t ray;

	Vector forward = (end - start).Normalized();
	//Math::AngleVectors(angle_impact, &forward);
	auto endpos = start + forward * 8196.f;

	ray.Init(start, endpos);

	bool failed = false;

	memcpy(player->m_CachedBoneData().Base(), record->leftmatrixes, record->bones_count * sizeof(matrix3x4_t));
	csgo.m_engine_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, player, &tr);

	const auto left_tr_hg = tr.hitgroup;
	const auto left_tr_endpos = tr.endpos;

	if (tr.m_pEnt != player)
		failed = true;

	memcpy(player->m_CachedBoneData().Base(), record->rightmatrixes, record->bones_count * sizeof(matrix3x4_t));
	csgo.m_engine_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, player, &tr);

	const auto right_tr_hg = tr.hitgroup;
	const auto right_tr_endpos = tr.endpos;

	if (tr.m_pEnt != player)
		failed = true;

	memcpy(player->m_CachedBoneData().Base(), record->matrixes, record->bones_count * sizeof(matrix3x4_t));
	csgo.m_engine_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, player, &tr);

	const auto zero_tr_hg = tr.hitgroup;
	const auto zero_tr_endpos = tr.endpos;

	if (tr.m_pEnt != player)
		failed = true;

	backup.apply(player, true);

	if (failed)
		return false;

	auto r_log = &player_records[player->entindex() - 1];
	const auto prev_resolving = record->resolver_index;

	// safety check if both sides are same hitgroup we should not even try to do any kind of that bs
	if ((left_tr_hg == right_tr_hg || left_tr_hg == zero_tr_hg) && left_tr_hg == shot->hitinfo.hitgroup)
	{
		//const auto zero_diff	= abs((zero_tr_endpos - end).LengthSquared());
		//const auto right_diff	= abs((right_tr_endpos - end).LengthSquared());
		//const auto left_diff = abs((left_tr_endpos - end).LengthSquared());

		//if (right_diff != zero_diff && left_diff != zero_diff && right_diff != left_diff)
		//{
		//	//ctx.shots_fired[player->entindex() - 1] = 0;

		//	if (min(right_diff, left_diff) < zero_diff)
		//	{
		//		if (right_diff < left_diff)
		//			r_log->last_hitpos_side = 2;
		//		else
		//			r_log->last_hitpos_side = 1;
		//	}
		//	else
		//		r_log->last_hitpos_side = 0;

		//	r_log->last_hitbox_correction_was_onshot = record->shot_this_tick;
		//	_events.push_back(_event(std::string("resolving method changed due to hitbox missmatch [i]:[" + std::to_string(prev_resolving) + "]:[" + std::to_string(r_log->last_hitpos_side) + "]")));
		//	return true;
		//}
		//else
		//{
			//_events.push_back(_event("none impacts are right."));
		return false;
	}

	//_events.push_back(_event(std::string("resolving method changed due to hitbox missmatch [i]:[" + std::to_string(prev_resolving) + "]:[" + std::to_string(r_log->last_hitpos_side) + "]")));

	//return true;
//}

	if (right_tr_hg == shot->hitinfo.hitgroup)
		r_log->last_hitpos_side = 2; //- 60.f
	else if (left_tr_hg == shot->hitinfo.hitgroup)
		r_log->last_hitpos_side = 1; //+ 60.f
	else
		r_log->last_hitpos_side = 0; //+ 0.f

	r_log->last_hitbox_correction_was_onshot = record->shot_this_tick;

	//_events.push_back(_event(std::string("resolving method changed due to hitbox missmatch [" + std::to_string(prev_resolving) + "]:[" + std::to_string(r_log->last_hitpos_side) + "]")));

	return true;
}

void c_resolver::collect_and_correct_info(shot_t* shot)
{
	const auto r_log = &player_records[shot->enemy_index - 1];
	const auto log = &feature::lagcomp->records[shot->enemy_index - 1];
	auto& shots = ctx.shots_fired[shot->enemy_index - 1];

	if (shot->hurt)
	{
		shots--;

		//_events.push_back({ "corrected shot count " + std::to_string(shots) });

		if (r_log->shots_missed[shot->enemy_index] > 0)
			return;
	}
	else {
		if (shot->hit) {
			r_log->missed_shots[R_USUAL]++;
			r_log->last_shot_missed = true;
		}
		else
		{
			if (shots > 0) {
				shots--;
			}
		}
	}
}