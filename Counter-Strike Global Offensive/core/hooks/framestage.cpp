#include <hooks/hooked.hpp>
#include <ragebot/prediction.hpp>
#include <ragebot/lag_comp.hpp>
#include <visuals/chams.hpp>
#include <props/weapon.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <props/displacement.hpp>
#include <misc/music_player.hpp>
#include <misc/movement.hpp>
#include <ragebot/autowall.hpp>
#include <ragebot/resolver.hpp>
#include <visuals/sound_parser.hpp>
#include "weather_controller.hpp"
#include <fstream>
#include <props/prop_manager.hpp>
#include <features/inventory/skins.h>
#include "lua/Clua.h"

using CNETMsg_File_constructor_fn = void(__thiscall*)(void*);
using CNETMsg_File_destructor_fn = void(__thiscall*)(void*);
using CNETMsg_File_proto_fn = void(__thiscall*)(void*, void*);

template<typename t>
t follow_rel32(uintptr_t address, size_t offset) {

	if (!address)
		return t{};

	auto offsetAddr = address + offset;
	auto relative = *(uint32_t*)offsetAddr;
	if (!relative)
		return t{};

	return (t)(offsetAddr + relative + sizeof(uint32_t));
}

IMaterial* smoke1 = nullptr;
IMaterial* smoke2 = nullptr;
IMaterial* smoke3 = nullptr;
IMaterial* smoke4 = nullptr;

namespace Hooked
{
	struct clientanimating_t {
		C_BaseAnimating* pAnimating;
		unsigned int	flags;
		clientanimating_t(C_BaseAnimating* _pAnim, unsigned int _flags) : pAnimating(_pAnim), flags(_flags) { }
	};


	void __fastcall FrameStageNotify(void* ecx, void* edx, ClientFrameStage_t stage)
	{
		auto clantag_changer = []() -> void
		{
			static float oldTime = -1.f;

			auto setclantag = [](const char* tag, const char* lol) -> void
			{
				typedef void(__fastcall* SetClanTagFn)(const char*, const char*);
				static auto set_clan_tag = reinterpret_cast<SetClanTagFn>(Engine::Displacement::Signatures[c_signatures::SET_CLAN_TAG]);

				if (!Engine::Displacement::Signatures[c_signatures::INIT_KEY_VALUES])
					return;

				set_clan_tag(tag, lol);
			};

			/*auto Marquee = [](std::string& clantag) -> void
			{
				std::string temp = clantag;
				clantag.erase(0, 1);
				clantag += temp[0];
			};*/

			static int v22 = 0;
			const auto v15 = (1.0f / csgo.m_globals()->interval_per_tick);
			/*static int tick = ctx.current_tickcount;
			if (v16 && ctx.current_tickcount != tick)
			{
				v17 = v16->GetAvgLatency(0);
				v14 = TIME_TO_TICKS(v17);
				tick = ctx.current_tickcount;
			}*/

			const auto v21 = static_cast<int>((csgo.m_globals()->tickcount + TIME_TO_TICKS(ctx.latency[FLOW_OUTGOING])) / (v15 / 2)) % 17;

			if (!ctx.m_settings.miscellaneous_clan_tag_spammer) {
				if (oldTime > -1.f) {
					setclantag(sxor("scan gang"), sxor("scan gang"));
					oldTime = -1.f;
				}

				return;
			}

			if (csgo.m_engine()->IsInGame() && csgo.m_client_state() && csgo.m_client_state()->m_iChockedCommands <= 0) {

				if (v21 != v22)
				{
					oldTime = csgo.m_globals()->realtime;
					v22 = v21;
					//Marquee(cur_clantag);
					switch (v21)
					{
					case 0: setclantag(sxor("ga"), sxor("darkai gay")); break;
					case 1: setclantag(sxor("gay"), sxor("darkai gay")); break;
					case 2: setclantag(sxor("gays"), sxor("darkai gay")); break;
					case 3: setclantag(sxor("gayse"), sxor("darkai gay")); break;
					case 4: setclantag(sxor("gaysex"), sxor("darkai gay")); break;
					case 10: setclantag(sxor("gaysex "), sxor("darkai gay")); break;
					case 11: setclantag(sxor("gayse "), sxor("darkai gay")); break;
					case 12: setclantag(sxor("gays"), sxor("darkai gay")); break;
					case 13: setclantag(sxor("gay"), sxor("darkai gay")); break;
					case 14: setclantag(sxor("ga"), sxor("darkai gay")); break;
					case 15: setclantag(sxor("g"), sxor("darkai gay")); break;
					case 16: setclantag(sxor(""), sxor("darkai gay")); break;
					default: break;
					}
				}
			}
		};

		using Fn = void(__thiscall*)(void*, ClientFrameStage_t);

		//if (stage != FRAME_START)
		ctx.last_frame_stage = stage;

		static int m_iLastCmdAck = 0;
		static float m_flNextCmdTime = 0.f;

		const auto is_valid = csgo.m_engine()->IsInGame() && ctx.m_local() && !ctx.m_local()->IsDead();

		if (!is_valid)
			ctx.latest_weapon_data = nullptr;

		if (csgo.m_engine()->IsInGame() && ctx.m_local() && !ctx.m_local()->IsDead())
		{
			if (csgo.m_client_state() && (m_iLastCmdAck != csgo.m_client_state()->m_iLastCommandAck || m_flNextCmdTime != csgo.m_client_state()->m_flNextCmdTime))
			{
				if (ctx.last_velocity_modifier != ctx.m_local()->m_flVelocityModifier())
				{
					*(bool*)((uintptr_t)csgo.m_prediction() + 0x24) = true;
					ctx.last_velocity_modifier = ctx.m_local()->m_flVelocityModifier();
				}

				m_iLastCmdAck = csgo.m_client_state()->m_iLastCommandAck;
				m_flNextCmdTime = csgo.m_client_state()->m_flNextCmdTime;
			}
		}

		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			g_skins.think();
		}

