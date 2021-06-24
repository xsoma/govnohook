#include <hooks/hooked.hpp>
#include <props/displacement.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
//#include <ragebot/prediction.hpp>
//#include <misc/movement.hpp>
//#include <antiaim/anti_aimbot.hpp>
//#include <intrin.h>
//#include <menu/menu/menu.hpp>
//#include <props/player.hpp>
#include <props/prop_manager.hpp>
#include <ragebot/autowall.hpp>

namespace Hooked
{
	void __fastcall OverrideView(void* ecx, void* edx, CViewSetup* vsView)
	{
		using Fn = void(__thiscall*)(void*, CViewSetup*);

		if (!csgo.m_engine()->IsInGame() || ctx.m_local() == nullptr || !ctx.m_local())
			return vmt.m_clientmode->VCall<Fn>(Index::IBaseClientDLL::OverrideView)(ecx, vsView);

		if ((/*ctx.m_settings.visuals_no_first_scope ||*/ !ctx.m_local()->m_bIsScoped()))
			vsView->fov = ctx.m_settings.miscellaneous_override_fov;

		if (ctx.m_settings.effects_instant_scope) {
			if (m_weapon() && m_weapon()->IsSniper())
			{
				vsView->fov = ctx.m_settings.miscellaneous_override_fov;
				const auto zl = m_weapon()->m_zoomLevel();

				if (ctx.m_local()->m_bIsScoped()) {
					
					switch (zl)
					{
					case 1:
						vsView->fov = 90.f - ((vsView->fov / 100) * ctx.m_settings.miscellaneous_override_zoom_fov / 4);
						break;
					case 2:
						vsView->fov = 90.f - ((vsView->fov / 100) * ctx.m_settings.miscellaneous_override_zoom_fov / 2);
						break;
					}
				}
			}
		}

		if (ctx.fakeducking && !ctx.m_local()->IsDead())
			vsView->origin = ctx.m_local()->m_vecAbsOrigin() + Vector(0, 0, csgo.m_movement()->GetPlayerViewOffset(false).z);

		static int tp_anim = 0;
		ctx.in_tp = ctx.get_key_press(ctx.m_settings.effects_force_third_person_key);

		const auto is_in_tp = ctx.in_tp || tp_anim > 0;

		if (is_in_tp && ctx.m_settings.effects_force_third_person_alive && !ctx.m_local()->IsDead())
		{
			if (!csgo.m_input()->is_in_tp())
				csgo.m_input()->m_fCameraInThirdPerson = true;

			if (csgo.m_input()->is_in_tp())
			{
				trace_t trace;
				auto angles = QAngle(0, 0, 0);

				csgo.m_engine()->GetViewAngles(angles);

				Vector camForward;
				Vector camAngles;

				camAngles[0] = angles.x;
				camAngles[1] = angles.y;
				camAngles[2] = 0;

				Math::AngleVectors(camAngles, &camForward, 0, 0);

				camAngles[2] = max(30, min(500, ctx.m_settings.effects_force_third_person_distance));

				const auto eyeorigin = ctx.m_local()->get_abs_origin() + (ctx.fakeducking ? Vector(0, 0, csgo.m_movement()->GetPlayerViewOffset(false).z) : ctx.m_local()->m_vecViewOffset());

				const Vector vecCamOffset(eyeorigin - (camForward * camAngles[2])/* + (camRight * 1.f) + (camUp * 1.f)*/);

				Ray_t ray;
				ray.Init(eyeorigin, vecCamOffset, Vector(-16, -16, -16), Vector(16, 16, 16));

				uint32_t filter[4] = { feature::autowall->get_filter_simple_vtable(), uint32_t(ctx.m_local()), 0, 0 };
				csgo.m_engine_trace()->TraceRay(ray, MASK_NPCWORLDSTATIC, reinterpret_cast<CTraceFilter*>(filter), &trace);

				static float old_frac = 0;

				if (tp_anim == 1 && !ctx.in_tp) {
					csgo.m_input()->m_fCameraInThirdPerson = false;
					old_frac = 0;
				}

				old_frac = Math::interpolate(old_frac, trace.fraction, (trace.fraction < old_frac && old_frac - trace.fraction > 0.1f) ? 0.125f : (csgo.m_globals()->frametime * 3.4f));

				camAngles[2] *= old_frac;

				if (ctx.in_tp)
					tp_anim = 1;
				else
					tp_anim = 0;

				csgo.m_input()->m_vecCameraOffset = camAngles;
			}
		}
		else 
		{
			static bool once = false;
			static bool once2 = false;
			if (is_in_tp && ctx.m_settings.effects_force_third_person_dead && ctx.m_local()->IsDead())
			{
				once = true;

				if (ctx.m_local()->IsDead()) {
					once2 = false;
					return;
				}

				auto spechandle = ctx.m_local()->m_hObserverTarget();
				C_BasePlayer* spec = reinterpret_cast<C_BasePlayer*>(csgo.m_entity_list()->GetClientEntityFromHandle(*spechandle));

				if (!spec)
					return;

				static bool once2 = false;

				if (once2)
					ctx.m_local()->m_iObserverMode() = 5;

				if (ctx.m_local()->m_iObserverMode() == 4)
					once2 = true;

				static Vector angles;
				csgo.m_engine()->GetViewAngles(angles);
				vsView->angles = QAngle(angles.x, angles.y, angles.z);

				trace_t tr;
				Ray_t ray;

				Vector forward, right, up;
				Math::AngleVectors(angles, &forward, &right, &up);

				Vector cam_offset = spec->GetEyePosition() + (forward * -120.f) + (right)+(up);

				ray.Init(spec->GetEyePosition(), cam_offset);
				CTraceFilterWorldOnly trace_filter;
				trace_filter.pSkip = spec;
				csgo.m_engine_trace()->TraceRay(ray, MASK_SHOT, &trace_filter, &tr);

				vsView->origin = tr.endpos;
			}
			else
			{
				csgo.m_input()->m_fCameraInThirdPerson = false;
				csgo.m_input()->m_vecCameraOffset.z = 0;
			}
		}

		//if (vmt.m_clientmode)
 		vmt.m_clientmode->VCall<Fn>(Index::IBaseClientDLL::OverrideView)(ecx, vsView);

		// remove scope edge blur.
		if (ctx.m_settings.effects_remove_scope_overlay) {
			if (ctx.m_local() && ctx.m_local()->m_bIsScoped())
				vsView->m_EdgeBlur = 0;
		}
	}

