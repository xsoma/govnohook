#include <ragebot/prediction.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <source.hpp>
#include <props/displacement.hpp>
#include <props/weapon.hpp>
#include <misc/movement.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <visuals/visuals.hpp>
#include <misc/misc.hpp>

bool did_predict = false;
float itime = 0.f;
int new_tickbase = 0;
int last_tickbase = 0;

#define VEC_VIEW			csgo.m_game_rules()->GetViewVectors()->m_vView
#define VEC_HULL_MIN		csgo.m_game_rules()->GetViewVectors()->m_vHullMin
#define VEC_HULL_MAX		csgo.m_game_rules()->GetViewVectors()->m_vHullMax

#define VEC_DUCK_HULL_MIN	csgo.m_game_rules()->GetViewVectors()->m_vDuckHullMin
#define VEC_DUCK_HULL_MAX	csgo.m_game_rules()->GetViewVectors()->m_vDuckHullMax
#define VEC_DUCK_VIEW		csgo.m_game_rules()->GetViewVectors()->m_vDuckView

namespace Engine
{
	void Prediction::calculate_desync_delta()
	{
		auto animstate = ctx.m_local()->get_animation_state();

		feature::anti_aim->min_delta = animstate->m_aim_yaw_min;
		feature::anti_aim->max_delta = animstate->m_aim_yaw_max;

		float max_speed = 260.f;

		if (ctx.latest_weapon_data)
			max_speed = fmaxf(.001f, ctx.latest_weapon_data->max_speed);

		const auto current_origin = ctx.m_local()->m_vecOrigin();
		const auto fraction = 1.0f / csgo.m_globals()->interval_per_tick;
		auto calculated_velocity = (current_origin - Engine::Prediction::Instance()->m_vecOrigin) * fraction;

		feature::anti_aim->animation_velocity = calculated_velocity;

		feature::anti_aim->animation_speed = feature::anti_aim->animation_velocity.Length();

		if (feature::anti_aim->animation_speed >= 260.f)
			feature::anti_aim->animation_speed = 260.f;

		auto v19 = (1.923077f / max_speed) * feature::anti_aim->animation_speed;
		auto v21 = (2.9411764f / max_speed) * feature::anti_aim->animation_speed;
		auto v20 = 1.0f;
		if (v19 <= 1.0f)
		{
			if (v19 < 0.0f)
				v19 = 0.0f;
		}
		else
		{
			v19 = 1.0f;
		}
		auto v22 = 1.0f - (((animstate->m_walk_to_run_transition_state * 0.30000001f) + 0.2f) * v19);
		auto v23 = animstate->m_anim_duck_amount;
		if (v23 > 0.0f)
		{
			if (v21 <= 1.0f)
			{
				if (v21 >= 0.0f)
					v20 = (2.9411764f / max_speed) * feature::anti_aim->animation_speed;
				else
					v20 = 0.0f;
			}
			v22 = v22 + ((v20 * v23) * (0.5f - v22));
		}

		feature::anti_aim->max_delta *= v22;
		feature::anti_aim->min_delta *= v22;

		if (calculated_velocity.Length() < 5)
			feature::anti_aim->animation_speed = 0.f;
	}

	void Prediction::SetupMovement(CUserCmd* cmd)
	{
		if (!ctx.m_local() || ctx.m_local()->IsDead())
			return;

		csgo.m_prediction()->Update(csgo.m_client_state()->m_iDeltaTick, csgo.m_client_state()->m_iDeltaTick > 0, csgo.m_client_state()->m_iLastCommandAck, csgo.m_client_state()->m_iLastOutgoingCommand + csgo.m_client_state()->m_iChockedCommands);
	}

