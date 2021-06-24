#include <ragebot/rage_aimbot.hpp>
#include "source.hpp"
#include <props/entity.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <hooks/hooked.hpp>
#include <sdk/math/math.hpp>
#include <props/displacement.hpp>
#include <ragebot/lag_comp.hpp>
//#include <ragebot/autowall.hpp>
#include <ragebot/resolver.hpp>
//#include "game_movement.h"
#include "usercmd.hpp"
#include <antiaim/anti_aimbot.hpp>
#include <unordered_map>
#include <algorithm>
#include <menu/menu/menu.hpp>
#include <visuals/visuals.hpp>
#include <ragebot/prediction.hpp>
#include <misc/movement.hpp>
#include <visuals/sound_parser.hpp>
#include <props/displacement.hpp>
#include <props/prop_manager.hpp>

#include <thread>

#define LAG_COMPENSATION_TICKS 32

void C_Tickrecord::store(C_BasePlayer* player, bool backup)
{
	shot_this_tick = false;
	valid = false;
	dormant = false;
	bones_count = player->m_bone_count();
	bones_count = Math::clamp(bones_count, 0, 128);

	if (backup) {
		memcpy(matrixes, player->m_CachedBoneData().Base(), bones_count * sizeof(matrix3x4_t));
		valid = false;
		dormant = false;
		animated = true;
		exploit = false;
	}
	origin = player->m_vecOrigin();
	abs_origin = player->get_abs_origin();
	velocity = player->m_vecVelocity();
	animation_time = feature::lagcomp->get_interpolated_time();
	object_mins = player->OBBMins();
	object_maxs = player->OBBMaxs();
	eye_angles = player->m_angEyeAngles();
	abs_angles = player->get_abs_angles().y;
	entity_flags = player->m_fFlags();
	simulation_time = player->m_flSimulationTime();
	simulation_time_old = player->m_flOldSimulationTime();
	lower_body_yaw = player->m_flLowerBodyYawTarget();
	time_of_last_injury = player->m_flTimeOfLastInjury();
	velocity_modifier = player->m_flVelocityModifier();
	ientity_flags = player->m_iEFlags();
	duck_amt = player->m_flDuckAmount();
	ground_accel_last_time = player->m_flGroundAccelLinearFracLastTime();

	if (!backup)
		head_pos = player->get_bone_pos(8);
	if (!backup)
		desync_delta = feature::anti_aim->get_max_desync_delta(player);

	thirdperson_recoil = player->m_flThirdpersonRecoil();
	stop_to_full_run_frac = player->get_animation_state() ? player->get_animation_state()->m_walk_run_transition : 0.f;

	if (auto weapon = player->get_weapon(); weapon != nullptr && weapon)
		shot_time = weapon->m_flLastShotTime();
	else
		shot_time = -1;

	lag = TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime());

	// clamp it so we don't interpolate too far : )
	lag = Math::clamp(lag, 0, 31);

	time_delta = player->m_flSimulationTime() - player->m_flOldSimulationTime();

	if (*(void**)player && player->get_animation_state())
		memcpy(&animstate, player->get_animation_state(), 0x334);

	fill(begin(pose_paramaters), end(pose_paramaters), 0.f);
	if (!backup) {
		fill(begin(left_poses), end(left_poses), 0.f);
		fill(begin(right_poses), end(right_poses), 0.f);
	}

	if (player->get_animation_layers_count() > 0)
		memcpy(anim_layers, player->animation_layers_ptr(), 0x38 * player->get_animation_layers_count());

	breaking_lc = false;

	tickcount = ctx.current_tickcount;
	simulation_time_delay = csgo.m_client_state()->m_clockdrift_manager.m_nServerTick - TIME_TO_TICKS(player->m_flSimulationTime());

	lc_exploit = simulation_time_delay >= 12;

	if (csgo.m_client_state())
		m_tick = csgo.m_client_state()->m_clockdrift_manager.m_nServerTick;

	latency = ctx.latency[FLOW_INCOMING];

	not_desyncing = false;
	data_filled = true;
}

void C_Tickrecord::apply(C_BasePlayer* player, bool backup, bool dont_force_matrix)
{
	const auto need_rotate = !backup && resolver_index != 0 && data_filled && animated;

	player->set_abs_angles(QAngle(0, (need_rotate && !player->IsBot() ? (resolver_index == 1 ? left_side : right_side) : abs_angles), 0));
	player->m_vecOrigin() = origin;
	player->set_collision_bounds(object_mins, object_maxs);
	player->set_abs_origin(backup ? abs_origin : origin);

	if (backup && player->get_animation_layers_count() > 0)
		memcpy(player->animation_layers_ptr(), anim_layers, sizeof(C_AnimationLayer) * player->get_animation_layers_count());

	if (!dont_force_matrix || backup)
	{
		if (bones_count > 0) {

			if (!need_rotate || player->IsBot())
				memcpy(player->m_CachedBoneData().Base(), matrixes, bones_count * sizeof(matrix3x4_t));
			else
				memcpy(player->m_CachedBoneData().Base(), resolved_matrix, bones_count * sizeof(matrix3x4_t));

			player->m_bone_count() = bones_count;
			player->force_bone_cache();
		}
	}
}

void c_lagcomp::backup_players(bool restore) {

	if (!ctx.m_local() || ctx.m_local()->IsDead())
		return;

	if (restore) {
		// restore stuff.
		for (int i{ 0 }; i <= csgo.m_globals()->maxClients; ++i) {
			auto player = csgo.m_entity_list()->GetClientEntity(i);

			if (player == nullptr || player == ctx.m_local() || player->entindex() <= 0 || player->entindex() >= 64 || player->IsDead() || player->IsDormant())
				continue;

			auto data = &feature::lagcomp->records[player->entindex() - 1];

			if (!data || data->records_count <= 1 || data->player != player || !data->restore_record.data_filled)
				continue;

			data->restore_record.apply(player, true);
			data->is_restored = true;
			data->restore_record.data_filled = false;
		}
	}

	else {
		// backup stuff.
		for (int i{ 0 }; i <= csgo.m_globals()->maxClients; ++i) {
			auto player = csgo.m_entity_list()->GetClientEntity(i);

			if (player == nullptr || player == ctx.m_local() || player->entindex() <= 0 || player->entindex() >= 64 || player->IsDead() || player->IsDormant())
				continue;

			auto data = &feature::lagcomp->records[player->entindex() - 1];

			if (!data || data == nullptr || data->player != player || data->records_count <= 1)
				continue;

			data->restore_record.store(player, true);
		}
	}
}

void c_lagcomp::update_lerp()
{
	VIRTUALIZER_START;;
	static auto cl_interpolate = csgo.m_engine_cvars()->FindVar(("cl_interpolate"));
	if (atoi(cl_interpolate->GetString()) != 0) {
		static auto cl_interp_ratio = csgo.m_engine_cvars()->FindVar(("cl_interp_ratio"));
		static auto cl_interp = csgo.m_engine_cvars()->FindVar(("cl_interp"));
		static auto sv_client_min_interp_ratio = csgo.m_engine_cvars()->FindVar(("sv_client_min_interp_ratio"));
		static auto sv_client_max_interp_ratio = csgo.m_engine_cvars()->FindVar(("sv_client_max_interp_ratio"));

		static auto cl_updaterate = csgo.m_engine_cvars()->FindVar(("cl_updaterate"));
		static auto sv_minupdaterate = csgo.m_engine_cvars()->FindVar(("sv_minupdaterate"));
		static auto sv_maxupdaterate = csgo.m_engine_cvars()->FindVar(("sv_maxupdaterate"));

		auto update_rate = atoi(cl_updaterate->GetString());
		if (sv_minupdaterate && sv_maxupdaterate)
			update_rate = std::clamp(update_rate, atoi(sv_minupdaterate->GetString()), atoi(sv_maxupdaterate->GetString()));

		auto lerp_amount = (float)atof(cl_interp->GetString());
		auto lerp_ratio = (float)atof(cl_interp_ratio->GetString());
		if (lerp_ratio == 0.f)
			lerp_ratio = 1.f;

		if (sv_client_min_interp_ratio && sv_client_max_interp_ratio && atof(sv_client_min_interp_ratio->GetString()) != -1.f)
			lerp_ratio = (float)std::clamp(lerp_ratio, (float)atof(sv_client_min_interp_ratio->GetString()), (float)atof(sv_client_max_interp_ratio->GetString()));

		ctx.lerp_time = max(lerp_amount, lerp_ratio / (float)update_rate);
		return;
	}

	ctx.lerp_time = 0.f;
	VIRTUALIZER_END;
	return;
}

void c_lagcomp::update_network_info()
{
	VIRTUALIZER_START;;

	if (auto net = csgo.m_engine()->GetNetChannelInfo(); net != nullptr) {
		ctx.latency[FLOW_OUTGOING] = net->GetLatency(FLOW_OUTGOING);

		ctx.latency[FLOW_INCOMING] = net->GetLatency(FLOW_INCOMING);

		ctx.avglatency[FLOW_OUTGOING] = net->GetAvgLatency(FLOW_OUTGOING);
		ctx.avglatency[FLOW_INCOMING] = net->GetAvgLatency(FLOW_INCOMING);
	}
	else {
		ctx.latency[FLOW_OUTGOING] = csgo.m_globals()->interval_per_tick;
		ctx.latency[FLOW_OUTGOING] = 0;
	}

	VIRTUALIZER_END;
}

void c_lagcomp::simulate_movement(C_Simulationdata& data) {

	static auto sv_gravity = csgo.m_engine_cvars()->FindVar(sxor("sv_gravity"));
	static auto sv_jump_impulse = csgo.m_engine_cvars()->FindVar(sxor("sv_jump_impulse"));

	if (!(data.flags & FL_ONGROUND)) {
		data.velocity.z -= (csgo.m_globals()->interval_per_tick * sv_gravity->GetFloat() * 0.5f);
	}
	else if (data.jumped) {
		data.jumped = false;
		data.velocity.z = sv_jump_impulse->GetFloat();
		data.flags &= ~FL_ONGROUND;
	}

	// can't step up onto very steep slopes
	static const float MIN_STEP_NORMAL = 0.7f;

	if (!data.velocity.IsZero()) {
		auto* collidable = data.entity->GetCollideable();
		const Vector mins = collidable->OBBMins();
		const Vector max = collidable->OBBMaxs();

		const Vector src = data.origin;
		Vector end = src + (data.velocity * csgo.m_globals()->interval_per_tick);

		Ray_t ray;
		ray.Init(src, end, mins, max);

		CGameTrace trace;
		CTraceFilter filter;
		filter.pSkip = data.entity;

		csgo.m_engine_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);

		// CGameMovement::TryPlayerMove
		if (trace.fraction != 1) {
			// BUGFIXME: is it should be 4? ( not 2 )
			for (int i = 0; i < 2; i++) {
				// decompose velocity into plane
				data.velocity -= trace.plane.normal * data.velocity.Dot(trace.plane.normal);

				const float dot = data.velocity.Dot(trace.plane.normal);
				if (dot < 0.f) { // moving against plane
					data.velocity.x -= dot * trace.plane.normal.x;
					data.velocity.y -= dot * trace.plane.normal.y;
					data.velocity.z -= dot * trace.plane.normal.z;
				}

				end = trace.endpos + (data.velocity * (csgo.m_globals()->interval_per_tick * (1.f - trace.fraction)));

				ray.Init(trace.endpos, end, mins, max);
				csgo.m_engine_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);
				if (trace.fraction == 1)
					break;
			}
		}

		data.origin = trace.endpos;
		end = trace.endpos;
		end.z -= 2;

		ray.Init(data.origin, end, mins, max);
		csgo.m_engine_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);

		data.flags &= ~FL_ONGROUND;

		if (trace.DidHit() && trace.plane.normal.z >= MIN_STEP_NORMAL) {
			data.flags |= FL_ONGROUND;
		}

		if (data.flags & FL_ONGROUND)
			data.velocity.z = 0;
		else
			data.velocity.z -= csgo.m_globals()->interval_per_tick * sv_gravity->GetFloat() * 0.5f;
	}
}

