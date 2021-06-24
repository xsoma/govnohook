#include <hooks/hooked.hpp>
#include <ragebot/prediction.hpp>
#include <props/weapon.hpp>
#include <map>
#include <intrin.h>
#include <visuals/visuals.hpp>
#include <antiaim/anti_aimbot.hpp>

int old_tickbase = 0;
int last_shift_amount = 0;
int tickbase_switch = 0;

namespace Hooked
{
	void __fastcall ProcessMovement(void* ecx, void* edx, C_BasePlayer* basePlayer, CMoveData* moveData)
	{
		using Fn = void(__thiscall*)(void*, C_BasePlayer*, CMoveData*);
		static auto ofc = vmt.m_movement->VCall<Fn>(1);
		moveData->m_bGameCodeMovedPlayer = false;
		ofc(ecx, basePlayer, moveData);
	}

	void __fastcall RunCommand(void* ecx, void* edx, C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper)
	{
		using Fn = void(__thiscall*)(void*, C_BasePlayer*, CUserCmd*, IMoveHelper*);

		const auto is_valid = player != nullptr && player == ctx.m_local() && !ctx.m_local()->IsDead();

		if (csgo.m_move_helper() != moveHelper)
			csgo.m_move_helper.set(moveHelper);

		if (!is_valid) {
			vmt.m_prediction->VCall<Fn>(Index::IPrediction::RunCommand)(ecx, player, ucmd, moveHelper);
			return;
		}

		if (ucmd->tick_count > csgo.m_globals()->tickcount + 72) //-V807
		{
			ucmd->hasbeenpredicted = true;
			player->set_abs_origin(player->m_vecOrigin());
			player->m_nTickBase()++;
			return;
		}

		ctx.fix_modify_eye_pos = true;
		ctx.fix_runcommand = !ucmd->hasbeenpredicted;
		auto& prediction = Engine::Prediction::Instance();
		int backup_tickbase = player->m_nTickBase();
		float backup_curtime = csgo.m_globals()->curtime;
		ctx.last_predicted_command = ucmd;

		float m_flVelModBackup = player->m_flVelocityModifier();
		if (ctx.g_bOverrideModifier && ucmd->command_number == csgo.m_client_state()->m_iLastCommandAck + 1)
			player->m_flVelocityModifier() = ctx.last_velocity_modifier;

		if (vmt.m_prediction)
			vmt.m_prediction->VCall<Fn>(Index::IPrediction::RunCommand)(ecx, player, ucmd, moveHelper);
		else
			return;

		if (ctx.m_local()->get_weapon()) {
			static int old_activity = ctx.m_local()->get_weapon()->m_Activity();
			const auto tickbase = player->m_nTickBase() - 1;
			auto activity = ctx.m_local()->get_weapon()->m_Activity();

			if (ctx.m_local()->get_weapon()->m_iItemDefinitionIndex() == 64) {

				if (old_activity != activity && ctx.m_local()->get_weapon()->m_Activity() == 208)
					old_tickbase = tickbase + 2;

				if (ctx.m_local()->get_weapon()->m_Activity() == 208 && old_tickbase == tickbase)
					ctx.m_local()->get_weapon()->m_flPostponeFireReadyTime() = TICKS_TO_TIME(tickbase) + 0.2f;
			}

			old_activity = activity;
		}

		if (!ctx.g_bOverrideModifier)
			player->m_flVelocityModifier() = m_flVelModBackup;

		player->m_vphysicsCollisionState() = 0;

		Engine::Prediction::Instance()->OnRunCommand(player);

		ctx.fix_modify_eye_pos = false;
		ctx.fix_runcommand = false;
		ctx.command_number = ucmd->command_number;
	}

	void __fastcall SetupMove(void* player, int ecx, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move)
	{
		using Fn = void(__thiscall*)(void*, CUserCmd*, IMoveHelper*, CMoveData*);
		static auto ofc = vmt.m_prediction->VCall<Fn>(Index::IPrediction::SetupMove);

		if ((&Engine::Prediction::Instance()->move_data) != move)
			memcpy(&Engine::Prediction::Instance()->move_data, move, 1380);

		ofc(player, ucmd, pHelper, move);
	}
	bool __fastcall InPrediction(void* ecx, void* edx)
	{
		if ((DWORD)ecx < 0x10)
			return false;

		using Fn = bool(__thiscall*)(void*);
		static auto ofc = vmt.m_prediction->VCall<Fn>(Index::IPrediction::InPrediction);

		static auto weapon_recoil_scale = csgo.m_engine_cvars()->FindVar(sxor("weapon_recoil_scale"));
		static auto view_recoil_tracking = csgo.m_engine_cvars()->FindVar(sxor("view_recoil_tracking"));


		static auto MaintainSequenceTransitions = (void*)Memory::Scan(sxor("client.dll"), sxor("84 C0 74 17 8B 87")); //C_BaseAnimating::MaintainSequenceTransitions
		static auto SetupBones_Timing = (void*)Memory::Scan(sxor("client.dll"), sxor("84 C0 74 0A F3 0F 10 05 ? ? ? ? EB 05")); //C_BaseAnimating::SetupBones

		auto result = ofc(ecx);

		if (ecx != nullptr) {
			if (MaintainSequenceTransitions && ctx.setup_bones && _ReturnAddress() == MaintainSequenceTransitions)
				return true;
			if (SetupBones_Timing && _ReturnAddress() == SetupBones_Timing)
				return false;
		}

		// note - dex; first 2 'test al, al' instructions in C_BasePlayer::CalcPlayerView.
		static void* CalcPlayerView_ret1 = (void*)Memory::Scan(sxor("client.dll"), sxor("84 C0 75 0B 8B 0D ? ? ? ? 8B 01 FF 50 4C"));
		static void* CalcPlayerView_ret2 = (void*)Memory::Scan(sxor("client.dll"), sxor("84 C0 75 08 57 8B CE E8 ? ? ? ? 8B 06"));
		static void* CalcPlayerView_ret3 = (void*)Memory::Scan(sxor("client.dll"), sxor("84 C0 75 24 ? ? ? ? ? ? ? ? ? ? FF 50 ? ? ? ? ? ? 51 C7 ? ? ? ? ? ? ? ? ? ? ? 53 57 FF 50"));
		static void* CalcPlayerView_ret4 = (void*)Memory::Scan(sxor("client.dll"), sxor("84 C0 0F 85 ? ? ? ? 83 EC 08 8D"));

		if (csgo.m_engine()->IsInGame()) {
			if (_ReturnAddress() == CalcPlayerView_ret1) {
				return true;
			}

			if (_ReturnAddress() == CalcPlayerView_ret2) {
				if (ctx.m_settings.effects_visual_recoil_adjustment && ctx.m_local() && !ctx.m_local()->IsDead())
				{
					uintptr_t stack = *(uintptr_t*)((*(uintptr_t*)((uintptr_t)_AddressOfReturnAddress() - sizeof(uintptr_t))) + 0xC);

					QAngle* angles = reinterpret_cast<QAngle*>(stack);
					if (angles) {
						*angles -= ctx.m_local()->m_viewPunchAngle()
							+ (ctx.m_local()->m_aimPunchAngle() * weapon_recoil_scale->GetFloat())
							* view_recoil_tracking->GetFloat();
					}
				}

				return true;
			}
		}

		return result;
	}
}