	void Prediction::PrePrediction(CUserCmd* cmd)
	{
		if (!sv_footsteps)
			sv_footsteps = csgo.m_engine_cvars()->FindVar(sxor("sv_footsteps"));

		if (!sv_min_jump_landing_sound)
			sv_min_jump_landing_sound = csgo.m_engine_cvars()->FindVar(sxor("sv_min_jump_landing_sound"));

		if (!ctx.m_local() || ctx.m_local()->IsDead())
			return;

		bFirstCommandPredicted = *(bool*)(uintptr_t(csgo.m_prediction()) + 0x18);
		m_bInPrediction = csgo.m_prediction()->m_in_prediction;
		m_flCurrentTime = csgo.m_globals()->curtime;
		m_flFrameTime = csgo.m_globals()->frametime;

		pred_error_time = *(float*)(uintptr_t(ctx.m_local()) + 0x35A4);

		m_fFlags = ctx.m_local()->m_fFlags();
		m_vecVelocity = ctx.m_local()->m_vecVelocity();
		m_flDuckAmount = ctx.m_local()->m_flDuckAmount();
		m_vecOrigin = ctx.m_local()->m_vecOrigin();
		m_MoveType = ctx.m_local()->m_MoveType();
		sv_footsteps_backup = *(float*)(uintptr_t(sv_footsteps) + 0x2C);
		sv_min_jump_landing_sound_backup = *(float*)(uintptr_t(sv_min_jump_landing_sound) + 0x2C);

		csgo.m_globals()->curtime = TICKS_TO_TIME(ctx.fixed_tickbase_backtrack);
		csgo.m_globals()->frametime = csgo.m_prediction()->m_engine_paused ? 0.f : csgo.m_globals()->interval_per_tick;
	}

	float elapsed_revolver_time;

	using MD5_PseudoRandom_t = uint32_t(__thiscall*)(uint32_t);


	void Prediction::Predict(CUserCmd* cmd)
	{
		VIRTUALIZER_START;

		if (!cmd)
			return;

		if (!ctx.m_local() || ctx.m_local()->IsDead())
			return;

		m_pWeapon = (C_WeaponCSBaseGun*)(csgo.m_entity_list()->GetClientEntityFromHandle(ctx.m_local()->m_hActiveWeapon()));

		if (!m_pWeapon)
			return;

		cmd->random_seed = reinterpret_cast<MD5_PseudoRandom_t>(Engine::Displacement::Signatures[c_signatures::MD5_PSEUDORANDOM])(cmd->command_number) & 0x7fffffff;

		ctx.is_predicting = true;
		did_predict = true;
		float value = 0.0f;

		if (sv_footsteps)
			*(float*)(uintptr_t(sv_footsteps) + 0x2C) = /**(uint32_t*) & (value) ^ */(uint32_t)sv_footsteps ^ uint32_t(value);

		if (sv_min_jump_landing_sound)
			*(float*)(uintptr_t(sv_min_jump_landing_sound) + 0x2C) = (uint32_t)sv_min_jump_landing_sound ^ 0x7F7FFFFF;

		*(BYTE*)(uintptr_t(csgo.m_prediction()) + 0x18) = 0;
		csgo.m_prediction()->m_in_prediction = true;

		ctx.m_local()->SetCurrentCommand(cmd);
		C_BaseEntity::SetPredictionRandomSeed(cmd);
		C_BaseEntity::SetPredictionPlayer(ctx.m_local());

		csgo.m_move_helper()->SetHost(ctx.m_local());
		csgo.m_prediction()->SetupMove(ctx.m_local(), cmd, csgo.m_move_helper(), &move_data);
		move_data.m_nButtons = cmd->buttons;
		move_data.m_nImpulseCommand = (unsigned __int8)cmd->impulse;
		move_data.m_flForwardMove = cmd->forwardmove;
		move_data.m_flSideMove = cmd->sidemove;
		move_data.m_flUpMove = cmd->upmove;
		move_data.m_vecAngles.x = cmd->viewangles.x;
		move_data.m_vecAngles.y = cmd->viewangles.y;
		move_data.m_vecAngles.z = cmd->viewangles.z;
		move_data.m_vecViewAngles.x = cmd->viewangles.x;
		move_data.m_vecViewAngles.y = cmd->viewangles.y;
		move_data.m_vecViewAngles.z = cmd->viewangles.z;

		csgo.m_movement()->ProcessMovement(ctx.m_local(), &move_data);
		csgo.m_prediction()->FinishMove(ctx.m_local(), cmd, &move_data);
		csgo.m_move_helper()->SetHost(nullptr);

		*(bool*)(uintptr_t(csgo.m_prediction()) + 0x18) = bFirstCommandPredicted;
		csgo.m_prediction()->m_in_prediction = m_bInPrediction;

		C_BaseEntity::SetPredictionRandomSeed(nullptr);
		C_BaseEntity::SetPredictionPlayer(nullptr);
		ctx.m_local()->SetCurrentCommand(0);

		calculate_desync_delta();

		m_flSpread = FLT_MAX;
		m_flInaccuracy = FLT_MAX;
		m_flCalculatedInaccuracy = 0.f;

		if (m_weapon()) {
			m_weapon()->UpdateAccuracyPenalty();

			m_flSpread = m_weapon()->GetSpread();
			m_flInaccuracy = m_weapon()->GetInaccuracy();

			auto is_special_weapon = (m_weapon()->m_iItemDefinitionIndex() == 9
				|| m_weapon()->m_iItemDefinitionIndex() == 11
				|| m_weapon()->m_iItemDefinitionIndex() == 38
				|| m_weapon()->m_iItemDefinitionIndex() == 40);

			auto pweapon_info = ctx.latest_weapon_data;

			if (ctx.m_local()->m_fFlags() & FL_DUCKING)
			{
				if (is_special_weapon)
					m_flCalculatedInaccuracy = pweapon_info->flInaccuracyCrouchAlt;
				else
					m_flCalculatedInaccuracy = pweapon_info->flInaccuracyCrouch;
			}
			else if (is_special_weapon)
			{
				m_flCalculatedInaccuracy = pweapon_info->flInaccuracyStandAlt;
			}
			else
			{
				m_flCalculatedInaccuracy = pweapon_info->flInaccuracyStand;
			}
		}

		m_vecPredVelocity = ctx.m_local()->m_vecVelocity();
		m_vecPrePredVelocity = m_vecVelocity;
		prev_buttons = cmd->buttons;

		if (ctx.m_local()->get_animation_state()) {
			const auto absang = ctx.m_local()->get_abs_angles();

			const auto oldposeparam = *(float*)(uintptr_t(ctx.m_local()) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48));

			auto angles = QAngle(0.f, ctx.m_local()->get_animation_state()->m_abs_yaw, 0);
			ctx.m_local()->set_abs_angles(angles);

			auto eye_pitch = Math::normalize_angle(ctx.cmd_original_angles.x);

			if (eye_pitch > 180.f)
				eye_pitch = eye_pitch - 360.f;

			eye_pitch = Math::clamp(eye_pitch, -90, 90);
			*(float*)(uintptr_t(ctx.m_local()) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48)) = Math::clamp((eye_pitch + 90.f) / 180.f, 0.0f, 1.0f);