bool c_lagcomp::is_time_delta_too_large(C_Tickrecord* record, bool ignore_deadtime)
{
	static auto sv_maxunlag = csgo.m_engine_cvars()->FindVar(sxor("sv_maxunlag"));

	auto serverTickcount = csgo.m_engine()->GetTick() + TIME_TO_TICKS(ctx.latency[FLOW_OUTGOING]);

	if (ctx.fakeducking)
		serverTickcount += 15 - csgo.m_client_state()->m_iChockedCommands;

	float total_latency = ctx.latency[FLOW_OUTGOING] + ctx.latency[FLOW_INCOMING];
	total_latency = fminf(fmaxf(total_latency, 0.0f), 1.0f);

	float correct = total_latency + ctx.lerp_time;
	correct = Math::clamp(correct, 0.f, sv_maxunlag->GetFloat());

	auto time = 0;
	if (ctx.m_local() && !ctx.m_local()->IsDead())
		time = ctx.m_local()->m_nTickBase();

	if (ctx.exploit_allowed && !ctx.fakeducking && ctx.main_exploit > 0)
		time -= ctx.shifting_amount;

	float deltaTime = abs(correct - (TICKS_TO_TIME(time) - record->simulation_time));

	if (deltaTime >= 0.2f)
		return true;

	return record->simulation_time < float(int(TICKS_TO_TIME(serverTickcount) - sv_maxunlag->GetFloat()));
}

float spawntime;

void c_lagcomp::reset(CCSGOPlayerAnimState* state)
{
	if (!state)
		return;

	state->m_last_update_frame = 0;
	state->m_step_height_left = 0;
	state->m_step_height_right = 0;

	state->m_weapon = state->m_player->get_weapon();
	state->m_weapon_last = state->m_weapon;

	state->m_weapon_last_bone_setup = state->m_weapon;
	state->m_eye_position_smooth_lerp = 0;
	state->m_strafe_change_weight_smooth_fall_off = 0;
	state->m_first_foot_plant_since_init = true;

	state->m_last_update_time = 0;
	state->m_last_update_increment = 0;

	state->m_eye_yaw = 0;
	state->m_eye_pitch = 0;
	state->m_abs_yaw = 0;
	state->m_abs_yaw_last = 0;
	state->m_move_yaw = 0;
	state->m_move_yaw_ideal = 0;
	state->m_move_yaw_current_to_ideal = 0;

	state->m_stand_walk_how_long_to_wait_until_transition_can_blend_in = 0.4f;
	state->m_stand_walk_how_long_to_wait_until_transition_can_blend_out = 0.2f;
	state->m_stand_run_how_long_to_wait_until_transition_can_blend_in = 0.2f;
	state->m_stand_run_how_long_to_wait_until_transition_can_blend_out = 0.4f;
	state->m_crouch_walk_how_long_to_wait_until_transition_can_blend_in = 0.3f;
	state->m_crouch_walk_how_long_to_wait_until_transition_can_blend_out = 0.3f;

	state->m_primary_cycle = 0;
	state->m_move_weight = 0;
	state->m_move_weight_smoothed = 0;
	state->m_anim_duck_amount = 0;
	state->m_duck_additional = 0; // for when we duck a bit after landing from a jump
	state->m_recrouch_weight = 0;

	state->m_position_current.clear();
	state->m_position_last.clear();

	state->m_velocity.clear();
	state->m_velocity_normalized.clear();
	state->m_velocity_normalized_non_zero.clear();
	state->m_velocity_length_xy = 0;
	state->m_velocity_length_z = 0;

	state->m_speed_as_portion_of_run_top_speed = 0;
	state->m_speed_as_portion_of_walk_top_speed = 0;
	state->m_speed_as_portion_of_crouch_top_speed = 0;

	state->m_duration_moving = 0;
	state->m_duration_still = 0;

	state->m_on_ground = true;

	state->m_land_anim_multiplier = 1.0f;
	state->m_left_ground_height = 0;
	state->m_landing = false;
	state->m_jump_to_fall = 0;
	state->m_duration_in_air = 0;

	state->m_walk_run_transition = 0;

	state->m_landed_on_ground_this_frame = false;
	state->m_left_the_ground_this_frame = false;
	state->m_in_air_smooth_value = 0;

	state->m_on_ladder = false;
	state->m_ladder_weight = 0;
	state->m_ladder_speed = 0;

	state->m_walk_to_run_transition_state = 0;

	state->m_defuse_started = false;
	state->m_plant_anim_started = false;
	state->m_twitch_anim_started = false;
	state->m_adjust_started = false;

	state->m_next_twitch_time = 0;

	state->m_time_of_last_known_injury = 0;

	state->m_last_velocity_test_time = 0;
	state->m_velocity_last.clear();
	state->m_target_acceleration.clear();
	state->m_acceleration.clear();
	state->m_acceleration_weight = 0;

	state->m_aim_matrix_transition = 0;
	state->m_aim_matrix_transition_delay = 0;

	state->m_flashed = 0;

	state->m_strafe_change_weight = 0;
	state->m_strafe_change_target_weight = 0;
	state->m_strafe_change_cycle = 0;
	state->m_strafe_sequence = -1;
	state->m_strafe_changing = false;
	state->m_duration_strafing = 0;

	state->m_foot_lerp = 0;

	state->m_feet_crossed = false;

	state->m_player_is_accelerating = false;

	state->m_duration_move_weight_is_too_high = 0;
	state->m_static_approach_speed = 80;

	state->m_stutter_step = 0;
	state->m_previous_move_state = 0;

	state->m_action_weight_bias_remainder = 0;

	state->m_aim_yaw_min = CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MIN;
	state->m_aim_yaw_max = CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MAX;
	state->m_aim_pitch_min = CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MIN;
	state->m_aim_pitch_max = CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MAX;

	//state->m_activity_modifiers.Purge();
	memset(&state->m_activity_modifiers_server[0], 0, 20);

	state->m_first_run_since_init = true;

	state->m_camera_smooth_height = 0;
	state->m_smooth_height_valid = false;
	state->m_last_time_velocity_over_ten = 0;
}