		bool was_in_tp = false;

		if (stage == FRAME_RENDER_START && is_valid)
		{
			for (int i = 1; i <= 64; i++) {

				C_BasePlayer* m_pPlayer = reinterpret_cast<C_BasePlayer*>(csgo.m_entity_list()->GetClientEntity(i));

				if (!m_pPlayer)
					continue;

				*(int*)((uintptr_t)m_pPlayer + 0xA30) = csgo.m_globals()->framecount;
				*(int*)((uintptr_t)m_pPlayer + 0xA28) = 0;
			}


			was_in_tp = csgo.m_input()->is_in_tp();

			if (was_in_tp) {
				ctx.m_local()->UpdateVisibilityAllEntities();
			}

			//if (ctx.is_charging)
			//	csgo.m_globals()->interpolation_amount = 0.f;

			//if (csgo.m_engine()->IsInGame())
			//	feature::lagcomp->interpolate(stage);

			csgo.m_input()->m_fCameraInThirdPerson = false;

			ctx.in_tp = ctx.get_key_press(ctx.m_settings.effects_force_third_person_key);

			ctx.active_keybinds[3].mode = 0;

			if (ctx.in_tp && ctx.m_settings.effects_force_third_person_key.mode != 4)
				ctx.active_keybinds[3].mode = ctx.m_settings.effects_force_third_person_key.mode + 1;
		}