			ctx.m_local()->force_bone_rebuild();

			const auto absorg = ctx.m_local()->get_abs_origin();
			ctx.m_local()->set_abs_origin(ctx.m_local()->m_vecOrigin());
			ctx.m_local()->SetupBonesEx(0x100);
			ctx.m_local()->set_abs_origin(absorg);

			ctx.m_local()->force_bone_cache();

			ctx.m_eye_position = ctx.m_local()->GetEyePosition(); //call weapon_shootpos

			*(float*)(uintptr_t(ctx.m_local()) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48)) = oldposeparam;
			ctx.m_local()->set_abs_angles(absang);
			//}
		}

		prev_cmd_command_num = cmd->command_number;
		if (sv_footsteps)
			*(float*)(uintptr_t(sv_footsteps) + 0x2C) = sv_footsteps_backup;
		ctx.is_predicting = false;

		VIRTUALIZER_END;
	}

	void Prediction::End()
	{
		csgo.m_globals()->curtime = m_flCurrentTime;
		csgo.m_globals()->frametime = m_flFrameTime;

		did_predict = false;
	}

	int Prediction::GetFlags()
	{
		return m_fFlags;
	}

	int Prediction::GetMoveType()
	{
		return m_MoveType;
	}

	Vector Prediction::GetVelocity()
	{
		return m_vecVelocity;
	}

	float Prediction::GetDuckAmount()
	{
		return m_flDuckAmount;
	}

	float Prediction::GetSpread()
	{
		return m_flSpread;
	}

	float Prediction::GetInaccuracy()
	{
		return m_flInaccuracy;
	}

	void Prediction::FixNetvarCompression(int time)
	{
		VIRTUALIZER_START;

		PlayerData* data = &m_Data[g_PacketManager.m_nSequence % 150];

		if (data == nullptr || !data->is_filled)
			return;

		const auto aim_punch_vel_diff = data->m_aimPunchAngleVel - ctx.m_local()->m_aimPunchAngleVel();
		const auto aim_punch_diff = data->m_aimPunchAngle - ctx.m_local()->m_aimPunchAngle();
		const auto viewpunch_diff = data->m_viewPunchAngle.x - ctx.m_local()->m_viewPunchAngle().x;
		const auto velocity_diff = data->m_vecVelocity - ctx.m_local()->m_vecVelocity();
		const auto origin_diff = data->m_vecOrigin - ctx.m_local()->m_vecOrigin();

		if (std::abs(aim_punch_diff.x) <= 0.03125f && std::abs(aim_punch_diff.y) <= 0.03125f && std::abs(aim_punch_diff.z) <= 0.03125f)
			ctx.m_local()->m_aimPunchAngle() = data->m_aimPunchAngle;

		if (std::abs(aim_punch_vel_diff.x) <= 0.03125f && std::abs(aim_punch_vel_diff.y) <= 0.03125f && std::abs(aim_punch_vel_diff.z) <= 0.03125f)
			ctx.m_local()->m_aimPunchAngleVel() = data->m_aimPunchAngleVel;

		if (std::abs(ctx.m_local()->m_vecViewOffset().z - data->m_vecViewOffset.z) <= 0.25f)
			ctx.m_local()->m_vecViewOffset().z = data->m_vecViewOffset.z;

		if (std::abs(viewpunch_diff) <= 0.03125f)
			ctx.m_local()->m_viewPunchAngle().x = data->m_viewPunchAngle.x;

		if (abs(ctx.m_local()->m_flDuckAmount() - data->m_flDuckAmount) <= 0.03125f)
			ctx.m_local()->m_flDuckAmount() = data->m_flDuckAmount;

		if (std::abs(velocity_diff.x) <= 0.03125f && std::abs(velocity_diff.y) <= 0.03125f && std::abs(velocity_diff.z) <= 0.03125f)
			ctx.m_local()->m_vecVelocity() = data->m_vecVelocity;

		if (abs(ctx.m_local()->m_flThirdpersonRecoil() - data->m_flThirdpersonRecoil) <= 0.03125f)
			ctx.m_local()->m_flThirdpersonRecoil() = data->m_flThirdpersonRecoil;

		if (abs(ctx.m_local()->m_flDuckSpeed() - data->m_flDuckSpeed) <= 0.03125f)
			ctx.m_local()->m_flDuckSpeed() = data->m_flDuckSpeed;

		if (abs(ctx.m_local()->m_flFallVelocity() - data->m_flFallVelocity) <= 0.03125f)
			ctx.m_local()->m_flFallVelocity() = data->m_flFallVelocity;

		VIRTUALIZER_END;
	}

	void RestoreData::Setup(C_BasePlayer* player)
	{
		VIRTUALIZER_START;

		if (!ctx.m_local() || ctx.m_local()->IsDead())
			return;

		m_aimPunchAngle = player->m_aimPunchAngle();
		m_aimPunchAngleVel = player->m_aimPunchAngleVel();
		m_viewPunchAngle = player->m_viewPunchAngle();

		m_vecViewOffset = player->m_vecViewOffset();
		m_vecBaseVelocity = player->m_vecBaseVelocity();
		m_vecVelocity = player->m_vecVelocity();
		m_vecOrigin = player->m_vecOrigin();
		m_vecPreviouslyPredictedOrigin = *(Vector*)(uintptr_t(player) + 0x35A8);
		m_vecNetworkOrigin = *(Vector*)(uintptr_t(player) + 0x138);
		m_flFallVelocity = player->m_flFallVelocity();
		m_flVelocityModifier = player->m_flVelocityModifier();
		m_flDuckAmount = player->m_flDuckAmount();
		m_flDuckSpeed = player->m_flDuckSpeed();
		m_iEFlags = player->m_iEFlags();

		m_surfaceFriction = player->m_surfaceFriction();
		m_flTimeLastTouchedGround = player->m_flTimeLastTouchedGround();

		m_hGroundEntity = player->m_hGroundEntity();
		m_nMoveType = player->m_MoveType();
		m_MoveCollide = player->m_MoveCollide();
		m_nFlags = player->m_fFlags();
		m_nTickBase = player->m_nTickBase();

		auto weapon = player->get_weapon();
		if (weapon) {
			m_fAccuracyPenalty = weapon->m_fAccuracyPenalty();
			m_flRecoilIndex = weapon->m_flRecoilIndex();
		}

		is_filled = true;
		VIRTUALIZER_END;
	}

	void RestoreData::Apply(C_BasePlayer* player) {
		VIRTUALIZER_START;

		if (!is_filled)
			return;

		if (!ctx.m_local() || ctx.m_local()->IsDead())
			return;

		player->m_aimPunchAngle() = m_aimPunchAngle;
		player->m_aimPunchAngleVel() = m_aimPunchAngleVel;
		player->m_viewPunchAngle() = m_viewPunchAngle;

		player->m_vecViewOffset() = m_vecViewOffset;
		player->m_vecBaseVelocity() = m_vecBaseVelocity;
		player->m_vecVelocity() = m_vecVelocity;
		player->m_iEFlags() = m_iEFlags;
		*(Vector*)(uintptr_t(player) + 0x35A8) = m_vecPreviouslyPredictedOrigin;
		player->m_vecOrigin() = m_vecOrigin;

		player->m_flFallVelocity() = m_flFallVelocity;
		player->m_flVelocityModifier() = m_flVelocityModifier;
		player->m_flDuckAmount() = m_flDuckAmount;
		player->m_flDuckSpeed() = m_flDuckSpeed;

		player->m_surfaceFriction() = m_surfaceFriction;

		player->m_hGroundEntity() = m_hGroundEntity;
		player->m_MoveType() = m_nMoveType;
		player->m_fFlags() = m_nFlags;
		player->m_nTickBase() = m_nTickBase;

		auto weapon = player->get_weapon();
		if (weapon) {
			weapon->m_fAccuracyPenalty() = m_fAccuracyPenalty;
			weapon->m_flRecoilIndex() = m_flRecoilIndex;
		}
		VIRTUALIZER_END;
	}

	void Prediction::OnRunCommand(C_BasePlayer* player)
	{
		auto local = ctx.m_local();

		if (!local || local != player /*|| ctx.is_predicting*/)
			return;

		auto data = &m_Data[g_PacketManager.m_nSequence % 150];

		data->m_aimPunchAngle = local->m_aimPunchAngle();
		data->m_vecViewOffset = local->m_vecViewOffset();
		data->m_vecVelocity = local->m_vecVelocity();
		data->m_vecOrigin = local->m_vecOrigin();
		data->m_aimPunchAngleVel = local->m_aimPunchAngleVel();

		data->m_flDuckAmount = local->m_flDuckAmount();
		data->m_flDuckSpeed = local->m_flDuckSpeed();

		data->m_viewPunchAngle = local->m_viewPunchAngle();
		data->m_vecBaseVelocity = local->m_vecBaseVelocity();
		data->m_vecOrigin = local->m_vecOrigin();

		data->m_flFallVelocity = local->m_flFallVelocity();
		data->m_flVelocityModifier = local->m_flVelocityModifier();
		data->m_hGroundEntity = local->m_hGroundEntity();
		data->m_nMoveType = local->m_MoveType();
		data->m_nFlags = local->m_fFlags();

		if (auto wpn = m_weapon(); wpn != nullptr)
		{
			data->m_fAccuracyPenalty = wpn->m_fAccuracyPenalty();
			data->m_flRecoilIndex = wpn->m_flRecoilIndex();
		}
		data->m_nTickBase = local->m_nTickBase();
		data->is_filled = true;
	}
}