void c_lagcomp::update_local_animations(CUserCmd* cmd, bool* send_packet)
{
	VIRTUALIZER_START;;

	if (!ctx.m_local() || csgo.m_client_state()->m_iDeltaTick < 0)
		return;

	static auto smoothstep_bounds = [](float edge0, float edge1, float x) -> float
	{
		x = Math::clamp((x - edge0) / (edge1 - edge0), 0, 1);
		return x * x * (3 - 2 * x);
	};

	static bool lock_viewangles = false;
	static float m_flDurationInAir = 0;
	static QAngle target_angle = cmd->viewangles;
	static QAngle non_shot_target_angle = cmd->viewangles;

	if (ctx.m_ragebot_shot_nr == cmd->command_number)
	{
		lock_viewangles = true;
		target_angle = cmd->viewangles;
	}

	if (!lock_viewangles)
		target_angle = cmd->viewangles;

	auto animstate = ctx.m_local()->get_animation_state();

	if (!animstate || ctx.m_local()->IsDead() || ctx.m_local()->IsDormant()) {
		spawntime = 0;
		return;
	}

	ctx.m_local()->m_flThirdpersonRecoil() = ctx.m_local()->m_aimPunchAngleScaled().x;
	const auto current_flags = ctx.m_local()->m_fFlags();

	C_AnimationLayer old_anim_layers[14];
	memcpy(old_anim_layers, ctx.m_local()->animation_layers_ptr(), 0x38 * ctx.m_local()->get_animation_layers_count());

	if (ctx.m_local()->m_flSpawnTime() != spawntime || animstate->m_player != ctx.m_local())
	{
		ctx.m_local()->update_clientside_animations();

		ctx.fake_state = *ctx.m_local()->get_animation_state();
		memset(&ctx.fake_state.m_activity_modifiers_server[0], 0, /*sizeof(char) * */20);

		memcpy(ctx.m_local()->animation_layers_ptr(), old_anim_layers, 0x38 * ctx.m_local()->get_animation_layers_count());

		ctx.last_time_layers_fixed = 0;
		spawntime = ctx.m_local()->m_flSpawnTime();
	}

	const auto old_absvel = ctx.m_local()->m_vecAbsVelocity();
	const auto old_render_angles = ctx.m_local()->get_render_angles();
	const auto old_pose_params = ctx.m_local()->m_flPoseParameter();

	if (feature::anti_aim->animation_speed < 3)
		ctx.m_local()->m_vecAbsVelocity().clear();// (feature::anti_aim->animation_speed > 3 ? ((ctx.m_local()->m_vecOrigin() - Engine::Prediction::Instance()->m_vecOrigin) * (1.f / csgo.m_globals()->interval_per_tick)) : Vector::Zero);

	ctx.m_local()->get_render_angles() = ((ctx.main_exploit == 1 || ctx.main_exploit == 3 && ctx.double_tapped < 1) && !ctx.fakeducking && ctx.has_exploit_toggled && ctx.exploit_allowed && lock_viewangles) ? non_shot_target_angle : target_angle;
	animstate->m_move_weight = 0;

	ctx.m_local()->update_clientside_animations();

	//fix_anim_layers(cmd, ctx.m_local()->get_animation_state());

	ctx.m_local()->force_bone_rebuild();

	if (*send_packet == false) {
		memcpy(ctx.m_local()->animation_layers_ptr(), old_anim_layers, 0x38 * ctx.m_local()->get_animation_layers_count());
		return;
	}

	if (!lock_viewangles)
		non_shot_target_angle = cmd->viewangles;

	//save off real data.
	ctx.poses[ANGLE_REAL] = ctx.m_local()->m_flPoseParameter();
	ctx.angles[ANGLE_REAL] = ctx.m_local()->get_animation_state()->m_abs_yaw;
	ctx.abs_origin[ANGLE_REAL] = ctx.m_local()->get_abs_origin();

	if (*send_packet)
	{
		ctx.m_local()->SetupBonesEx();
		memcpy(ctx.local_matrix, ctx.m_local()->m_CachedBoneData().Base(), min(128, ctx.m_local()->m_bone_count()) * sizeof(matrix3x4_t));

		for (int i = 0; i < 128; i++)
			ctx.local_origin[i] = ctx.m_local()->get_abs_origin() - ctx.local_matrix[i].GetOrigin();
	}

	memcpy(ctx.local_layers[ANGLE_REAL], ctx.m_local()->animation_layers_ptr(), 0x38 * ctx.m_local()->get_animation_layers_count());

	//restore animation backup
	ctx.m_local()->m_flPoseParameter() = old_pose_params;
	memcpy(ctx.m_local()->animation_layers_ptr(), old_anim_layers, 0x38 * ctx.m_local()->get_animation_layers_count());

	if (feature::anti_aim->animation_speed < 3)
		ctx.m_local()->m_vecAbsVelocity().clear();

	ctx.m_local()->get_render_angles() = target_angle;

	const auto backup_state = *ctx.m_local()->get_animation_state();
	ctx.fake_state.m_landing = false;
	ctx.fake_state.m_last_update_frame = 0;
	*ctx.m_local()->get_animation_state() = ctx.fake_state;
	ctx.is_updating_fake = true;
	ctx.m_local()->update_clientside_animations();
	ctx.is_updating_fake = false;
	ctx.fake_state = *ctx.m_local()->get_animation_state();
	*ctx.m_local()->get_animation_state() = backup_state;

	//save off fake data.
	ctx.poses[ANGLE_FAKE] = ctx.m_local()->m_flPoseParameter();
	ctx.angles[ANGLE_FAKE] = ctx.fake_state.m_abs_yaw;
	ctx.abs_origin[ANGLE_FAKE] = ctx.m_local()->get_abs_origin();
	//csgo.m_engine()->GetViewAngles(ctx.camera_angles[ANGLE_FAKE]);
	memcpy(ctx.local_layers[ANGLE_FAKE], ctx.m_local()->animation_layers_ptr(), 0x38 * ctx.m_local()->get_animation_layers_count());

	ctx.local_layers[ANGLE_FAKE][12].m_flWeight = 0;
	ctx.local_layers[ANGLE_FAKE][3].m_flWeight = 0;

	ctx.m_local()->force_bone_rebuild();
	memcpy(ctx.m_local()->animation_layers_ptr(), ctx.local_layers[ANGLE_FAKE], 0x38 * ctx.m_local()->get_animation_layers_count());
	if (*send_packet)
	{
		ctx.m_local()->SetupBonesEx();
		memcpy(ctx.fake_matrix, ctx.m_local()->m_CachedBoneData().Base(), min(128, ctx.m_local()->m_bone_count()) * sizeof(matrix3x4_t));

		for (int i = 0; i < 128; i++)
			ctx.desync_origin[i] = ctx.m_local()->get_abs_origin() - ctx.fake_matrix[i].GetOrigin();
	}

	//restore animation backup
	ctx.m_local()->m_flPoseParameter() = old_pose_params;

	memcpy(ctx.m_local()->animation_layers_ptr(), old_anim_layers, 0x38 * ctx.m_local()->get_animation_layers_count());

	ctx.m_local()->m_vecAbsVelocity() = old_absvel;
	ctx.m_local()->get_render_angles() = old_render_angles;
	ctx.m_local()->m_iEFlags() &= ~EFL_DIRTY_ABSANGVELOCITY;

	lock_viewangles = false;
}

void c_lagcomp::build_local_bones(C_BasePlayer* local)
{
	const auto old_poses = local->m_flPoseParameter();
	local->force_bone_rebuild();
	local->m_flPoseParameter() = ctx.poses[ANGLE_REAL];
	local->set_abs_angles(QAngle(0, ctx.angles[ANGLE_REAL], 0));
	memcpy(ctx.m_local()->animation_layers_ptr(), ctx.local_layers[ANGLE_REAL], 0x38 * ctx.m_local()->get_animation_layers_count());
	local->SetupBonesEx();
	local->m_flPoseParameter() = old_poses;
}

bool c_lagcomp::can_resolve_by_anims(C_Tickrecord* record, C_Tickrecord* prev_record)
{
	if (prev_record->anim_layers[6].m_nSequence != record->anim_layers[6].m_nSequence
		|| int(record->anim_layers[6].m_flWeight * 1000.f) == int(prev_record->anim_layers[6].m_flWeight * 1000.f)
		|| record->anim_layers[12].m_flWeight != 0)
		return false;

	return true;
}

bool c_lagcomp::has_firing_animation(C_BasePlayer* m_player, C_Tickrecord* record)
{
	auto weapon = m_player->get_weapon();
	if (weapon)
	{
		int iWeaponIndex = weapon->m_iItemDefinitionIndex();
		auto act = m_player->get_sec_activity(record->anim_layers[1].m_nSequence);
		if (act == ACT_CSGO_FIRE_PRIMARY || ((act == ACT_CSGO_FIRE_SECONDARY || act == ACT_CSGO_FIRE_SECONDARY_OPT_1 || act == ACT_CSGO_FIRE_SECONDARY_OPT_2) && (iWeaponIndex == WEAPON_GLOCK || iWeaponIndex == WEAPON_REVOLVER || iWeaponIndex == WEAPON_FAMAS || weapon->is_knife())))
			return true;
	}
	return false;
}

void c_lagcomp::prepare_player_anim_update(C_BasePlayer* m_player, C_Tickrecord* record, C_Tickrecord* previous, int resolver_side)
{
	VIRTUALIZER_START;
	resolver_records* resolver_info = &feature::resolver->player_records[m_player->entindex() - 1];
	c_player_records* log = &records[m_player->entindex() - 1];

	if (previous && !previous->dormant && previous->data_filled && !record->first_after_dormancy)
	{
		m_player->get_animation_state()->m_primary_cycle = previous->m_primary_cycle;
		m_player->get_animation_state()->m_move_weight = previous->m_move_weight;

		m_player->get_animation_state()->m_strafe_sequence = previous->m_strafe_sequence;
		m_player->get_animation_state()->m_strafe_change_weight = previous->m_strafe_change_weight;
		m_player->get_animation_state()->m_strafe_change_cycle = previous->m_strafe_change_cycle;
		m_player->get_animation_state()->m_acceleration_weight = previous->m_acceleration_weight;
		memcpy(m_player->animation_layers_ptr(), previous->anim_layers, 0x38 * m_player->get_animation_layers_count());
	}
	else
	{
		if (record->entity_flags & FL_ONGROUND) {
			m_player->get_animation_state()->m_on_ground = true;
			m_player->get_animation_state()->m_landing = false;
		}

		m_player->get_animation_state()->m_primary_cycle = record->anim_layers[6].m_flCycle;
		m_player->get_animation_state()->m_move_weight = record->anim_layers[6].m_flWeight;
		m_player->get_animation_state()->m_strafe_sequence = record->m_strafe_sequence;
		m_player->get_animation_state()->m_strafe_change_weight = record->m_strafe_change_weight;
		m_player->get_animation_state()->m_strafe_change_cycle = record->m_strafe_change_cycle;
		m_player->get_animation_state()->m_acceleration_weight = record->m_acceleration_weight;
		m_player->get_animation_state()->m_duration_in_air = 0.f;
		m_player->m_flPoseParameter()[6] = 0.f;

		memcpy(m_player->animation_layers_ptr(), record->anim_layers, 0x38 * m_player->get_animation_layers_count());
		m_player->get_animation_state()->m_last_update_time = (record->simulation_time - csgo.m_globals()->interval_per_tick);
	}

	m_player->m_vecOrigin() = record->origin;
	m_player->set_abs_origin(record->origin);

	record->duck_amount_per_tick = 0.0f;
	record->lby_flicked_time = 0;
	m_player->m_flLowerBodyYawTarget() = record->lower_body_yaw;
	m_player->m_flThirdpersonRecoil() = record->thirdperson_recoil;
	record->entity_anim_flags = record->entity_flags;

	if (previous && previous->data_filled && !previous->dormant && !record->first_after_dormancy) {
		if (record->duck_amt != previous->duck_amt
			&& record->entity_flags & FL_ONGROUND
			&& previous->entity_flags & FL_ONGROUND)
		{
			auto v210 = record->time_delta - TICKS_TO_TIME(1);
			if (v210 != 0.f)
				record->duck_amount_per_tick = csgo.m_globals()->interval_per_tick / v210;
		}

		if (record->lag > 1)
		{
			m_player->m_flDuckAmount() = previous->duck_amt;

			if (record->shot_this_tick) {
				m_player->m_flThirdpersonRecoil() = previous->thirdperson_recoil;
				m_player->m_flLowerBodyYawTarget() = previous->lower_body_yaw;
			}

			record->lby_flicked_time = 0;
			record->entity_anim_flags = record->entity_flags;

			if (record->land_type > 0)
				record->entity_anim_flags = (record->entity_anim_flags & ~FL_ONGROUND);
		}
	}
	VIRTUALIZER_END;
}

