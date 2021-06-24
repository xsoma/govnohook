#include <hooks/hooked.hpp>
#include <props/displacement.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <ragebot/prediction.hpp>
#include <misc/movement.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <misc/misc.hpp>
#include <intrin.h>
#include "usercmd.hpp"
#include <ragebot/lag_comp.hpp>
#include <ragebot/rage_aimbot.hpp>
//#include <misc/music_player.hpp>
#include <visuals/grenades.hpp>
#include <ragebot/resolver.hpp>
#include <visuals/visuals.hpp>
#include <menu/menu/menu.hpp>
#include <legitbot/aimbot.hpp>
#include "lua/Clua.h"
#include <misc/modify_packet.h>

bool IsTickcountValid(int tick) {
	return (ctx.cmd_tickcount + 64 + 8) > tick;
}
void ApplyShift(CUserCmd* cmd, bool* bSendPacket) {
	if (*bSendPacket) {
		INetChannel* net_channel = (INetChannel*)(csgo.m_client_state()->m_ptrNetChannel);
		auto v7 = net_channel->choked_packets;
		if (v7 >= 0) {
			auto v8 = ctx.ticks_allowed;
			auto v9 = cmd->command_number - v7;
			do
			{
				auto v10 = &csgo.m_input()->m_pCommands[cmd->command_number - 150 * (v9 / 150) - v7];
				if (!v10
					|| IsTickcountValid(v10->tick_count))
				{
					if (--v8 <= 0)
						v8 = 0;
					ctx.ticks_allowed = v8;
				}
				++v9;
				--v7;
			} while (v7 >= 0);
		}
	}

	auto v14 = ctx.ticks_allowed;
	if (v14 > 16) {
		auto v15 = v14 - 1;
		ctx.ticks_allowed = Math::clamp(v15, 0, 16);
	}
}
void UpdateRevolverCock(CUserCmd* m_cmd) {
	if (m_weapon()->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
		return;

	static auto last_checked = 0;
	static auto last_spawn_time = 0.f;
	static auto tick_cocked = 0;
	static auto tick_strip = 0;

	const auto max_ticks = TIME_TO_TICKS(.25f) - 1;
	const auto tick_base = TIME_TO_TICKS(csgo.m_globals()->curtime);

	if (ctx.m_local()->m_flSpawnTime() != last_spawn_time) {
		last_spawn_time = ctx.m_local()->m_flSpawnTime();
		tick_cocked = tick_base;
		tick_strip = tick_base - max_ticks - 1;
	}

	if (m_weapon()->m_flNextPrimaryAttack() > TICKS_TO_TIME(ctx.fixed_tickbase_backtrack)) {
		m_cmd->buttons &= ~IN_ATTACK;
		ctx.m_revolver_fire = false;
		return;
	}

	if (last_checked == tick_base)
		return;

	last_checked = tick_base;
	ctx.m_revolver_fire = false;

	if (tick_base - tick_strip > 2 && tick_base - tick_strip < 14)
		ctx.m_revolver_fire = true;

	if (m_cmd->buttons & IN_ATTACK && ctx.m_revolver_fire)
		return;

	m_cmd->buttons |= IN_ATTACK;

	if (m_weapon()->m_flNextSecondaryAttack() >= csgo.m_globals()->curtime)
		m_cmd->buttons |= IN_ATTACK2;

	if (tick_base - tick_cocked > max_ticks * 2 + 1) {
		tick_cocked = tick_base;
		tick_strip = tick_base - max_ticks - 1;
	}

	const auto cock_limit = tick_base - tick_cocked >= max_ticks;
	const auto after_strip = tick_base - tick_strip <= max_ticks;

	if (cock_limit || after_strip) {
		tick_cocked = tick_base;
		m_cmd->buttons &= ~IN_ATTACK;

		if (cock_limit)
			tick_strip = tick_base;
	}
}
void normalize_angles(QAngle& angles)
{
	while (angles.x > 89.0f)
		angles.x -= 180.0f;

	while (angles.x < -89.0f)
		angles.x += 180.0f;

	while (angles.y < -180.0f)
		angles.y += 360.0f;

	while (angles.y > 180.0f)
		angles.y -= 360.0f;

	angles.z = 0.0f;
}
void CopyCommand(CUserCmd* cmd, int tickbase_shift)
{
	static auto cl_forwardspeed = csgo.m_engine_cvars()->FindVar(sxor("cl_forwardspeed"));
	static auto cl_sidespeed = csgo.m_engine_cvars()->FindVar(sxor("cl_sidespeed"));
	auto shift_data = &Engine::Prediction::Instance()->m_tickbase_array[cmd->command_number % 150];


	if (cmd->forwardmove >= 5.0f)
		cmd->forwardmove = cl_forwardspeed->GetFloat();
	else if (cmd->forwardmove <= -5.0f)
		cmd->forwardmove = -cl_forwardspeed->GetFloat();

	if (cmd->sidemove >= 5.0f)
		cmd->sidemove = cl_sidespeed->GetFloat();
	else if (cmd->sidemove <= -5.0f)
		cmd->sidemove = -cl_sidespeed->GetFloat();


	auto commands_to_add = 0;

	do
	{
		auto sequence_number = commands_to_add + cmd->command_number;

		auto command = csgo.m_input()->GetUserCmd(sequence_number);
		auto verified_command = csgo.m_input()->GetVerifiedUserCmd(sequence_number);

		memcpy(command, cmd, sizeof(CUserCmd)); //-V598

		if (command->tick_count != INT_MAX && csgo.m_client_state()->m_iDeltaTick > 0)
			csgo.m_prediction()->Update(csgo.m_client_state()->m_iDeltaTick, true, csgo.m_client_state()->m_iLastCommandAck, csgo.m_client_state()->m_iLastOutgoingCommand + csgo.m_client_state()->m_iChockedCommands);

		command->command_number = sequence_number;
		command->hasbeenpredicted = command->tick_count != INT_MAX;

		++csgo.m_client_state()->m_iChockedCommands; //-V807

		if (csgo.m_client_state()->m_ptrNetChannel)
		{
			++csgo.m_client_state()->m_ptrNetChannel->choked_packets;
			++csgo.m_client_state()->m_ptrNetChannel->out_sequence_nr;
		}

		normalize_angles(command->viewangles);

		memcpy(&verified_command->m_cmd, command, sizeof(CUserCmd)); //-V598
		verified_command->m_crc = command->GetChecksum();

		++commands_to_add;
	} while (commands_to_add != tickbase_shift);

	ctx.speed_hacking = false;
	ctx.doubletap_now = false;

	*(bool*)((uintptr_t)csgo.m_prediction() + 0x24) = true;
	*(int*)((uintptr_t)csgo.m_prediction() + 0x1C) = 0;
}

namespace Hooked
{
	bool __stdcall CreateMove(float flInputSampleTime, CUserCmd* cmd)
	{
		VIRTUALIZER_START;

		ctx.fix_modify_eye_pos = false;

		//ctx.latest_weapon_data = nullptr;
		auto& prediction = Engine::Prediction::Instance();
		auto& movement = Engine::Movement::Instance();

		bool cant_repredict = false;

		if (!cmd || !ctx.m_local())
			return false;

		bool original = vmt.m_clientmode->VCall<bool(__stdcall*)(float, CUserCmd*)>(Index::IBaseClientDLL::CreateMove)(flInputSampleTime, cmd);

		if (original) {
			csgo.m_engine()->SetViewAngles(cmd->viewangles);
			csgo.m_prediction()->SetLocalViewAngles(cmd->viewangles);
		}

		if (cmd->command_number == 0)
			return false;

		g_PacketManager.m_aCommands = cmd;
		g_PacketManager.m_bResetModifablePacket = false;
		g_PacketManager.m_nSequence = cmd->command_number;

		if (modify_packet->Init(&g_PacketManager, cmd->command_number))
		{
			cmd = g_PacketManager.m_aCommands;

			feature::lagcomp->update_lerp();
			feature::lagcomp->update_network_info();

			//if (ctx.buy_weapons)
			//{
			//	std::string buy;

			//	ctx.ticks_allowed = 0;
			//	buy += "buy awp; ";


			//	if (!buy.empty())
			//		csgo.m_engine()->ClientCmd_Unrestricted(buy.c_str()); //TODO: maybe find another method how to autobuy? since this one is ghetto asf

			//	ctx.has_scar = false;
			//	ctx.buy_weapons = false;
			//}

			ctx.fix_modify_eye_pos = true;
			ctx.applied_tickbase = false;
			bool send_packet = true;
			static bool did_shift_before = false;
			ctx.start_switching = false;
			ctx.did_shot = false;
			ctx.exploit_tickbase_shift = 0;
			ctx.shift_amount = 0;
			ctx.max_weapon_speed = 250.f;
			ctx.can_aimbot = true;
			bool skip_fakelags = false;
			feature::anti_aim->skip_fakelag_this_tick = false;
			Engine::Movement::Instance()->did_force = false;

			ctx.weapon = ctx.m_local()->get_weapon()->WeaponGroup();

			ctx.active_keybinds[14].mode = 0;

			if (csgo.m_game_rules()) {
				ctx.active_keybinds[0].mode = 0;
				if (ctx.fakeducking)
				{
					if (!csgo.m_game_rules()->IsValveDS())
						ctx.active_keybinds[0].mode = ctx.m_settings.anti_aim_fakeduck_key.mode + 1;
				}
			}

			ctx.doubletap_charged = false;
			ctx.is_cocking = false;
			ctx.is_able_to_shoot = false;
			ctx.cmd = cmd;
			ctx.cmd_original_angles = cmd->viewangles;
			ctx.cmd_tickcount = cmd->tick_count;

			ctx.cmd_original_buttons = cmd->buttons;
			ctx.current_realtime = csgo.m_globals()->realtime;
			ctx.tickrate = 1.f / csgo.m_globals()->interval_per_tick;

			const auto is_switching_weapons = cmd->weaponselect != 0;

			if (feature::anti_aim->did_shot_in_chocked_cycle && csgo.m_client_state()->m_iChockedCommands == 0)
				feature::anti_aim->did_shot_in_chocked_cycle = false;

			if (is_switching_weapons || feature::menu->_menu_opened || ctx.m_local()->m_bWaitForNoAttack() || ctx.m_settings.fake_lag_shooting && feature::anti_aim->did_shot_in_chocked_cycle) {
				cmd->buttons &= ~IN_ATTACK;
			}

			if (ctx.m_local() && m_weapon() != nullptr) {
				ctx.latest_weapon_data = m_weapon()->GetCSWeaponData();
				ctx.max_weapon_speed = (!ctx.m_local()->m_bIsScoped() ? *(float*)(uintptr_t(ctx.latest_weapon_data) + 0x130) : *(float*)(uintptr_t(ctx.latest_weapon_data) + 0x134));
			}

			ctx.m_eye_position.clear();

			const auto aim_toggled = ctx.m_settings.aimbot_enabled;
			ctx.allows_aimbot = ctx.m_settings.aimbot_enabled;

			if (ctx.m_local() && ctx.m_local()->GetClientClass() && !ctx.m_local()->IsDead())
			{
				if (!vmt.m_clientstate)
					vmt.m_clientstate = std::make_shared<Memory::VmtSwap>((CClientState*)(uint32_t(csgo.m_client_state()) + 8));

				if (vmt.m_clientstate)
				{
					vmt.m_clientstate->Hook(&Hooked::PacketEnd, 6);
					vmt.m_clientstate->Hook(&Hooked::PacketStart, 5);
					//vmt.m_clientstate->Hook(&Hooked::ProcessPacket, 59);
				}

				if (!vmt.m_net_channel)
				    vmt.m_net_channel = std::make_shared<Memory::VmtSwap>((DWORD**)csgo.m_client_state()->m_ptrNetChannel);

				if (vmt.m_net_channel)
				{
					vmt.m_net_channel->Hook(&Hooked::SendNetMsg, 40);
					vmt.m_net_channel->Hook(&Hooked::SendDatagram, 46);//fix
					vmt.m_net_channel->Hook(&Hooked::ProcessPacketNet, 39);
					vmt.m_net_channel->Hook(&Hooked::Shutdown, 27);
				}

				static auto old_value_fc = !ctx.m_settings.other_esp_crosshair;
				static int old_fc = INT_MAX;

				if (old_value_fc != ctx.m_settings.other_esp_crosshair) {
					static auto* m_iCrosshairData = csgo.m_engine_cvars()->FindVar(sxor("weapon_debug_spread_show"));

					if (old_fc == INT_MAX)
						old_fc = m_iCrosshairData->GetInt();

					if (ctx.m_settings.other_esp_crosshair && !ctx.m_local()->m_bIsScoped())
						m_iCrosshairData->SetValue(3);
					else
						m_iCrosshairData->SetValue(old_fc);

					old_value_fc = ctx.m_settings.other_esp_crosshair;
				}

				ctx.last_usercmd = cmd; // we don't need usercmds from non-reliable ticks 

				auto exploit_toggled = (!ctx.has_exploit_toggled ? 0 : ctx.main_exploit);


				ctx.fps = 1 / csgo.m_globals()->frametime;

				feature::misc->pre_prediction(cmd);

				ctx.active_keybinds[4].mode = 0;

				movement->Begin(cmd, send_packet);
				movement->did_force = false;

				ctx.last_velocity_modifier_tick = cmd->tick_count;
				ctx.g_bOverrideModifier = true;
				prediction->PrePrediction(cmd);
				prediction->SetupMovement(cmd);

				if (csgo.m_game_rules() != nullptr && !csgo.m_game_rules()->IsValveDS()/* && cmd->buttons & IN_DUCK*/ || ctx.fakeducking)
					cmd->buttons |= IN_BULLRUSH;

				ctx.fakeduck_will_choke = 0;

				if (ctx.fakeducking)
				{
					if (!ctx.fakeducking_prev_state)
					{
						if (ctx.m_local()->m_flDuckAmount() > 0)
							cmd->buttons |= IN_DUCK;

						send_packet = true;
					}
					else
					{
						if (csgo.m_client_state()->m_iChockedCommands <= 6)
							cmd->buttons &= ~IN_DUCK;
						else
							cmd->buttons |= IN_DUCK;

						// credits: onetap
						ctx.fakeduck_will_choke = (14 - csgo.m_client_state()->m_iChockedCommands);

						send_packet = (csgo.m_client_state()->m_iChockedCommands >= 14);
					}

					ctx.accurate_max_previous_chocked_amt = 14;

					ctx.next_shift_amount = 0;
					ctx.shift_amount = 0;
				}

				const Vector old_move = Vector(cmd->forwardmove, cmd->sidemove, cmd->upmove);
				const bool pressed_b4 = cmd->buttons & 0x20000u;

				if (m_weapon()) {
					if (ctx.do_autostop)
						feature::ragebot->autostop(cmd, m_weapon());
				}

				movement->FixMove(cmd, movement->m_qAnglesView);

				prediction->Predict(cmd);

				C_BasePlayer* viewmodel = csgo.m_entity_list()->GetClientEntityFromHandle(
					(ctx.m_local()->m_hViewModel()));

				prediction->m_hWeapon = 0;

				if (viewmodel)
				{
					prediction->m_hWeapon = ((C_BaseViewModel*)viewmodel)->get_viewmodel_weapon();
					prediction->m_nViewModelIndex = ((C_BaseViewModel*)viewmodel)->m_nViewModelIndex();
					prediction->m_nSequence = ((C_BaseViewModel*)viewmodel)->m_nSequence();

					prediction->networkedCycle = viewmodel->m_flCycle();
					prediction->m_nAnimationParity = ((C_BaseViewModel*)viewmodel)->m_nAnimationParity();
					prediction->animationTime = ((C_BaseViewModel*)viewmodel)->m_flModelAnimTime();
				}

				//feature::grenades->think(cmd);
				feature::grenade_tracer->think(cmd);

				ctx.active_keybinds[2].mode = 0;
				ctx.active_keybinds[5].mode = 0;
				ctx.active_keybinds[6].mode = 0;
				ctx.active_keybinds[7].mode = 0;
				ctx.active_keybinds[10].mode = 0;
				ctx.active_keybinds[12].mode = 0;
				ctx.allow_freestanding = false;
				if (ctx.get_key_press(ctx.m_settings.anti_aim_freestand_key)) {
					ctx.active_keybinds[12].mode = ctx.m_settings.anti_aim_freestand_key.mode + 1;
					ctx.allow_freestanding = true;
				}

				ctx.active_keybinds[9].mode = 0;
				ctx.active_keybinds[8].mode = 0;


				if (m_weapon() != nullptr && ctx.m_local()->m_MoveType() != 10)
				{
					feature::lagcomp->backup_players(false);

					if (ctx.m_settings.aimbot_enabled)
						feature::anti_aim->fake_lagv2(cmd, &send_packet);
					else
					{
						if (ctx.m_settings.anti_aim_enabled)
						{
							if (csgo.m_client_state()->m_iChockedCommands == 0)
								send_packet = false;
						}
					}


					if (csgo.m_client_state()->m_iChockedCommands >= 15u)
						send_packet = 1;

					UpdateRevolverCock(cmd);

					const auto did_shot = !ctx.m_settings.aimbot_enabled ? false : feature::ragebot->think(cmd, &send_packet);

					if (!ctx.m_settings.aimbot_enabled)
						feature::legitbot->run(cmd);

					const auto can_firstattak = (cmd->buttons & IN_ATTACK && (did_shot || ctx.pressed_keys[1] && m_weapon()->m_iItemDefinitionIndex() == 64 || m_weapon()->m_iItemDefinitionIndex() != 64)) && !is_switching_weapons && !ctx.m_local()->m_bWaitForNoAttack();
					const auto can_secondattk = (cmd->buttons & IN_ATTACK2 && (did_shot && m_weapon()->m_iItemDefinitionIndex() == 64 || m_weapon()->is_knife())) && !is_switching_weapons;

					if (skip_fakelags)
						send_packet = true;

					if ((can_firstattak || can_secondattk) && (did_shot || m_weapon()->can_shoot()) && (m_weapon()->IsGun() || m_weapon()->is_knife()))
					{
						ctx.start_switching = true;
						
						if (!ctx.autopeek_back)
							ctx.autopeek_back = true;

						if (!send_packet)
							feature::anti_aim->did_shot_in_chocked_cycle = true;

						if (ctx.m_settings.aimbot_enabled && ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_remove_recoil) {
							auto recoil = ctx.m_local()->m_aimPunchAngleScaled();

							if (recoil.x != 0 || recoil.y != 0) {
								cmd->viewangles.x = Math::normalize_angle(cmd->viewangles.x) - recoil.x;
								cmd->viewangles.y = Math::normalize_angle(cmd->viewangles.y) - recoil.y;
								cmd->viewangles.z = Math::normalize_angle(cmd->viewangles.z) - recoil.z;
							}
						}

						ctx.last_shot_time_clientside = csgo.m_globals()->realtime;
						ctx.m_ragebot_shot_nr = cmd->command_number;
						ctx.m_ragebot_shot_ang = cmd->viewangles;
						ctx.hold_angles = cmd->viewangles;

						ctx.hold_aim = true;
						ctx.hold_aim_ticks = 0;
						ctx.force_aimbot = 0;

						if (!did_shot)
							feature::resolver->add_shot(ctx.m_eye_position, Vector(0, 0, 0), nullptr, 0, -1, -1);
					}
					else
					{
						if (ctx.m_settings.aimbot_enabled && m_weapon()->m_iItemDefinitionIndex() != 64 && !m_weapon()->IsGrenade() && m_weapon()->IsGun())
							cmd->buttons &= ~IN_ATTACK;

						if (ctx.m_settings.aimbot_enabled || ctx.m_settings.anti_aim_enabled)
							feature::anti_aim->work(cmd, &send_packet);
					}

					movement->End(cmd);

					if (ctx.m_settings.aimbot_enabled && ctx.m_local()->m_fFlags() & FL_ONGROUND)
						feature::misc->end(cmd);

					feature::lagcomp->backup_players(true);

					ctx.last_angles = cmd->viewangles;

					if (exploit_toggled != 0 && ctx.exploit_allowed && !ctx.fakeducking && m_weapon()->IsGun() && m_weapon()->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !is_switching_weapons && exploit_toggled > 0)
					{
						ctx.next_shift_amount = 0;
						ctx.shift_amount = 0;

						if (exploit_toggled == 2)
						{
							ctx.shifting_amount = 14;
							if (cmd->buttons & IN_ATTACK && m_weapon()->can_exploit(14))
							{
								ctx.shift_amount = 14;
								ctx.doubletap_now = true;
								//CopyCommand(cmd, 14);

								ctx.started_speedhack = ctx.current_tickcount;
								ctx.shifted_cmd = cmd->command_number;
								ctx.last_speedhack_time = csgo.m_globals()->realtime;
								ctx.original_tickbase = ctx.m_local()->m_nTickBase();
								ctx.exploit_tickbase_shift = abs(ctx.shift_amount);
								send_packet = false;
							}
						}
						else
							ctx.next_shift_amount = 11;

						cant_repredict = true;
					}
					else
					{
						ctx.shifting_amount = 0;
						ctx.next_shift_amount = 0;
					}
				}

				auto backup_tickbase = ctx.m_local()->m_nTickBase();

				if (ctx.shifting_amount)
					ctx.fixed_tickbase_backtrack = ctx.m_local()->m_nTickBase() - ctx.shifting_amount;
				else
					ctx.fixed_tickbase_backtrack = backup_tickbase;


				if (ctx.m_local()->m_vecVelocity().Length2D() > 0.1f || ctx.m_local()->get_animation_state()->m_velocity_length_xy > 100)
					feature::anti_aim->lby_timer = TICKS_TO_TIME(ctx.m_local()->m_nTickBase()) + 0.22f;
				else
				{
					if (TICKS_TO_TIME(ctx.m_local()->m_nTickBase()) > feature::anti_aim->lby_timer) {
						feature::anti_aim->lby_timer = TICKS_TO_TIME(ctx.m_local()->m_nTickBase()) + 1.1f;
					}
				}

				if (!ctx.air_stuck) {
					const auto bk = ctx.m_local()->m_flThirdpersonRecoil();

					const auto movestate = ctx.m_local()->m_iMoveState();
					const auto iswalking = ctx.m_local()->m_bIsWalking();

					ctx.m_local()->m_iMoveState() = 0;
					ctx.m_local()->m_bIsWalking() = false;

					auto m_forward = cmd->buttons & IN_FORWARD;
					auto m_back = cmd->buttons & IN_BACK;
					auto m_right = cmd->buttons & IN_MOVERIGHT;
					auto m_left = cmd->buttons & IN_MOVELEFT;
					auto m_walking = cmd->buttons & IN_SPEED;

					bool m_walk_state = m_walking ? true : false;

					if (cmd->buttons & IN_DUCK || ctx.m_local()->m_bDucking() || ctx.m_local()->m_fFlags() & FL_DUCKING)
						m_walk_state = false;
					else if (m_walking)
					{
						float m_max_speed = ctx.m_local()->m_flMaxSpeed() * 0.52f;

						if (m_max_speed + 25.f > ctx.m_local()->m_vecVelocity().Length())
							ctx.m_local()->m_bIsWalking() = true;
					}

					auto move_buttons_pressed = cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_RUN);

					bool holding_forward_and_back;
					bool holding_right_and_left;

					if (!m_forward)
						holding_forward_and_back = false;
					else
						holding_forward_and_back = m_back;

					if (!m_right)
						holding_right_and_left = false;
					else
						holding_right_and_left = m_left;

					if (move_buttons_pressed)
					{
						if (holding_forward_and_back)
						{
							if (holding_right_and_left) // if pressing two keys you get stopped
								ctx.m_local()->m_iMoveState() = 0;
							else if (m_right || m_left)
								ctx.m_local()->m_iMoveState() = 2;
							else //none of keys pressed.
								ctx.m_local()->m_iMoveState() = 0;
						}
						else
						{
							if (holding_forward_and_back) // if pressing two keys you get stopped
								ctx.m_local()->m_iMoveState() = 0;
							else if (m_back || m_forward)
								ctx.m_local()->m_iMoveState() = 2;
							else  //none of keys pressed.
								ctx.m_local()->m_iMoveState() = 0;
						}
					}

					if (ctx.m_local()->m_iMoveState() == 2 && m_walk_state)
						ctx.m_local()->m_iMoveState() = 1;

					feature::lagcomp->update_local_animations(cmd, &send_packet);

					ctx.m_local()->m_iMoveState() = movestate;
					ctx.m_local()->m_bIsWalking() = iswalking;
					ctx.m_local()->m_flThirdpersonRecoil() = bk;
				}

				feature::anti_aim->previous_velocity = feature::anti_aim->animation_velocity;
				static QAngle stored_real_angles;

				if (send_packet)
				{
					feature::anti_aim->visual_fake_angle = cmd->viewangles; //-V807
					feature::anti_aim->visual_real_angle = stored_real_angles;
				}

				if (!feature::anti_aim->m_will_lby_update)
					stored_real_angles = cmd->viewangles;

				stored_real_angles = cmd->viewangles;

				if (send_packet)
					feature::anti_aim->visual_real_angle = stored_real_angles;

				feature::anti_aim->visual_fake_angle = stored_real_angles;

				if (!send_packet)
					ctx.skip_communication = false;

				if (!skip_fakelags)
					ApplyShift(cmd, &send_packet);

				ctx.force_next_packet_choke = false;
			}

			ctx.m_settings.aimbot_enabled = aim_toggled;
			prediction->End();

			ctx.g_bOverrideModifier = false;

			ctx.previous_tickcount = ctx.current_tickcount;
			ctx.previous_buttons = cmd->buttons;
			ctx.fix_modify_eye_pos = false;

			auto& out = ctx.command_numbers.emplace_back();
			out.is_outgoing = send_packet;
			out.command_nr = cmd->command_number;
			out.is_used = false;
			out.prev_command_nr = 0;

			while (int(ctx.command_numbers.size()) > int(1.0f / csgo.m_globals()->interval_per_tick)) {
				ctx.command_numbers.pop_front();
			}

			for (auto current : c_lua::get().hooks.getHooks(("createmove")))
				current.func();

			/*if (csgo.m_client_state() != nullptr) {
				INetChannel* net_channel = csgo.m_client_state()->m_ptrNetChannel;
				if (net_channel != nullptr && !send_packet)
				{
					const auto current_choke = net_channel->choked_packets;
					net_channel->choked_packets = 0;
					net_channel->send_datagram();
					--net_channel->out_sequence_nr;
					net_channel->choked_packets = current_choke;
				}
			}*/

			*reinterpret_cast<bool*>(*reinterpret_cast<uintptr_t*>(static_cast<char*>(_AddressOfReturnAddress()) - 0x4) - 0x1C) = send_packet;
		}
		VIRTUALIZER_END;
		return false;
	}
}