		if (csgo.m_engine()->IsInGame()) {
			if (csgo.m_client_state()->m_iDeltaTick > 0 && stage == FRAME_RENDER_START)
				clantag_changer();
		}

		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			if (ctx.m_local() != nullptr)
			{
				if (ctx.m_settings.effects_remove_flashbang_effect)
					ctx.m_local()->m_flFlashDuration() = 0;
			}
#if 0
			if (GetAsyncKeyState(VK_RETURN))
			{
				g_WeatherController.StopAllWeather();
				for (int i = 0; i <= Interfaces::ClientEntList->GetHighestEntityIndex(); ++i)
				{
					CBaseEntity* bleh = Interfaces::ClientEntList->GetBaseEntity(i);
					if (bleh && bleh->GetClientClass() && bleh->GetClientClass()->m_ClassID == ClassID::_CPrecipitation)
					{
						int modelindex = bleh->GetModelIndex();
						bleh->GetClientNetworkable()->Release();
					}
				}
			}
			else
			{
				int i = 1;
				if (!g_WeatherController.HasSpawnedWeatherOfType(CWeatherController::CClient_Precipitation::PRECIPITATION_TYPE_PARTICLESNOW))
				{
					if (GetAsyncKeyState(VK_F1))
						g_WeatherController.CreateWeather(CWeatherController::CClient_Precipitation::PRECIPITATION_TYPE_PARTICLESNOW);
				}
				if (!g_WeatherController.HasSpawnedWeatherOfType(CWeatherController::CClient_Precipitation::PRECIPITATION_TYPE_PARTICLERAIN))
				{
					if (GetAsyncKeyState(VK_F2))
						g_WeatherController.CreateWeather(CWeatherController::CClient_Precipitation::PRECIPITATION_TYPE_PARTICLERAIN);
				}
				g_WeatherController.UpdateAllWeather();
			}
#endif
		}


		//static float networkedCycle = 0.0f;
		//static float animationTime = 0.f;

		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && is_valid && csgo.m_input() && ctx.m_local()->m_hViewModel()) {
			auto viewmodel = csgo.m_entity_list()->GetClientEntityFromHandle((CBaseHandle)ctx.m_local()->m_hViewModel());

			// m_flAnimTime : 0x260
			if (viewmodel) {
				// onetap.su
				if (Engine::Prediction::Instance()->m_nSequence == ((C_BaseViewModel*)viewmodel)->m_nSequence()
					&& Engine::Prediction::Instance()->m_hWeapon == ((C_BaseViewModel*)viewmodel)->get_viewmodel_weapon()
					&& Engine::Prediction::Instance()->m_nAnimationParity == ((C_BaseViewModel*)viewmodel)->m_nAnimationParity()) {
					viewmodel->m_flCycle() = Engine::Prediction::Instance()->networkedCycle;
					((C_BaseViewModel*)viewmodel)->m_flModelAnimTime() = Engine::Prediction::Instance()->animationTime;
					ctx.update_cycle = CYCLE_NONE;
				}
			}
		}


		feature::resolver->approve_shots(stage);

		feature::resolver->update_missed_shots(stage);

		for (auto current : c_lua::get().hooks.getHooks(("framestage")))
			current.func();

		vmt.m_client->VCall<Fn>(Index::IBaseClientDLL::FrameStageNotify)(ecx, stage);

		//ctx.setup_bones = false;

		if (csgo.m_engine()->IsInGame())
			feature::lagcomp->store_records(stage);

		if (ctx.current_tickcount % 2 == 0) {
			if (csgo.m_engine()->IsInGame() && ctx.m_local()) {

				if (ctx.hud_death_notice == nullptr || ctx.update_hud_weapons == nullptr || csgo.m_player_resource() == nullptr || csgo.m_player_resource()->this_ptr == nullptr)
				{
					csgo.m_player_resource.set(C_PlayerResource::get());

					if (csgo.m_game_rules() == nullptr)
						csgo.m_game_rules.set((**reinterpret_cast<CCSGameRules***>(Engine::Displacement::Signatures[c_signatures::GAMERULES] + 0x1)));
					if (ctx.hud_death_notice == nullptr)
						ctx.hud_death_notice = feature::find_hud_element<CCSGO_HudDeathNotice*>(sxor("CCSGO_HudDeathNotice"));
					if (ctx.update_hud_weapons == nullptr)
						ctx.update_hud_weapons = (void*)Engine::Displacement::Signatures[c_signatures::UPDATEHUDWEAPONS];
				}
			}
			else
			{
				if (csgo.m_game_rules() != nullptr) {
					ctx.hud_death_notice = nullptr;
					ctx.update_hud_weapons = nullptr;
					ctx.fake_state.m_player = 0;
					ctx.shots_fired.fill(0);
					ctx.shots_total.fill(0);
					csgo.m_game_rules.set(nullptr);
					csgo.m_player_resource.set(nullptr);
				}
			}
		}

		if (is_valid && ctx.current_tickcount % 2 == 0) {
			static float local_spawntime = ctx.m_local()->m_flSpawnTime();

			if (local_spawntime != ctx.m_local()->m_flSpawnTime())
			{
				if (ctx.m_settings.miscellaneous_persistent_kill_feed) {

					ctx.hud_death_notice = feature::find_hud_element<CCSGO_HudDeathNotice*>(sxor("CCSGO_HudDeathNotice"));
					ctx.update_hud_weapons = reinterpret_cast<void*>(Engine::Displacement::Signatures[c_signatures::UPDATEHUDWEAPONS]);

					if (ctx.hud_death_notice) {
						int* death_notices = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(ctx.hud_death_notice) -
							20);

						if (death_notices) {
							auto ClearDeathNotices = reinterpret_cast<void(__thiscall*)(DWORD)>(Engine::Displacement::Signatures[c_signatures::CLEARDEATHNOTICES]);

							if (ClearDeathNotices)
								ClearDeathNotices(reinterpret_cast<DWORD>(ctx.hud_death_notice) - 20);
						}
					}
				}

				local_spawntime = ctx.m_local()->m_flSpawnTime();
			}
		}

		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
		{
			if (!smoke1)
				smoke1 = csgo.m_material_system()->FindMaterial(sxor("particle/vistasmokev1/vistasmokev1_fire"), sxor("Other textures"));

			if (!smoke2)
				smoke2 = csgo.m_material_system()->FindMaterial(sxor("particle/vistasmokev1/vistasmokev1_smokegrenade"), sxor("Other textures"));

			if (!smoke3)
				smoke3 = csgo.m_material_system()->FindMaterial(sxor("particle/vistasmokev1/vistasmokev1_emods"), sxor("Other textures"));

			if (!smoke4)
				smoke4 = csgo.m_material_system()->FindMaterial(sxor("particle/vistasmokev1/vistasmokev1_emods_impactdust"), sxor("Other textures"));

			if (ctx.m_settings.effects_remove_smoke_grenades) {
				if (!smoke1->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
					smoke1->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

				if (!smoke2->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
					smoke2->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

				if (!smoke3->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
					smoke3->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

				if (!smoke4->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
					smoke4->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			}
			else {
				if (smoke1->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
					smoke1->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

				if (smoke2->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
					smoke2->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

				if (smoke3->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
					smoke3->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

				if (smoke4->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
					smoke4->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			}


			for (auto current : c_lua::get().hooks.getHooks(("framestage_NETUPDATEPOSTDATAUPDATEEND")))
				current.func();
		}

		if (stage == FRAME_NET_UPDATE_END)
		{
			Engine::Prediction::Instance()->FixNetvarCompression(ctx.fixed_tickbase_time);
		}
	}
}