	bool __fastcall DoPostScreenEffects(void* clientmode, void*, int a1)
	{
		using Fn = bool(__thiscall*)(void*, int);
		static auto ofc = vmt.m_clientmode->VCall<Fn>(44);

		//ctx.glow_rendering = true;

		//if (csgo.m_engine()->IsInGame() && ctx.m_local())
		//{
		//	ctx.m_settings.esp_glow[3]
		//}

		feature::hitchams->OnPostScreenEffects();
		if (csgo.m_engine()->IsConnected() && csgo.m_engine()->IsInGame() && ctx.m_local() && csgo.m_glow_object() != nullptr && csgo.m_glow_object()->GetSize() > 0)
		{
			for (auto i = 0; i < csgo.m_glow_object()->m_GlowObjectDefinitions.Count(); i++)
			{
				auto& glowObject = csgo.m_glow_object()->m_GlowObjectDefinitions[i];
				C_BasePlayer* entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

				if (!entity || entity->IsDead() || entity->IsDormant())
					continue;

				if (!entity->GetClientClass())
					continue;

				if (entity->GetClientClass()->m_ClassID == class_ids::CCSPlayer)
				{
					if (entity == ctx.m_local() && ctx.m_settings.glow[1].enabled)
					{
						Color color = ctx.flt2color(ctx.m_settings.glow[1].color);

						glowObject.m_nGlowStyle = 0;
						glowObject.m_bFullBloomRender = ctx.m_settings.glow[1].m_bFullBloomRender;
						glowObject.m_flRed = color.r() / 255.0f;
						glowObject.m_flGreen = color.g() / 255.0f;
						glowObject.m_flBlue = color.b() / 255.0f;
						glowObject.m_flAlpha = color.a() / 255.0f;
						glowObject.m_bRenderWhenOccluded = ctx.m_settings.glow[1].m_bRenderWhenOccluded;
						glowObject.m_bRenderWhenUnoccluded = ctx.m_settings.glow[1].m_bRenderWhenUnoccluded;
					}
					else if (ctx.m_local()->m_iTeamNum() != entity->m_iTeamNum() && ctx.m_settings.glow[0].enabled)
					{
						Color color = ctx.flt2color(ctx.m_settings.glow[0].color);

						glowObject.m_nGlowStyle = 0;
						glowObject.m_bFullBloomRender = ctx.m_settings.glow[0].m_bFullBloomRender;
						glowObject.m_flRed = color.r() / 255.0f;
						glowObject.m_flGreen = color.g() / 255.0f;
						glowObject.m_flBlue = color.b() / 255.0f;
						glowObject.m_flAlpha = color.a() / 255.0f;
						glowObject.m_bRenderWhenOccluded = ctx.m_settings.glow[0].m_bRenderWhenOccluded;
						glowObject.m_bRenderWhenUnoccluded = ctx.m_settings.glow[0].m_bRenderWhenUnoccluded;
					}
				}
				else
				{
					const auto owner = entity->m_hOwnerEntity();

					if (owner == -1)
					{

						auto* const weapon = reinterpret_cast<C_WeaponCSBaseGun*>(entity);

						if (!weapon || !weapon->is_weapon() || ctx.m_settings.other_esp_dropped_weapons.at(2))
							continue;

						Color color = ctx.flt2color(ctx.m_settings.other_esp_dropped_weapons_color);

						//if (is_local_player)
						//	color = ctx.flt2color(ctx.m_settings.player_esp_glow_color);

						glowObject.m_nGlowStyle = 0;
						glowObject.m_bFullBloomRender = false;
						glowObject.m_flRed = color.r() / 255.0f;
						glowObject.m_flGreen = color.g() / 255.0f;
						glowObject.m_flBlue = color.b() / 255.0f;
						glowObject.m_flAlpha = color.a() / 255.0f;
						glowObject.m_bRenderWhenOccluded = true;
						glowObject.m_bRenderWhenUnoccluded = false;
					}
				}
			}
		}
		auto penis = ofc(clientmode, a1);
		//ctx.glow_rendering = false;

		return penis;
	}

	float __fastcall GetViewModelFOV(void* a1, int ecx)
	{
		using Fn = float(__thiscall*)(void*);
		static auto ofc = vmt.m_clientmode->VCall<Fn>(35);

		auto fov = ofc(a1);

		/*if (ctx.m_settings.misc_override_viewmodel && ctx.m_local() != nullptr)
		{
			auto weapon = m_weapon();
			if (weapon && (weapon->m_iItemDefinitionIndex() != 8 && weapon->m_iItemDefinitionIndex() != 39 || !ctx.m_local()->m_bIsScoped()))
				fov = ctx.m_settings.misc_override_viewmodel_val + fov;
		}*/

		return fov;
	}
}