void c_lagcomp::update_animation_system(C_BasePlayer* m_player, C_Tickrecord* record, C_Tickrecord* previous, int resolver_side)
{
	VIRTUALIZER_START;

	const auto backup_flags = m_player->m_fFlags();
	const auto backup_ieflags = m_player->m_iEFlags();
	const auto backup_duckamt = m_player->m_flDuckAmount();
	const auto backup_lowerbody = m_player->m_flLowerBodyYawTarget();
	const auto backup_move_weight = m_player->get_animation_state()->m_move_weight;
	const auto backup_primary_cycle = m_player->get_animation_state()->m_primary_cycle;

	static ConVar* sv_gravity = csgo.m_engine_cvars()->FindVar(sxor("sv_gravity"));
	static ConVar* sv_jump_impulse = csgo.m_engine_cvars()->FindVar(sxor("sv_jump_impulse"));

	resolver_records* resolver_info = &feature::resolver->player_records[m_player->entindex() - 1];
	c_player_records* log = &records[m_player->entindex() - 1];

	m_player->m_angEyeAngles() = record->eye_angles;
	m_player->m_angEyeAngles().z = 0.f;

	prepare_player_anim_update(m_player, record, previous, resolver_side);

	m_player->set_abs_angles(QAngle(0, m_player->get_animation_state()->m_abs_yaw, 0));

	if (record->lag > 1 && previous && previous->data_filled)
	{
		const auto velocity_per_tick = (record->velocity - previous->velocity) / record->lag;
		const auto lby_delta = fabsf(Math::angle_diff(record->eye_angles.y, record->lower_body_yaw));

		record->is_landed = false;

		for (auto i = 1; i <= record->lag; i++)
		{
			auto simulated_time = record->animation_update_start_time + TICKS_TO_TIME(i);

			auto velocity = ((velocity_per_tick * (float)i) + previous->velocity);

			if (record->duck_amount_per_tick != 0.0f)
			{
				auto v208 = ((record->duck_amt - m_player->m_flDuckAmount()) * record->duck_amount_per_tick) + m_player->m_flDuckAmount();
				m_player->m_flDuckAmount() = fminf(fmaxf(v208, 0.0f), 1.0f);
			}

			if (i == record->lag)
				simulated_time = record->simulation_time;

			if (record->land_in_cycle && !record->is_landed) // landing animation fix
			{
				if (record->land_time < simulated_time) {
					record->entity_anim_flags |= 1;
					record->is_landed = true;
					auto layer = &m_player->get_animation_layer(4);
					layer->m_flCycle = 0;
					layer->m_flWeight = 0;

				}
			}

			m_player->m_fFlags() = record->entity_anim_flags;

			record->can_rotate = resolver_side != 0 && i < record->lag;

			if (record->shot_this_tick)
			{
				if (record->shot_time <= simulated_time) {
					m_player->m_flThirdpersonRecoil() = record->thirdperson_recoil;
					m_player->m_flLowerBodyYawTarget() = record->lower_body_yaw;
				}
			}
			if (i == record->lag)
			{
				m_player->m_flDuckAmount() = fminf(fmaxf(record->duck_amt, 0.0f), 1.0f);
				m_player->m_flLowerBodyYawTarget() = record->lower_body_yaw;
				m_player->m_fFlags() = record->entity_flags;
			}

			m_player->m_vecAbsVelocity() = m_player->m_vecVelocity() = velocity;

			if (record->can_rotate) {
				auto angle = record->resolver_delta_multiplier;

				if (resolver_side < 0)
					m_player->get_animation_state()->m_abs_yaw = Math::normalize_angle(record->eye_angles.y - angle);
				else
					m_player->get_animation_state()->m_abs_yaw = Math::normalize_angle(record->eye_angles.y + angle);
			}

			ctx.updating_resolver = true;
			resolver_info->new_velocity = velocity;
			resolver_info->force_velocity = true;

			auto realtime_backup = csgo.m_globals()->realtime;
			auto curtime = csgo.m_globals()->curtime;
			auto frametime = csgo.m_globals()->frametime;
			auto absoluteframetime = csgo.m_globals()->absoluteframetime;
			auto framecount = csgo.m_globals()->framecount;
			auto tickcount = csgo.m_globals()->tickcount;
			auto interpolation_amount = csgo.m_globals()->interpolation_amount;

			int ticks = TIME_TO_TICKS(simulated_time);

			csgo.m_globals()->realtime = simulated_time;
			csgo.m_globals()->curtime = simulated_time;
			csgo.m_globals()->frametime = csgo.m_globals()->interval_per_tick;
			csgo.m_globals()->absoluteframetime = csgo.m_globals()->interval_per_tick;
			csgo.m_globals()->framecount = ticks;
			csgo.m_globals()->tickcount = ticks;
			csgo.m_globals()->interpolation_amount = 0.f;

			m_player->m_iEFlags() &= ~EFL_DIRTY_ABSVELOCITY;
			m_player->update_clientside_animations();
			m_player->m_iEFlags() = backup_ieflags;
			resolver_info->force_velocity = false;
			ctx.updating_resolver = false;

			csgo.m_globals()->realtime = realtime_backup;
			csgo.m_globals()->curtime = curtime;
			csgo.m_globals()->frametime = frametime;
			csgo.m_globals()->absoluteframetime = absoluteframetime;
			csgo.m_globals()->framecount = framecount;
			csgo.m_globals()->tickcount = tickcount;
			csgo.m_globals()->interpolation_amount = interpolation_amount;
		}
	}
	else
	{
		m_player->m_flLowerBodyYawTarget() = record->lower_body_yaw;
		auto vel = record->velocity;

		m_player->m_flDuckAmount() = fminf(fmaxf(record->duck_amt, 0.0f), 1.0f);
		m_player->m_fFlags() = record->entity_flags;
		m_player->m_iEFlags() &= ~EFL_DIRTY_ABSVELOCITY;

		if (resolver_side != 0)
		{
			auto angle = record->resolver_delta_multiplier;

			if (resolver_side < 0)
				m_player->get_animation_state()->m_abs_yaw = Math::normalize_angle(record->eye_angles.y - angle);
			else
				m_player->get_animation_state()->m_abs_yaw = Math::normalize_angle(record->eye_angles.y + angle);
		}

		auto realtime_backup = csgo.m_globals()->realtime;
		auto curtime = csgo.m_globals()->curtime;
		auto frametime = csgo.m_globals()->frametime;
		auto absoluteframetime = csgo.m_globals()->absoluteframetime;
		auto framecount = csgo.m_globals()->framecount;
		auto tickcount = csgo.m_globals()->tickcount;
		auto interpolation_amount = csgo.m_globals()->interpolation_amount;

		int ticks = TIME_TO_TICKS(record->simulation_time);

		csgo.m_globals()->realtime = record->simulation_time;
		csgo.m_globals()->curtime = record->simulation_time;
		csgo.m_globals()->frametime = csgo.m_globals()->interval_per_tick;
		csgo.m_globals()->absoluteframetime = csgo.m_globals()->interval_per_tick;
		csgo.m_globals()->framecount = ticks;
		csgo.m_globals()->tickcount = ticks;
		csgo.m_globals()->interpolation_amount = 0.f;

		m_player->m_vecAbsVelocity() = m_player->m_vecVelocity() = vel;

		ctx.updating_resolver = true;
		resolver_info->new_velocity = record->velocity;
		resolver_info->force_velocity = true;
		m_player->update_clientside_animations();
		resolver_info->force_velocity = false;
		ctx.updating_resolver = false;

		csgo.m_globals()->realtime = realtime_backup;
		csgo.m_globals()->curtime = curtime;
		csgo.m_globals()->frametime = frametime;
		csgo.m_globals()->absoluteframetime = absoluteframetime;
		csgo.m_globals()->framecount = framecount;
		csgo.m_globals()->tickcount = tickcount;
		csgo.m_globals()->interpolation_amount = interpolation_amount;
	}

	m_player->m_fFlags() = backup_flags;
	m_player->m_flDuckAmount() = backup_duckamt;
	m_player->m_flLowerBodyYawTarget() = backup_lowerbody;
	m_player->m_iEFlags() = backup_ieflags;
	m_player->get_animation_state()->m_primary_cycle = backup_primary_cycle;
	m_player->get_animation_state()->m_move_weight = backup_move_weight;

	if (resolver_side != 0) {
		if (resolver_side > 0)
		{
			record->animstate_right_params[0] = m_player->get_animation_state()->m_abs_yaw;
			record->animstate_right_params[1] = m_player->get_animation_state()->m_abs_yaw_last;
			record->animstate_right_params[2] = m_player->get_animation_state()->m_move_yaw;
			record->animstate_right_params[3] = m_player->get_animation_state()->m_move_yaw_ideal;
			record->animstate_right_params[4] = m_player->get_animation_state()->m_move_yaw_current_to_ideal;
			record->animstate_right_params[5] = m_player->get_animation_state()->m_move_weight_smoothed;
			record->animstate_right_params[6] = m_player->get_animation_state()->m_in_air_smooth_value;
			record->animstate_right_params[7] = m_player->get_animation_state()->m_time_to_align_lower_body;
		}
		else
		{
			record->animstate_left_params[0] = m_player->get_animation_state()->m_abs_yaw;
			record->animstate_left_params[1] = m_player->get_animation_state()->m_abs_yaw_last;
			record->animstate_left_params[2] = m_player->get_animation_state()->m_move_yaw;
			record->animstate_left_params[3] = m_player->get_animation_state()->m_move_yaw_ideal;
			record->animstate_left_params[4] = m_player->get_animation_state()->m_move_yaw_current_to_ideal;
			record->animstate_left_params[5] = m_player->get_animation_state()->m_move_weight_smoothed;
			record->animstate_left_params[6] = m_player->get_animation_state()->m_in_air_smooth_value;
			record->animstate_left_params[7] = m_player->get_animation_state()->m_time_to_align_lower_body;
		}
	}

	m_player->invalidate_anims(8);
	VIRTUALIZER_END;
}

void c_lagcomp::recalculate_velocity(C_Tickrecord* record, C_BasePlayer* m_player, C_Tickrecord* previous)
{
	VIRTUALIZER_START;

	static /*const*/ ConVar* sv_gravity = csgo.m_engine_cvars()->FindVar(sxor("sv_gravity"));
	static /*const*/ ConVar* sv_jump_impulse = csgo.m_engine_cvars()->FindVar(sxor("sv_jump_impulse"));
	static /*const*/ ConVar* sv_enablebunnyhopping = csgo.m_engine_cvars()->FindVar(sxor("sv_enablebunnyhopping"));

	auto log = &records[m_player->entindex() - 1];
	auto r_log = &feature::resolver->player_records[m_player->entindex() - 1];

	if (record->entity_flags & FL_ONGROUND
		&& record->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_flWeight > 0.0f
		&& record->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_flWeight < 1.0f)
	{
		// float val = clamp ( ( speed - 0.55f ) / ( 0.9f - 0.55f), 0.f, 1.f );
		// layer11_weight = 1.f - val;
		auto val = (1.0f - record->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_flWeight) * 0.35f;

		if (val > 0.0f && val < 1.0f)
			record->animation_speed = val + 0.55f;
		else
			record->animation_speed = -1.f;
	}

	if (_fdtest(&record->velocity.x) > 0
		|| _fdtest(&record->velocity.y) > 0
		|| _fdtest(&record->velocity.z) > 0)
		record->velocity.clear();

	if (!record->first_after_dormancy && previous && !previous->dormant && previous->data_filled)
	{
		//
		//	calculate new velocity based on (new_origin - old_origin) / (new_time - old_time) formula.
		//
		if (record->lag > 1 && record->lag <= 20)
			record->velocity = (record->origin - previous->origin) / record->time_delta;

		if (abs(record->velocity.x) < 0.001f)
			record->velocity.x = 0.0f;
		if (abs(record->velocity.y) < 0.001f)
			record->velocity.y = 0.0f;
		if (abs(record->velocity.z) < 0.001f)
			record->velocity.z = 0.0f;

		if (_fdtest(&record->velocity.x) > 0
			|| _fdtest(&record->velocity.y) > 0
			|| _fdtest(&record->velocity.z) > 0)
			record->velocity.clear();

		auto curr_direction = RAD2DEG(std::atan2f(record->velocity.y, record->velocity.x));
		auto prev_direction = previous == nullptr ? FLT_MAX : RAD2DEG(std::atan2f(previous->velocity.y, previous->velocity.x));

		auto delta = Math::normalize_angle(curr_direction - prev_direction);

		if (record->velocity.Length2D() > 0.1f) {
			if (previous->velocity.Length2D() > 0.1f && abs(delta) >= 60.f)
				r_log->last_time_changed_direction = csgo.m_globals()->realtime;
		}
		else
			r_log->last_time_changed_direction = 0;

		//
		// these requirements pass only when layer[6].weight is accurate to normalized velocity.
		//
		if (record->entity_flags & FL_ONGROUND
			&& record->velocity.Length2D() >= 0.1f
			&& std::abs(delta) < 1.0f
			&& std::abs(record->duck_amt - previous->duck_amt) <= 0.0f
			&& record->anim_layers[6].m_flPlaybackRate > previous->anim_layers[6].m_flPlaybackRate
			&& record->anim_layers[6].m_flWeight > previous->anim_layers[6].m_flWeight)
		{
			auto weight_speed = record->anim_layers[6].m_flWeight;

			if (weight_speed <= 0.7f && weight_speed > 0.0f)
			{
				if (record->anim_layers[6].m_flPlaybackRate == 0.0f)
					record->velocity.clear();
				else
				{
					const auto m_post_velocity_lenght = record->velocity.Length2D();

					if (m_post_velocity_lenght != 0.0f)
					{
						float mult = 1;
						if (record->entity_flags & 6)
							mult = 0.34f;
						else if (record->fake_walking)
							mult = 0.52f;

						record->velocity.x = (record->velocity.x / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
						record->velocity.y = (record->velocity.y / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
					}
				}
			}
		}

		//
		// fix velocity with fakelag.
		//
		if (record->entity_flags & FL_ONGROUND && record->velocity.Length2D() > 0.1f && record->lag > 1)
		{
			//
			// get velocity lenght from 11th layer calc.
			//
			if (record->animation_speed > 0) {
				const auto m_pre_velocity_lenght = record->velocity.Length2D();
				C_WeaponCSBaseGun* weapon = m_player->get_weapon();

				if (weapon) {
					auto wdata = weapon->GetCSWeaponData();
					if (wdata) {
						auto adjusted_velocity = (record->animation_speed * record->max_current_speed) / m_pre_velocity_lenght;
						record->velocity.x *= adjusted_velocity;
						record->velocity.y *= adjusted_velocity;
					}
				}
			}
		}

		if (log->records_count > 2 && record->lag > 1 && !record->first_after_dormancy
			&& previous->velocity.Length() > 0 && !(record->entity_flags & FL_ONGROUND && previous->entity_flags & FL_ONGROUND))
		{
			auto pre_pre_record = &log->tick_records[(log->records_count - 2) & 63];

			if (!pre_pre_record->dormant && pre_pre_record->data_filled) {
				const auto prev_direction = RAD2DEG(std::atan2f(previous->velocity.y, previous->velocity.x));

				auto real_velocity = record->velocity.Length2D();

				float delta = curr_direction - prev_direction;

				if (delta <= 180.0f)
				{
					if (delta < -180.0f)
						delta = delta + 360.0f;
				}
				else
				{
					delta = delta - 360.0f;
				}

				float v63 = delta * 0.5f + curr_direction;

				auto direction = (v63 + 90.f) * 0.017453292f;

				record->velocity.x = sinf(direction) * real_velocity;
				record->velocity.y = cosf(direction) * real_velocity;
			}
		}
		if (!(record->entity_flags & FL_ONGROUND))
		{
			record->velocity.z -= sv_gravity->GetFloat() * record->time_delta * 0.5f;

			record->in_jump = true;
		}
	}
	else if (record->first_after_dormancy)
	{
		auto weight_speed = record->anim_layers[6].m_flWeight;

		if (record->anim_layers[6].m_flPlaybackRate < 0.00001f)
			record->velocity.clear();
		else
		{
			const auto m_post_velocity_lenght = record->velocity.Length2D();

			if (m_post_velocity_lenght != 0.0f && weight_speed > 0.01f && weight_speed < 0.95f)
			{
				float mult = 1;
				if (record->entity_flags & 6)
					mult = 0.34f;
				else if (record->fake_walking)
					mult = 0.52f;

				record->velocity.x = (record->velocity.x / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
				record->velocity.y = (record->velocity.y / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
			}
		}

		if (record->entity_flags & FL_ONGROUND)
			record->velocity.z = 0;
	}

	if (_fdtest(&record->velocity.x) > 0
		|| _fdtest(&record->velocity.y) > 0
		|| _fdtest(&record->velocity.z) > 0)
		record->velocity.clear();

	if (record->entity_flags & FL_ONGROUND && record->lag > 1 && record->velocity.Length() > 0.1f && record->anim_layers[6].m_flPlaybackRate < 0.00001f)
		record->velocity.clear();

	r_log->tick_stopped = -1;
	r_log->velocity_stopped = Vector::Zero;

	m_player->m_vecVelocity() = record->velocity;
	VIRTUALIZER_END;
}

void c_lagcomp::parse_player_data(C_Tickrecord* record, C_BasePlayer* m_player)
{
	VIRTUALIZER_START;
	const auto idx = m_player->entindex() - 1;
	resolver_records* resolver_info = &feature::resolver->player_records[idx];
	c_player_records* log = &records[idx];
	auto previous = (log->records_count > 1 && !record->first_after_dormancy ? &log->tick_records[(log->records_count - 1) & 63] : nullptr);

	record->fake_walking = m_player->m_bIsWalking();

	if (fabs(Math::normalize_angle(record->thirdperson_recoil + record->eye_angles.x - 180.0f)) <= 0.1f || record->eye_angles.x == 88.9947510f)
		record->eye_angles.x = 89.0f;

	record->eye_angles.x = Math::normalize_angle(record->eye_angles.x);
	record->eye_angles.x = fminf(fmaxf(record->eye_angles.x, -90.0f), 90.0f);
	record->in_jump = !(record->entity_flags & FL_ONGROUND);

	record->exploit = false;
	record->animated = false;

	record->breaking_lc = false;
	record->shot_this_tick = false;
	record->not_desyncing = false;
	resolver_info->breaking_lc = false;
	resolver_info->last_simtime = record->simulation_time;

	record->land_time = 0.0f;
	record->is_landed = false;
	record->land_in_cycle = false;

	record->m_move_weight = record->anim_layers[6].m_flWeight;
	record->m_primary_cycle = record->anim_layers[6].m_flCycle;
	record->m_strafe_sequence = record->anim_layers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_nSequence;
	record->m_strafe_change_weight = record->anim_layers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_flWeight;
	record->m_strafe_change_cycle = record->anim_layers[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE].m_flCycle;
	record->m_acceleration_weight = record->anim_layers[ANIMATION_LAYER_LEAN].m_flWeight;

	int ticks_to_simulate = 1;

	if (previous && !previous->dormant && previous->data_filled && !record->first_after_dormancy)
	{
		int simulation_ticks = TIME_TO_TICKS(record->simulation_time - previous->simulation_time);

		if ((simulation_ticks - 1) > 31 || previous->simulation_time == 0.f)
			simulation_ticks = 1;

		auto layer_cycle = record->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_flCycle;
		auto previous_playback = previous->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate;

		if (previous_playback > 0.f && record->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate > 0.f
			&& previous->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_nSequence == record->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_nSequence)
		{
			auto previous_cycle = previous->anim_layers[11].m_flCycle;
			simulation_ticks = 0;

			if (previous_cycle > layer_cycle)
				layer_cycle = layer_cycle + 1.0f;

			while (layer_cycle > previous_cycle)
			{
				const auto ticks_backup = simulation_ticks;
				const auto playback_mult_ipt = csgo.m_globals()->interval_per_tick * previous_playback;

				previous_cycle = previous_cycle + (csgo.m_globals()->interval_per_tick * previous_playback);

				if (previous_cycle >= 1.0f)
					previous_playback = record->anim_layers[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate;

				++simulation_ticks;

				if (previous_cycle > layer_cycle && (previous_cycle - layer_cycle) > (playback_mult_ipt * 0.5f))
					simulation_ticks = ticks_backup;
			}
		}

		ticks_to_simulate = simulation_ticks;
	}

	ticks_to_simulate = Math::clamp(ticks_to_simulate, 1, 64);

	record->lag = ticks_to_simulate;
	record->time_delta = float(ticks_to_simulate) * csgo.m_globals()->interval_per_tick;

	record->animation_update_start_time = record->simulation_time - record->time_delta;

	record->had_use_key = false;

	if (previous && previous->data_filled) {
		if (record->simulation_time > record->simulation_time_old) {

			if ((record->origin - previous->origin).Length2D() > 4096.0f)
				record->breaking_lc = resolver_info->breaking_lc = true;

			// check if landed in choke cycle
			if (!record->first_after_dormancy && !previous->dormant)
			{
				if (!(record->entity_flags & 1 && previous->entity_flags & 1)) {
					if (record->anim_layers[4].m_flCycle < 0.5f) {
						record->land_time = record->simulation_time - (record->anim_layers[4].m_flCycle / record->anim_layers[4].m_flPlaybackRate);
						record->land_in_cycle = record->land_time >= record->animation_update_start_time;
					}
				}
				else
					record->velocity.z = 0;
			}

			if (!previous->dormant && record->simulation_time >= record->shot_time && record->shot_time > record->animation_update_start_time)// {
				record->shot_this_tick = true;
		}
	}


	float m_flSpeed = std::fmin(m_player->m_vecVelocity().Length(), 260.f);
	static float m_flEyeYaw = 0.f;
	static float m_flGoalFeetYaw = 0.f;
	static float m_flNextLowerBodyYawUpdateTime = 0.f;
	float m_flGoalMoveDirGoalFeetDelta = 0.f;
	Vector& vVelocity = record->animstate.m_velocity;
	if (m_flSpeed > 1.5f)
	{
		m_flNextLowerBodyYawUpdateTime = csgo.m_globals()->curtime + 0.1f;
		if (m_player->m_flLowerBodyYawTarget() != m_flEyeYaw)
			m_flEyeYaw = m_player->m_flLowerBodyYawTarget();
	}

	float m_flDelta = Math::normalize_angle(Math::AngleDiff(m_flEyeYaw, record->original_abs_yaw));

	if (resolver_info->last_low_delta < 10.f && std::fabs(m_flDelta) < 10.f)
		record->resolver_delta_multiplier = std::clamp(std::fabs(m_flDelta), 0.f, feature::anti_aim->get_max_desync_delta(m_player));
	else
	{

		if (resolver_info->last_low_delta > 0.f && resolver_info->had_low_delta == true && resolver_info->last_low_delta < 30.f)
		{
			record->resolver_delta_multiplier = std::clamp(resolver_info->last_low_delta, 0.f, feature::anti_aim->get_max_desync_delta(m_player));
		}
		else
		{
			record->resolver_delta_multiplier = std::clamp(std::fabs(m_flDelta), 0.f, feature::anti_aim->get_max_desync_delta(m_player));
		}
	}

	record->max_current_speed = m_player->m_flMaxSpeed();

	resolver_info->tick_jumped = -1;

	C_WeaponCSBaseGun* weapon = m_player->get_weapon();

	if (weapon) {
		auto wdata = weapon->GetCSWeaponData();

		if (wdata)
			record->max_current_speed = std::fminf(m_player->m_flMaxSpeed(), m_player->m_bIsScoped() ? *(float*)(uintptr_t(wdata) + 0x134) : *(float*)(uintptr_t(wdata) + 0x130));
	}

	record->max_current_speed = fmaxf(record->max_current_speed, 0.001f);

	record->animation_speed = -1.f;

	VIRTUALIZER_END;
}
void c_lagcomp::RebuiltLayer6(C_BasePlayer* player, int side, float m_abs_yaw, C_Tickrecord* record) {

	VIRTUALIZER_START;
	auto m_pState = player->get_animation_state();

	static const float CS_PLAYER_SPEED_RUN = 260.0f;

	// TODO: Find these members in the actual animstate struct
	auto m_flLastUpdateIncrement = *(float*)((DWORD)m_pState + 0x74);
	auto m_flFootYaw = m_abs_yaw;
	auto m_flMoveYaw = m_pState->m_move_yaw;
	auto m_vecVelocityNormalizedNonZero = *(Vector*)((DWORD)m_pState + 0xE0);
	auto m_flInAirSmoothValue = *(float*)((DWORD)m_pState + 0x124);

	char m_szDestination[64];
	sprintf_s(m_szDestination, sxor("move_%s"), m_pState->GetWeaponPrefix());

	int m_nMoveSequence = player->LookupSequence(m_szDestination);
	if (m_nMoveSequence == -1)
	{
		m_nMoveSequence = player->LookupSequence(sxor("move"));
	}

	float m_flMovementTimeDelta = *(float*)((DWORD)m_pState + 0x74) * 40.0f;

	if (-record->m_LayerData[side].m_flMovePlaybackRate <= m_flMovementTimeDelta)
	{
		if (-m_flMovementTimeDelta <= -record->m_LayerData[side].m_flMovePlaybackRate)
			record->m_LayerData[side].m_flMovePlaybackRate = 0.0f;
		else
			record->m_LayerData[side].m_flMovePlaybackRate = record->m_LayerData[side].m_flMovePlaybackRate - m_flMovementTimeDelta;
	}
	else
	{
		record->m_LayerData[side].m_flMovePlaybackRate = record->m_LayerData[side].m_flMovePlaybackRate + m_flMovementTimeDelta;
	}

	record->m_LayerData[side].m_flMovePlaybackRate = std::clamp(record->m_LayerData[side].m_flMovePlaybackRate, 0.0f, 100.0f);

	float m_flDuckSpeedClamped = std::clamp(*(float*)((DWORD)m_pState + 0xFC), 0.0f, 1.0f);
	float m_flRunSpeedClamped = std::clamp(*(float*)((DWORD)m_pState + 0xF8), 0.0f, 1.0f);

	float m_flSpeedWeight = ((m_flDuckSpeedClamped - m_flRunSpeedClamped) * *(float*)((DWORD)m_pState + 0xA4)) + m_flRunSpeedClamped;

	if (m_flSpeedWeight < record->m_LayerData[side].m_flFeetWeight)
	{
		float v34 = std::clamp(record->m_LayerData[side].m_flMovePlaybackRate * 0.01f, 0.0f, 1.0f);
		float m_flFeetWeightElapsed = ((v34 * 18.0f) + 2.0f) * *(float*)((DWORD)m_pState + 0x74);
		if (m_flSpeedWeight - record->m_LayerData[side].m_flFeetWeight <= m_flFeetWeightElapsed)
			record->m_LayerData[side].m_flFeetWeight = -m_flFeetWeightElapsed <= (m_flSpeedWeight - record->m_LayerData[side].m_flFeetWeight) ? m_flSpeedWeight : record->m_LayerData[side].m_flFeetWeight - m_flFeetWeightElapsed;
		else
			record->m_LayerData[side].m_flFeetWeight = m_flFeetWeightElapsed + record->m_LayerData[side].m_flFeetWeight;
	}
	else
	{
		record->m_LayerData[side].m_flFeetWeight = m_flSpeedWeight;
	}

	float m_flYaw = Math::normalize_angle((m_pState->m_move_yaw + m_abs_yaw) + 180.0f);
	Vector m_angAngle = { 0.0f, m_flYaw, 0.0f };
	Vector m_vecDirection;
	Math::AngleVectors(m_angAngle, &m_vecDirection);

	float m_flMovementSide = Math::DotProduct(m_vecVelocityNormalizedNonZero, m_vecDirection);
	record->m_LayerData[side].m_flMovementSideRight = m_flMovementSide;

	if (m_flMovementSide < 0.0f)
		m_flMovementSide = -m_flMovementSide;

	float m_flNewFeetWeight = Math::Bias(m_flMovementSide, 0.2f) * record->m_LayerData[side].m_flFeetWeight;

	float m_flNewFeetWeightWithAirSmooth = m_flNewFeetWeight * m_flInAirSmoothValue;

	// m_flLayer5Weight looks a bit weird so i decided to name it m_flLayer5_Weight instead.
	float m_flLayer5_Weight = player->animation_layer(5).m_flWeight;

	float m_flNewWeight = 0.55f;
	if (1.0f - m_flLayer5_Weight > 0.55f)
		m_flNewWeight = 1.0f - m_flLayer5_Weight;

	float m_flNewFeetWeightLayerWeight = m_flNewWeight * m_flNewFeetWeightWithAirSmooth;
	float m_flFeetCycleRate = 0.0f;

	float m_flSpeed = std::fmin(player->m_vecVelocity().Length(), CS_PLAYER_SPEED_RUN);
	if (m_flSpeed > 0.00f)
	{
		float m_flSequenceCycleRate = player->GetSequenceCycleRate(player->GetModelPtr(), m_nMoveSequence);

		float m_flSequenceMoveDist = player->GetSequenceMoveDist(player->GetModelPtr(), m_nMoveSequence);
		m_flSequenceMoveDist *= 1.0f / (1.0f / m_flSequenceCycleRate);
		if (m_flSequenceMoveDist <= 0.001f)
			m_flSequenceMoveDist = 0.001f;

		float m_flSpeedMultiplier = m_flSpeed / m_flSequenceMoveDist;
		m_flFeetCycleRate = (1.0f - (m_pState->m_walk_run_transition * 0.15f)) * (m_flSpeedMultiplier * m_flSequenceCycleRate);
	}

	float m_flFeetCyclePlaybackRate = (*(float*)((DWORD)m_pState + 0x74) * m_flFeetCycleRate);
	record->m_LayerData[side].m_flPrimaryCycle = m_flFeetCyclePlaybackRate + record->m_LayerData[side].m_flPrimaryCycle;

	// store possible information for resolving.
	record->m_LayerData[side].m_flMovementSide = m_flMovementSide;
	record->m_LayerData[side].m_flMovementSideRight = m_flMovementSide;
	record->m_LayerData[side].m_angMoveYaw = m_angAngle;
	record->m_LayerData[side].m_vecDirection = m_vecDirection;
	record->m_LayerData[side].m_flFeetWeight = m_flNewFeetWeight;

	// maybe it can be used for something, keeping it just in case.
	record->m_LayerData[side].m_nSequence = m_nMoveSequence;
	record->m_LayerData[side].m_flFeetCycleRate = m_flFeetCycleRate;
	record->m_LayerData[side].m_flPlaybackRate = m_flFeetCyclePlaybackRate;
	record->m_LayerData[side].m_flCycle = record->m_LayerData[side].m_flPrimaryCycle;
	record->m_LayerData[side].m_flWeight = std::clamp(m_flNewFeetWeightLayerWeight, 0.0f, 1.0f);

	//#ifdef VIRTUALIZER
	VIRTUALIZER_END;
	//#endif // VIRTUALIZER
}
void c_lagcomp::run_animation_data_resolver(C_Tickrecord* record, C_BasePlayer* m_player, C_Tickrecord* prerecord)
{
	VIRTUALIZER_START;
	const auto idx = m_player->entindex() - 1;

	resolver_records* resolver_info = &feature::resolver->player_records[idx];
	c_player_records* log = &records[idx];

	auto speed = record->velocity.Length2D();
	if (resolver_info->missed_shots[0] < 1)
	{
		if (speed > 1.1f)
		{
			float eye_delta = std::abs(resolver_info->resolver_layers[0][6].m_flPlaybackRate - record->anim_layers[6].m_flPlaybackRate);
			float eye_positive_delta = std::abs(resolver_info->resolver_layers[1][6].m_flPlaybackRate - record->anim_layers[6].m_flPlaybackRate);
			float eye_negative_delta = std::abs(resolver_info->resolver_layers[2][6].m_flPlaybackRate - record->anim_layers[6].m_flPlaybackRate);
			static float old_layer_12_weight = prerecord->anim_layers[12].m_flWeight;
			static float layer_12_weight = record->anim_layers[12].m_flWeight;
			static float old_torso_yaw = std::abs(prerecord->ready_animstate.m_flCurrentFeetYaw);
			static float old_lean = std::abs(prerecord->ready_animstate.m_flLeanWeight);

			static float torso_yaw = std::abs(record->ready_animstate.m_flCurrentFeetYaw);
			static float lean = std::abs(record->ready_animstate.m_flLeanWeight);
			static int last_side = 0;

			RebuiltLayer6(m_player, 0, m_player->get_animation_state()->m_abs_yaw, record);
			RebuiltLayer6(m_player, 1, m_player->get_animation_state()->m_abs_yaw + record->resolver_delta_multiplier, record);
			RebuiltLayer6(m_player, 2, m_player->get_animation_state()->m_abs_yaw - record->resolver_delta_multiplier, record);

			const auto float_matches = [](const float float1, const float float2, float tolerance = 0.002f) { return fabsf(float1 - float2) < tolerance; };
			const auto float_tolerance = [](const float float1, const float float2) { return fabsf(float1 - float2); };


			if (layer_12_weight * 1000.f < 0.99f && old_layer_12_weight * 1000.f < 0.99f)//layer 12 laen
			{
				if (old_layer_12_weight <= 0.0009f && layer_12_weight <= 0.0009f)
				{
					if (layer_12_weight >= 0.0001f && old_layer_12_weight <= 0.0004f && layer_12_weight <= 0.0004f && old_layer_12_weight <= 0.0000001f)
					{
						if (layer_12_weight > 0.00001f && old_layer_12_weight > 0.00001f)
						{
							if (old_torso_yaw > torso_yaw && old_lean > lean)
							{
								last_side = -1;
							}
							else if (old_torso_yaw < torso_yaw && old_lean < lean)
							{
								last_side = 1;
							}
						}
					}
				}
				if (eye_positive_delta > eye_negative_delta && eye_positive_delta != eye_delta)
				{
					record->resolver_index = 1;
					record->resolver_type = 3;
					resolver_info->anims_pre_resolving = record->resolver_index;
					resolver_info->anim_time = csgo.m_globals()->realtime;
					resolver_info->did_anims_update = true;
					record->animations_updated = true;
				}
				else if (eye_negative_delta > eye_positive_delta && eye_negative_delta != eye_delta)
				{
					record->resolver_index = -1;
					record->resolver_type = 3;
					resolver_info->anims_pre_resolving = record->resolver_index;
					resolver_info->anim_time = csgo.m_globals()->realtime;
					resolver_info->did_anims_update = true;
					record->animations_updated = true;
				}
				else
				{
					if (last_side == resolver_info->anims_pre_resolving)
					{
						record->resolver_index = last_side;
						record->resolver_type = 4;
						resolver_info->anim_time = csgo.m_globals()->realtime;
						resolver_info->did_anims_update = true;
						record->animations_updated = true;
					}
					else
					{
						record->resolver_index = record->freestanding_index;
						record->resolver_type = 1;
						resolver_info->anims_pre_resolving = record->resolver_index;
						resolver_info->anim_time = csgo.m_globals()->realtime;
						resolver_info->did_anims_update = true;
						record->animations_updated = true;

						last_side = resolver_info->anims_pre_resolving;
					}
				}
			}

			//record->m_LayerData[0].m_flPlaybackRate = eye_delta;
			//record->m_LayerData[1].m_flPlaybackRate = eye_positive_delta;
			//record->m_LayerData[2].m_flPlaybackRate = eye_negative_delta;
		}
		else
		{
			float delta = Math::AngleDiff(record->lower_body_yaw, record->abs_angles);

			if (delta < 65.f && delta > 0.f)
			{
				record->resolver_index = delta > 0.f ? 1 : -1;
				record->resolver_type = 5;
				resolver_info->anims_pre_resolving = delta > 0.f ? 1 : -1;
				resolver_info->anim_time = csgo.m_globals()->realtime;
				resolver_info->did_anims_update = true;
				record->animations_updated = true;
			}
			else
			{
				record->resolver_index = resolver_info->anims_pre_resolving;
				record->resolver_type = 5;
				resolver_info->anim_time = csgo.m_globals()->realtime;
				resolver_info->did_anims_update = true;
				record->animations_updated = true;
			}
		}

		//if (speed < 5.0f)
		//{
		//	record->resolver_index = record->freestanding_index;
		//	record->resolver_type = 1;
		//	resolver_info->anims_pre_resolving = record->freestanding_index;
		//	resolver_info->anim_time = csgo.m_globals()->realtime;
		//	resolver_info->did_anims_update = true;
		//	record->animations_updated = true;
		//}
		//else
		//{
		//	//RebuiltLayer6(m_player, 0, m_player->get_animation_state()->m_abs_yaw, record);
		//	//RebuiltLayer6(m_player, 1, m_player->get_animation_state()->m_abs_yaw + record->resolver_delta_multiplier, record);
		//	//RebuiltLayer6(m_player, 2, m_player->get_animation_state()->m_abs_yaw - record->resolver_delta_multiplier, record);

		//	record->resolver_index = record->freestanding_index;
		//	record->resolver_type = 1;
		//	resolver_info->anims_pre_resolving = record->freestanding_index;
		//	resolver_info->anim_time = csgo.m_globals()->realtime;
		//	resolver_info->did_anims_update = true;
		//	record->animations_updated = true;
		//}

	}
	else
	{
		switch (resolver_info->missed_shots[0] - 1 % 3)
		{
		case 0:
			record->resolver_index = -resolver_info->anims_pre_resolving;
			record->resolver_type = 10;
			resolver_info->anim_time = csgo.m_globals()->realtime;
			resolver_info->did_anims_update = true;
			record->animations_updated = true;
			break;
		case 1:
			record->resolver_index = 0;
			record->resolver_type = 11;
			resolver_info->anim_time = csgo.m_globals()->realtime;
			resolver_info->did_anims_update = true;
			record->animations_updated = true;
			break;
		case 2:
			record->resolver_index = resolver_info->anims_pre_resolving;
			record->resolver_type = 12;
			resolver_info->anim_time = csgo.m_globals()->realtime;
			resolver_info->did_anims_update = true;
			record->animations_updated = true;
			break;
		}
	}

	VIRTUALIZER_END;
}

void c_lagcomp::update_animations_data(C_Tickrecord* record, C_BasePlayer* m_player)
{
	VIRTUALIZER_START;

	CCSGOPlayerAnimState* state = m_player->get_animation_state();

	const auto idx = m_player->entindex() - 1;

	if (m_player == ctx.m_local())
		return;

	if (!state) {
		m_player->update_clientside_animations();
		return;
	}

	record->valid = true;
	record->animations_updated = false;
	record->accurate_anims = false;
	resolver_records* resolver_info = &feature::resolver->player_records[idx];
	c_player_records* log = &records[idx];

	resolver_info->did_store_abs_yaw = false;
	resolver_info->last_angle = record->eye_angles;
	record->resolved = false;

	auto previous = (log->records_count > 1 && !record->first_after_dormancy ? &log->tick_records[(log->records_count - 1) & 63] : nullptr);

	const auto backup_origin = m_player->m_vecOrigin();
	const auto backup_m_velocity = m_player->m_vecVelocity();
	const auto backup_m_abs_velocity = m_player->m_vecAbsVelocity();
	const auto backup_m_abs_angles = m_player->get_abs_angles();
	const auto backup_m_flags = m_player->m_fFlags();
	const auto backup_m_eflags = m_player->m_iEFlags();
	const auto backup_m_duck = m_player->m_flDuckAmount();
	const auto backup_m_body = m_player->m_flLowerBodyYawTarget();
	const auto backup_m_absorigin = m_player->get_abs_origin();
	const auto backup_m_tp_recoil = m_player->m_flThirdpersonRecoil();
	const auto backup_animstate = *m_player->get_animation_state();
	const auto backup_poses = m_player->m_flPoseParameter();

	record->resolver_delta_multiplier = 58.f;
	RandomSeed(csgo.m_client_state()->m_clockdrift_manager.m_nServerTick & 255);

	parse_player_data(record, m_player);
	recalculate_velocity(record, m_player, previous);

	update_animation_system(m_player, record, previous, 0);
	record->pose_paramaters = m_player->m_flPoseParameter();
	record->stop_to_full_run_frac = m_player->get_animation_state()->m_walk_run_transition;

	resolver_info->original_abs_yaw = m_player->get_animation_state()->m_abs_yaw;
	record->original_abs_yaw = m_player->get_animation_state()->m_abs_yaw;
	memcpy(resolver_info->resolver_layers[0], m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());

	resolver_info->last_desync_delta = record->desync_delta;
	record->animstate = *m_player->get_animation_state();
	record->ready_animstate = *m_player->GetAnimState();

	m_player->set_abs_angles(QAngle(0, m_player->get_animation_state()->m_abs_yaw, 0));
	memcpy(m_player->animation_layers_ptr(), record->anim_layers, 0x38 * m_player->get_animation_layers_count());

	m_player->force_bone_rebuild();
	m_player->SetupBonesEx();

	memcpy(record->matrixes, m_player->m_CachedBoneData().Base(), m_player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

	*m_player->get_animation_state() = backup_animstate;
	memcpy(m_player->m_flPoseParameter().data(), backup_poses.data(), 80);

	update_animation_system(m_player, record, previous, -1);

	resolver_info->right_side = record->right_side = m_player->get_animation_state()->m_abs_yaw;
	record->right_poses = m_player->m_flPoseParameter();
	memcpy(resolver_info->resolver_layers[1], m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
	memcpy(m_player->animation_layers_ptr(), record->anim_layers, 0x38 * m_player->get_animation_layers_count());

	if (record->valid) {
		m_player->set_abs_angles(QAngle(0, m_player->get_animation_state()->m_abs_yaw, 0));
		m_player->force_bone_rebuild();
		m_player->SetupBonesEx();

		memcpy(record->rightmatrixes, m_player->m_CachedBoneData().Base(), m_player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
	}

	*m_player->get_animation_state() = backup_animstate;
	memcpy(m_player->m_flPoseParameter().data(), backup_poses.data(), 80);

	update_animation_system(m_player, record, previous, 1);

	resolver_info->left_side = record->left_side = m_player->get_animation_state()->m_abs_yaw;
	record->left_poses = m_player->m_flPoseParameter();
	memcpy(resolver_info->resolver_layers[2], m_player->animation_layers_ptr(), 0x38 * m_player->get_animation_layers_count());
	memcpy(m_player->animation_layers_ptr(), record->anim_layers, 0x38 * m_player->get_animation_layers_count());

	if (record->valid) {
		m_player->set_abs_angles(QAngle(0, m_player->get_animation_state()->m_abs_yaw, 0));
		m_player->force_bone_rebuild();
		m_player->SetupBonesEx();

		memcpy(record->leftmatrixes, m_player->m_CachedBoneData().Base(), m_player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
	}

	record->bones_count = m_player->m_CachedBoneData().Count();
	record->not_desyncing = false; //!(abs(Math::angle_diff(record->right_side, record->original_abs_yaw)) > 0.0f || abs(Math::angle_diff(record->left_side, record->original_abs_yaw)) > 0.0f) || record->lag < 1;

	if (previous && previous->data_filled && !previous->dormant)
		run_animation_data_resolver(record, m_player, previous);

	if (record->resolver_index == 0 || m_player->IsBot()) {
		m_player->get_animation_state()->m_abs_yaw = record->original_abs_yaw;
	}
	else {
		if (record->resolver_index > 0)
			m_player->get_animation_state()->m_abs_yaw = Math::normalize_angle(m_player->m_angEyeAngles().y + record->resolver_delta_multiplier);
		else
			m_player->get_animation_state()->m_abs_yaw = Math::normalize_angle(m_player->m_angEyeAngles().y - record->resolver_delta_multiplier);
	}
	if (record->resolver_index == 0 || m_player->IsBot()) {
		m_player->m_flPoseParameter() = record->pose_paramaters;
	}
	else {
		m_player->m_flPoseParameter() = (record->resolver_index > 0 ? record->left_poses : record->right_poses);
	}
	m_player->set_abs_angles(QAngle(0, m_player->get_animation_state()->m_abs_yaw, 0));
	m_player->force_bone_rebuild();
	m_player->SetupBonesEx();

	memcpy(record->resolved_matrix, m_player->m_CachedBoneData().Base(), m_player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));


	/*if (record->resolver_index == 0) {
		m_player->m_flPoseParameter() = record->pose_paramaters;
	}
	else {
		m_player->m_flPoseParameter() = (record->resolver_index > 0 ? record->left_poses : record->right_poses);
	}*/

	//memcpy(m_player->m_CachedBoneData().Base(), (record->resolver_index == 0) ? record->matrixes : (record->resolver_index > 0 ? record->leftmatrixes : record->rightmatrixes), m_player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

	record->abs_angles = record->animstate.m_abs_yaw;

	m_player->set_abs_origin(backup_m_absorigin);

	memcpy(m_player->animation_layers_ptr(), record->anim_layers, 0x38 * m_player->get_animation_layers_count());

	record->animation_time = feature::lagcomp->get_interpolated_time();
	log->interpolated_count = 0;

	m_player->m_vecVelocity() = record->velocity;

	record->head_pos = m_player->get_bone_pos(8);

	m_player->m_vecOrigin() = backup_origin;
	m_player->m_fFlags() = backup_m_flags;
	m_player->m_iEFlags() = backup_m_eflags;
	m_player->m_flDuckAmount() = backup_m_duck;
	m_player->m_flThirdpersonRecoil() = backup_m_tp_recoil;
	m_player->m_flLowerBodyYawTarget() = backup_m_body;
	record->animated = true;
	VIRTUALIZER_END;
}

void c_lagcomp::interpolate(ClientFrameStage_t stage)
{
	return;

	for (auto i = 1; i < csgo.m_globals()->maxClients; i++)
	{
		auto m_player = csgo.m_entity_list()->GetClientEntity(i);

		if (!m_player || m_player->entindex() <= 0 || m_player->entindex() > 63 || m_player == ctx.m_local() || m_player->IsDormant() || m_player->IsDead() || m_player->m_iTeamNum() == ctx.m_local()->m_iTeamNum() || !m_player->IsPlayer())
			continue;

		auto log = &records[m_player->entindex() - 1];

		if (!log
			|| log->records_count < 2
			|| log->player != m_player)
			continue;

		auto rlog = &feature::resolver->player_records[m_player->entindex() - 1];
		auto newest_record = &log->tick_records[log->records_count & 63];

		if (!log->is_restored
			|| newest_record->first_after_dormancy
			|| rlog->breaking_lc
			|| newest_record->exploit
			|| abs(csgo.m_client_state()->m_clockdrift_manager.m_nServerTick - newest_record->m_tick) <= 2)
			continue;

		auto delta = m_player->get_abs_origin() - m_player->m_vecOrigin();

		if (delta.LengthSquared() > 1.f)
		{
			const auto o_origin = m_player->m_vecOrigin();
			m_player->m_vecOrigin() = m_player->get_abs_origin();
			m_player->force_bone_rebuild();
			m_player->SetupBonesEx();
			m_player->m_vecOrigin() = o_origin;
		}

		++log->interpolated_count;
	}
}

int GetCorrectionTicks()
{
	float v1; // st7
	float v4; // xmm0_4
	float v6; // [esp+0h] [ebp-10h]

	static auto sv_clockcorrection_msecs = csgo.m_engine_cvars()->FindVar(sxor("sv_clockcorrection_msecs"));

	if (!sv_clockcorrection_msecs || csgo.m_globals()->maxClients <= 1)
		return -1;

	v1 = sv_clockcorrection_msecs->GetFloat();
	v4 = 1.0f;
	v6 = v1 / 1000.0f;
	if (v6 <= 1.0f)
	{
		v4 = 0.0f;
		if (v6 >= 0.0f)
			v4 = v1 / 1000.0f;
	}
	return ((v4 / csgo.m_globals()->interval_per_tick) + 0.5f);
}

void c_lagcomp::store_records(ClientFrameStage_t stage)
{
	VIRTUALIZER_START;

	if (stage != FRAME_NET_UPDATE_END)
		return;

	if (!ctx.m_local())
		return;

	if (stage == FRAME_NET_UPDATE_END)
	{
		auto v18 = GetCorrectionTicks();

		if (v18 == -1)
			ctx.last_cmd_delta = 0;
		else
		{
			auto v19 = csgo.m_client_state()->m_clockdrift_manager.m_nServerTick; //csgo.m_engine()->GetTick();

			auto m_sim_ticks = TIME_TO_TICKS(ctx.m_local()->m_flSimulationTime());

			if (m_sim_ticks > TIME_TO_TICKS(ctx.m_local()->m_flOldSimulationTime())
				&& abs(m_sim_ticks - v19) <= v18)
				ctx.last_cmd_delta = m_sim_ticks - v19;
		}
	}

	if (ctx.m_local()->IsDead()) {
		if (spawntime > 0)
			spawntime = 0;
	}

	for (auto i = 0; i < csgo.m_globals()->maxClients; i++)
	{
		auto player = csgo.m_entity_list()->GetClientEntity(i);

		if (!player || player == ctx.m_local() || !player->GetClientClass())
			continue;

		net_update(player);
	}
	VIRTUALIZER_END;
}

void c_lagcomp::net_update(C_BasePlayer* m_player)
{
	VIRTUALIZER_START;

	player_info pinfo;
	const auto idx = m_player->entindex() - 1;

	const auto& curlog = &records[idx];
	const auto& resolver_log = &feature::resolver->player_records[idx];

	if (!m_player->GetClientClass()
		|| m_player->GetClientClass()->m_ClassID != class_ids::CCSPlayer
		|| m_player->m_iTeamNum() == ctx.m_local()->m_iTeamNum()
		|| m_player->get_animation_layers_count() <= 0)
		return;

	if (!csgo.m_engine()->GetPlayerInfo(idx + 1, &pinfo))
		return;

	if (ctx.m_local()->IsDead())
	{
		m_player->client_side_animation() = true;
		ctx.updating_anims = true;
		ctx.setup_bones = true;
		return;
	}
	else
	{
		ctx.updating_anims = false;
		ctx.setup_bones = false;
		m_player->client_side_animation() = true;
	}

	if (curlog->player != m_player || pinfo.userid != curlog->userid) {
		curlog->reset();
		resolver_log->reset();
		ctx.shots_fired[idx] = 0;
		ctx.shots_total[idx] = 0;

		resolver_log->missed_shots[R_SHOT] = 0;
		resolver_log->missed_shots[R_60_DELTA] = 0;
		resolver_log->missed_shots[R_40_DELTA] = 0;
		resolver_log->missed_shots[R_USUAL] = 0;
	}

	curlog->player = m_player;
	curlog->userid = pinfo.userid;
	curlog->saved_info = pinfo;

	const auto reset = m_player->IsDead() || m_player->m_iHealth() <= 0;

	// if this happens, delete all the lagrecords.
	if (reset)
	{
		m_player->client_side_animation() = true;
		curlog->reset();
		resolver_log->reset();
		ctx.shots_fired[idx] = 0;
		resolver_log->missed_shots[R_SHOT] = 0;
		resolver_log->missed_shots[R_60_DELTA] = 0;
		resolver_log->missed_shots[R_40_DELTA] = 0;
		resolver_log->missed_shots[R_USUAL] = 0;
		ctx.shots_total[idx] = 0;
		curlog->saved_info.userid = -1;
		return;
	}

	if (m_player->IsDormant())
	{
		if (curlog->records_count > 0) {
			// get reference to latest added record.
			auto current = &curlog->tick_records[curlog->records_count & 63];

			// mark as dormant.
			if (!current->dormant)
				current->dormant = true;
		}

		//curlog->m_cur_sim = 0;
		curlog->records_count = 0;

		return;
	}

	int player_updated = false;
	int invalid_simulation = false;

	if (m_player->m_flSimulationTime() != 0.0f)
	{
		if (m_player->get_animation_layer(11).m_flCycle != curlog->m_sim_cycle
			|| m_player->get_animation_layer(11).m_flPlaybackRate != curlog->m_sim_rate)
			player_updated = 1;
		else
		{
			m_player->m_flOldSimulationTime() = curlog->m_old_sim;
			invalid_simulation = 1;
			m_player->m_flSimulationTime() = curlog->m_cur_sim;
		}
	}
	else
		return;

	bool silent_update = false;

	auto update = 0;
	if (!invalid_simulation)
	{

		auto v23 = curlog->m_cur_sim;
		curlog->m_old_sim = v23;
		auto v24 = m_player->m_flSimulationTime();
		curlog->m_cur_sim = v24;
		if (player_updated || v24 != v23 && (curlog->m_cur_sim == 0))
			update = 1;

		if (player_updated && v24 == v23)
			silent_update = true;
	}

	if (update || silent_update)
	{
		++curlog->records_count;

		curlog->m_sim_cycle = m_player->get_animation_layer(11).m_flCycle;
		curlog->m_sim_rate = m_player->get_animation_layer(11).m_flPlaybackRate;

		// add new record.
		auto current = Encrypted_t<C_Tickrecord>(&curlog->tick_records[curlog->records_count & 63]);

		current->store(m_player, false);

		current->record_index = curlog->records_count;
		curlog->render_origin = current->origin;

		current->dormant = false;
		current->first_after_dormancy = curlog->records_count <= 1 || curlog->m_cur_sim < 1;
		current->animated = false;

		update_animations_data(current.get(), m_player);

		if (silent_update)
			current->valid = false;
	}
	VIRTUALIZER_END;
}

void c_lagcomp::reset()
{
	for (auto i = 0; i < 64; i++) {
		records[i].reset();
	}
}

float c_lagcomp::get_interpolated_time()
{
	VIRTUALIZER_START;;

	auto v20 = csgo.m_globals()->tickcount;

	if (ctx.m_local() && !ctx.m_local()->IsDead())
		v20 = ctx.m_local()->m_nFinalPredictedTick();// 0x00003434

	if (ctx.exploit_allowed && !ctx.fakeducking && ctx.main_exploit > 0)
		v20 -= ctx.next_shift_amount;

	float time = ((TICKS_TO_TIME(v20) - csgo.m_globals()->interval_per_tick)
		+ (csgo.m_globals()->interpolation_amount * csgo.m_globals()->interval_per_tick))
		- (ctx.lerp_time + (ctx.latency[FLOW_OUTGOING] + ctx.latency[FLOW_INCOMING]));

	VIRTUALIZER_END;

	return time;
};