#include <visuals/visuals.hpp>
#include "source.hpp"
#include <props/entity.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <hooks/hooked.hpp>
#include <props/displacement.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <ragebot/resolver.hpp>
#include <props/prop_manager.hpp>
#include <algorithm>
#include <misc/movement.hpp>
#include <menu/menu/menu.hpp>
#include <time.h>
#include <ragebot/prediction.hpp>
#include <ragebot/rage_aimbot.hpp>
#include <ragebot/lag_comp.hpp>
#include <visuals/chams.hpp>
#include <visuals/sound_parser.hpp>
#include <ragebot/autowall.hpp>
#include <misc/music_player.hpp>
#include <visuals/grenades.hpp>

#include <thread>
#include <cctype>
#include <map>

std::vector<_event> _events;
std::vector<WorldHitmarkerData_t> world_hitmarker;
std::vector<c_bullet_tracer> bullet_tracers;
std::vector<c_damage_indicator> damage_indicators;

bool c_visuals::get_espbox(C_BasePlayer* entity, int& x, int& y, int& w, int& h)
{
	if (!entity || !entity->GetClientClass())
		return false;

	if (entity->IsPlayer() && entity->GetCollideable())
	{
		auto log = &feature::lagcomp->records[entity->entindex() - 1];

		auto min = entity->GetCollideable()->OBBMins();
		auto max = entity->GetCollideable()->OBBMaxs();

		Vector dir, vF, vR, vU;

		csgo.m_engine()->GetViewAngles(dir);
		dir.x = 0;
		dir.z = 0;

		Math::AngleVectors(dir, &vF, &vR, &vU);

		auto zh = vU * max.z + vF * max.y + vR * min.x; // = Front left front
		auto e = vU * max.z + vF * max.y + vR * max.x; //  = Front right front
		auto d = vU * max.z + vF * min.y + vR * min.x; //  = Front left back
		auto c = vU * max.z + vF * min.y + vR * max.x; //  = Front right back

		auto g = vU * min.z + vF * max.y + vR * min.x; //  = Bottom left front
		auto f = vU * min.z + vF * max.y + vR * max.x; //  = Bottom right front
		auto a = vU * min.z + vF * min.y + vR * min.x; //  = Bottom left back
		auto b = vU * min.z + vF * min.y + vR * max.x; //  = Bottom right back*-

		Vector pointList[] = {
			a,
			b,
			c,
			d,
			e,
			f,
			g,
			zh,
		};

		Vector transformed[ARRAYSIZE(pointList)];

		for (int i = 0; i < ARRAYSIZE(pointList); i++)
		{
			auto origin = !entity->IsDormant() ? entity->get_abs_origin() : entity->m_vecOrigin();

			if (log && log->player == entity && log->records_count > 0 && !entity->IsDormant() && ctx.m_local() && !ctx.m_local()->IsDead()) {
				auto last = &log->tick_records[log->records_count & 63];
				origin = entity->get_bone_pos(8) + (last->origin - last->head_pos);
			}



			pointList[i] += origin;

			if (!Drawing::WorldToScreen(pointList[i], transformed[i]))
				return false;
		}

		float left = FLT_MAX;
		float top = -FLT_MAX;
		float right = -FLT_MAX;
		float bottom = FLT_MAX;

		for (int i = 0; i < ARRAYSIZE(pointList); i++) {
			if (left > transformed[i].x)
				left = transformed[i].x;
			if (top < transformed[i].y)
				top = transformed[i].y;
			if (right < transformed[i].x)
				right = transformed[i].x;
			if (bottom > transformed[i].y)
				bottom = transformed[i].y;
		}

		x = left;
		y = bottom;
		w = right - left;
		h = top - bottom;
		return true;
	}
	else
	{
		Vector /*vOrigin, */min, max, flb, brt, blb, frt, frb, brb, blt, flt;
		//float left, top, right, bottom;

		auto* collideable = entity->GetCollideable();

		if (!collideable)
			return false;

		min = collideable->OBBMins();
		max = collideable->OBBMaxs();

		matrix3x4_t& trans = entity->GetCollisionBoundTrans();

		Vector points[] =
		{
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		Vector pointsTransformed[8];
		for (int i = 0; i < 8; i++) {
			Math::VectorTransform(points[i], trans, pointsTransformed[i]);
		}

		Vector pos = entity->get_abs_origin();

		if (!Drawing::WorldToScreen(pointsTransformed[3], flb) || !Drawing::WorldToScreen(pointsTransformed[5], brt)
			|| !Drawing::WorldToScreen(pointsTransformed[0], blb) || !Drawing::WorldToScreen(pointsTransformed[4], frt)
			|| !Drawing::WorldToScreen(pointsTransformed[2], frb) || !Drawing::WorldToScreen(pointsTransformed[1], brb)
			|| !Drawing::WorldToScreen(pointsTransformed[6], blt) || !Drawing::WorldToScreen(pointsTransformed[7], flt))
			return false;

		Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
		//+1 for each cuz of borders at the original box
		float left = flb.x;        // left
		float top = flb.y;        // top
		float right = flb.x;    // right
		float bottom = flb.y;    // bottom

		for (int i = 1; i < 8; i++)
		{
			if (left > arr[i].x)
				left = arr[i].x;
			if (bottom < arr[i].y)
				bottom = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (top > arr[i].y)
				top = arr[i].y;
		}

		x = (int)left;
		y = (int)top;
		w = (int)(right - left);
		h = (int)(bottom - top);

		return true;
	}
	return false;
}

void DECLSPEC_NOINLINE c_visuals::render(bool reset)
{
	static int lastFrame;

	if (lastFrame == csgo.m_globals()->framecount)
		return;

	lastFrame = csgo.m_globals()->framecount;

	logs();

	if (ctx.m_local() == nullptr || !csgo.m_engine()->IsInGame())
		return;

	static auto max_bombtime = csgo.m_engine_cvars()->FindVar("mp_c4timer");
	player_info info;
	int x, y, w, h;
	auto radar_base = feature::find_hud_element<DWORD>(sxor("CCSGO_HudRadar"));
	auto hud_radar = (CCSGO_HudRadar*)(radar_base + 0x74);
	other_visuals(reset);
	indicators(reset);

	static auto linegoesthrusmoke = Memory::Scan(sxor("client.dll"), sxor("55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0"));
	static auto smokecout = *reinterpret_cast<DWORD*>(linegoesthrusmoke + 0x8);

	if (ctx.m_settings.effects_remove_smoke_grenades)
		*reinterpret_cast<int*>(smokecout) = 0;

	feature::sound_parser->draw_sounds();

	for (auto k = 0; k < csgo.m_entity_list()->GetHighestEntityIndex(); k++)
	{
		C_BasePlayer* entity = csgo.m_entity_list()->GetClientEntity(k);
		
		if (entity == nullptr ||
			!entity->GetClientClass() ||
			entity == ctx.m_local())
			continue;

		if (entity->GetClientClass()->m_ClassID == class_ids::CCSPlayer && k < 64)
		{
			if (!entity 
				|| entity->m_iTeamNum() == ctx.m_local()->m_iTeamNum() || !csgo.m_engine()->GetPlayerInfo(entity->entindex(), &info)) continue;

			const auto idx = entity->entindex() - 1;
			const auto is_teammate = (entity->m_iTeamNum() == ctx.m_local()->m_iTeamNum());
			auto &radar_info = *reinterpret_cast<RadarPlayer_t*>(radar_base + (0x174 * (k + 1)) - 0x3C);
			auto was_spotted = (entity->m_bSpotted() || radar_info.spotted) && hud_radar && radar_base && !is_teammate;

			if (ctx.m_settings.other_esp_radar && !entity->IsDormant() && entity->m_iHealth() > 0)
				entity->m_bSpotted() = true;

			c_player_records* log = &feature::lagcomp->records[idx];
			resolver_records* r_log = &feature::resolver->player_records[idx];

			if (entity->IsDormant() && was_spotted && TIME_TO_TICKS(csgo.m_globals()->curtime - entity->m_flSpawnTime()) > 5) {
				log->saved_hp = radar_info.health;
			}

			if (entity->IsDormant() && ctx.m_settings.player_esp_dormant && abs(log->last_sound - csgo.m_globals()->realtime) < 3.f && !log->render_origin.IsZero() && log->render_origin.IsValid()/*&& dormant_alpha[idx] > 0*/) {
				auto dd = 255 * (1.0f - min(1.0f, abs(log->last_sound - csgo.m_globals()->realtime) * 0.3f));

				Vector mins, maxs;
				Vector bottom, top;

				mins = csgo.m_movement()->GetPlayerMins(log->dormant_flags & FL_DUCKING);
				maxs = csgo.m_movement()->GetPlayerMaxs(log->dormant_flags & FL_DUCKING);

				// correct x and y coordinates.
				mins = { log->render_origin.x, log->render_origin.y, log->render_origin.z + mins.z };
				maxs = { log->render_origin.x, log->render_origin.y, log->render_origin.z + maxs.z + 8.f };

				if (!Drawing::WorldToScreen(mins, bottom) || !Drawing::WorldToScreen(maxs, top))
					continue;

				int h = bottom.y - top.y;
				int y = bottom.y - h;
				int w = h / 2.f;
				int x = bottom.x - (w / 2.f);

				int right = 0;
				int down = 0;

				static auto size_info = 9.f;

				auto box_color = ctx.flt2color(ctx.m_settings.player_esp_bounding_box_color);
				auto skeletons_color = ctx.flt2color(ctx.m_settings.player_esp_skeleton_color);
				auto name_color = ctx.flt2color(ctx.m_settings.player_esp_name_color);
				//auto weap_color = ctx.m_settings.colors_esp_weapon;

				int lol = 0;

				if (ctx.m_settings.player_esp_bounding_box) {
					Drawing::DrawOutlinedRect(x, y, w, h, box_color.alpha(min(box_color.a(), dd)));
					//Drawing::DrawOutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(10, 10, 10, min(box_color.a(), dd * 0.5f)));
					//Drawing::DrawOutlinedRect(x + 1, y + 1, w - 2, h - 2, Color(10, 10, 10, min(box_color.a(), dd * 0.5f)));
				}

				if (ctx.m_settings.player_esp_name) {
					auto text_size = Drawing::GetTextSize(F::ESP, log->saved_info.name);
					Drawing::DrawString(F::ESP, x + w / 2 - text_size.right / 2, y - 14, name_color.alpha(min(name_color.a(), dd - 25.f)), FONT_LEFT, "%s", log->saved_info.name);
				}

				if (ctx.m_settings.player_esp_health_bar) {
					int hp = log->saved_hp;

					if (hp > 100)
						hp = 100;

					int hp_percent = h - (int)((h * hp) / 100);

					int width = (w * (hp / 100.f));

					int red = 0x50;
					int green = 0xFF;
					int blue = 0x50;

					if (hp >= 25)
					{
						if (hp < 50)
						{
							red = 0xD7;
							green = 0xC8;
							blue = 0x50;
						}
					}
					else
					{
						red = 0xFF;
						green = 0x32;
						blue = 0x50;
					}

					char hps[10] = "";

					sprintf(hps, "%iHP", hp);

					auto text_size = Drawing::GetTextSize(F::OtherVisualsFont, hps);

					Drawing::DrawRect(x - 5, y - 1, 4, h + 2, Color(80, 80, 80, dd * 0.49f));
					Drawing::DrawOutlinedRect(x - 5, y - 1, 4, h + 2, Color(10, 10, 10, (dd * 0.5f)));
					Drawing::DrawRect(x - 4, y + hp_percent, 2, h - hp_percent, Color(red, green, 0, dd));

					if (hp < 100)
						Drawing::DrawString(F::OtherVisualsFont, x - text_size.right - 6, y - 1, Color(255, 255, 255, dd - 55.f), FONT_LEFT, hps);
				}

				continue;
			}

			if (entity->m_iHealth() <= 0)
				continue;

			log->saved_hp = entity->m_iHealth();
			log->render_origin = Math::interpolate(log->render_origin, entity->get_abs_origin(), Math::clamp((csgo.m_globals()->realtime - log->last_sound) * csgo.m_globals()->frametime, 0, 1));

			dormant_alpha[idx] = 255 * (1.f - min(1.f, abs(entity->m_flSimulationTime() - TICKS_TO_TIME(csgo.m_globals()->tickcount)) * 0.3f));

			if (ctx.m_settings.player_esp_out_of_fov_arrow && !is_teammate)
				offscreen_esp(entity, max(1, dormant_alpha[idx]) / 255);

			auto bk = entity->m_vecOrigin();
			entity->m_vecOrigin() = log->render_origin;
			if (!get_espbox(entity, x, y, w, h)) { entity->m_vecOrigin() = bk; continue; }
			entity->m_vecOrigin() = bk;

			if (log)
				log->last_esp_box.Set(x, y, w, h);

			int right = 0;
			int down = 0;

			auto box_color = ctx.flt2color(ctx.m_settings.player_esp_bounding_box_color);
			auto skeletons_color = ctx.flt2color(ctx.m_settings.player_esp_skeleton_color);
			auto name_color = ctx.flt2color(ctx.m_settings.player_esp_name_color);
			auto ammo_color = ctx.flt2color(ctx.m_settings.player_esp_ammo_color);

			const int lol = (is_teammate ? 1 : 0);

			if (ctx.m_settings.player_esp_bounding_box) {
				Drawing::DrawOutlinedRect(x, y, w, h, box_color.alpha(min(box_color.a(), dormant_alpha[idx])));
				Drawing::DrawOutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(10, 10, 10, min(box_color.a(), dormant_alpha[idx] * 0.8)));
				Drawing::DrawOutlinedRect(x + 1, y + 1, w - 2, h - 2, Color(10, 10, 10, min(box_color.a(), dormant_alpha[idx] * 0.8)));
			}

			if (ctx.m_settings.player_esp_name)
				Drawing::DrawString(F::ESP, x + w / 2, y - 14, name_color.alpha(min(name_color.a(), dormant_alpha[idx] - 10)), FONT_CENTER, "%s", info.name);

			/*Drawing::DrawString(F::ESP, x + w, y + 150, name_color.alpha(min(name_color.a(), dormant_alpha[idx] - 10)), FONT_LEFT, "%f", r_log->feet_weight_server[0]);
			Drawing::DrawString(F::ESP, x + w, y + 170, name_color.alpha(min(name_color.a(), dormant_alpha[idx] - 10)), FONT_LEFT, "%f", r_log->feet_weight_server[1]);
			Drawing::DrawString(F::ESP, x + w, y + 190, name_color.alpha(min(name_color.a(), dormant_alpha[idx] - 10)), FONT_LEFT, "%f", r_log->feet_weight_server[2]);
			Drawing::DrawString(F::ESP, x + w, y + 210, name_color.alpha(min(name_color.a(), dormant_alpha[idx] - 10)), FONT_LEFT, "%f", r_log->feet_weight_rebuild[0]);
			Drawing::DrawString(F::ESP, x + w, y + 230, name_color.alpha(min(name_color.a(), dormant_alpha[idx] - 10)), FONT_LEFT, "%f", r_log->feet_weight_rebuild[1]);
			Drawing::DrawString(F::ESP, x + w, y + 250, name_color.alpha(min(name_color.a(), dormant_alpha[idx] - 10)), FONT_LEFT, "%f", r_log->feet_weight_rebuild[2]);*/


			if (ctx.m_settings.player_esp_health_bar) {
				int hp = log->saved_hp;

				if (hp > 100)
					hp = 100;

				const int hp_percent = h - (int)((h * hp) / 100);
				const int width = (w * float(hp / 100));

				int red = 0x50;
				int green = 0xFF;
				int blue = 0x50;

				if (hp >= 25)
				{
					if (hp < 50)
					{
						red = 0xD7;
						green = 0xC8;
						blue = 0x50;
					}
				}
				else
				{
					red = 0xFF;
					green = 0x32;
					blue = 0x50;
				}
			
				Drawing::DrawRect(x - 5, y - 1, 4, h + 1, Color(0, 0, 0, min(0xA0, dormant_alpha[idx] * 0.49f)));
				Drawing::DrawRect(x - 4, y + hp_percent, 2, h - hp_percent, Color(red, green, blue, min(160, dormant_alpha[idx])));

				if (hp < 100)
					Drawing::DrawString(F::OtherVisualsFont, x - 2, y + max(-1, min(h - 1, hp_percent - 1)), Color(255, 255, 255, min(0xC8, dormant_alpha[idx] - 10)), FONT_CENTER, "%i", hp);
			}

			if (ctx.m_settings.player_esp_flags && entity->m_ArmorValue() > 1)
				Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(255, 255, 255, min(0xC8, dormant_alpha[idx]-10)), FONT_LEFT, (entity->m_bHasHelmet() && entity->m_ArmorValue() > 0 ? _hk.operator std::string().c_str() : (entity->m_bHasHelmet() ? _h.operator std::string().c_str() : _k.operator std::string().c_str())));

			if (ctx.m_settings.player_esp_flags && entity->m_flFlashTime() > 0.1f)
			{
				static auto flashsize = Drawing::GetTextSize(F::OtherVisualsFont, "FLASHED");

				Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right * size_info, Color(30, 30, 30, 250), FONT_LEFT, "FLASHED");

				auto factor = Math::clamp((entity->m_flFlashTime() / 255.f), 0.f, 1.f);

				*(bool*)((DWORD)csgo.m_surface() + 0x280) = true;
				int x1, y1, x2, y2;
				Drawing::GetDrawingArea(x1, y1, x2, y2);
				Drawing::LimitDrawingArea(x + w + 3, y + right * size_info, int(flashsize.right * factor), (int)flashsize.bottom);

				Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(255, 255, 0, dormant_alpha[idx] - 55.f), FONT_LEFT, "FLASHED");

				Drawing::LimitDrawingArea(x1, y1, x2, y2);
				*(bool*)((DWORD)csgo.m_surface() + 0x280) = false;
			}

			if (ctx.m_settings.player_esp_flags && strlen(entity->m_szLastPlaceName()) > 1) {

				const char* last_place = entity->m_szLastPlaceName();

				if (last_place && *last_place)
				{
					//const wchar_t* u_last_place = csgo.m_localize()->find(last_place);
					auto st = std::string(last_place);

					std::transform(st.begin(), st.end(), st.begin(), ::toupper);
					
					Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, entity->IsDormant() ? Color(255, 170, 170, min(0xC8, dormant_alpha[idx] - 10)) : Color(255, 255, 255, min(0xC8, dormant_alpha[idx] - 10)), FONT_LEFT, st.c_str());
				}
			}

			if (ctx.last_aim_index == entity->entindex())
				Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(255, 255, 255, min(0xC8, dormant_alpha[idx] - 10)), FONT_LEFT, "%i", ctx.last_aim_state);

			if (ctx.m_settings.player_esp_skeleton)
				skeleton(entity, skeletons_color.alpha(min(skeletons_color.a(), dormant_alpha[idx] - 10)), entity->m_CachedBoneData().Base());

			const auto &weapon = (C_WeaponCSBaseGun*)(csgo.m_entity_list()->GetClientEntityFromHandle(entity->m_hActiveWeapon()));
			
			if (weapon)
			{
				bool low_ammo = false;

				const auto& weap_info = weapon->GetCSWeaponData();

				if (!weap_info)
					continue;

				int ammo = weapon->m_iClip1();

				if (ammo > weap_info->max_clip)
					ammo = weap_info->max_clip;

				low_ammo = ammo < (weap_info->max_clip / 2);

				if (ctx.m_settings.player_esp_ammo) {
					float width = w;

					const auto is_reloading = entity->get_sec_activity(entity->get_animation_layer(1).m_nSequence) == ACT_CSGO_RELOAD && entity->get_animation_layer(1).m_flWeight > 0;

					if (is_reloading)
						width *= entity->get_animation_layer(1).m_flCycle;
					else
						width = (w * (ammo / float(weap_info->max_clip)));

					char ammostr[10];

					if (!is_reloading)
						sprintf(ammostr, "%d", ammo);
					else
						sprintf(ammostr, sxor("R"));

					const auto text_size = Drawing::GetTextSize(F::OtherVisualsFont, ammostr);

					Drawing::DrawRect(x, y + 1 + h, w + 1, 4, Color(0, 0, 0, min(0xA0, dormant_alpha[idx] * 0.49f)));
					Drawing::DrawRect(x + 1, y + 2 + h, max(0, (width - 1)), 2, ammo_color.alpha(min(ammo_color.a(), min(0xC8, dormant_alpha[idx] - 10))));

					if (ammo < (weap_info->max_clip / 2) && ammo > 0)
						Drawing::DrawString(F::OtherVisualsFont, x + (is_reloading ? w / 2 : min(w - 2, width)), y + h - 1, Color(255, 255, 255, min(0xC8, dormant_alpha[idx] - 10)), FONT_CENTER, ammostr);
				}
				int offset = 0;
				if (ctx.m_settings.player_esp_distance)
				{
					auto distance = ctx.m_local()->m_vecOrigin().Distance(entity->m_vecOrigin()) * 0.01905f;

					if (ctx.m_local()->IsDead())
						distance = csgo.m_input()->m_vecCameraOffset.Distance(entity->m_vecOrigin()) * 0.01905f;

					auto distance_size = Drawing::GetTextSize(F::OtherVisualsFont, "FT");

					Drawing::DrawString(F::OtherVisualsFont, x + w / 2, y + h + 5 + offset, Color::White().alpha(min(200, dormant_alpha[idx] - 10)), FONT_CENTER, "%i FT", (int)distance);
					offset = distance_size.bottom - 3;
				}

				if (ctx.m_settings.player_esp_weapon_text) {

					auto st = std::string(weap_info->weapon_name + 7);

					if (weapon->m_iItemDefinitionIndex() == 64)
						st = _r8;

					std::transform(st.begin(), st.end(), st.begin(), ::toupper);

					auto wpn_name_size = Drawing::GetTextSize(F::OtherVisualsFont, st.c_str());
					Drawing::DrawString(F::OtherVisualsFont, x + w / 2 - wpn_name_size.right / 2, y + h + 5 + offset, Color::White().alpha(min(200, dormant_alpha[idx] - 10)), false, st.c_str());

					offset += wpn_name_size.bottom;
				}
				if (ctx.m_settings.player_esp_weapon_icon)
				{
					const auto name = weapon->get_icon();

					if (name.size() > 0)
						Drawing::DrawString(F::Icons, x + w / 2, y + h + 5 + offset, Color::White().alpha(min(200, dormant_alpha[idx] - 10)), FONT_CENTER, name.c_str());
				}

				if ((weapon->m_zoomLevel() > 0) && ctx.m_settings.player_esp_flags && weapon->IsSniper())
					Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(77, 137, 234, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, _z.operator std::string().c_str()); // ping color : 77, 137, 234

				if (weapon->IsGrenade() && weapon->m_bPinPulled() && !is_teammate && ctx.m_settings.player_esp_flags)
					Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(230, 0, 0, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, _pin.operator std::string().c_str());

				if (csgo.m_player_resource())
				{
					if (ctx.m_settings.player_esp_flags && csgo.m_player_resource()->get_c4_carrier() == entity->entindex())
						Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(230, 0, 0, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, _c4.operator std::string().c_str());
					if (ctx.m_settings.player_esp_flags && csgo.m_player_resource()->get_hostage_carrier() == entity->entindex())
						Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(230, 0, 0, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, _vip.operator std::string().c_str());
				}

				if (ctx.m_settings.player_esp_money)
					Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(170, 190, 80, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, "%i$", entity->m_iAccount());

			}

			
			if (!is_teammate && ctx.m_settings.player_esp_flags && log != nullptr && log->records_count > 0) {

				if (log->tick_records[log->records_count & 63].valid) {
					Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(255, 0, 0, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, sxor("%d"), log->tick_records[log->records_count & 63].simulation_time_delay);

					if (log->tick_records[log->records_count & 63].shot_this_tick)
						Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(255, 255, 255, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, sxor("SHOT"));

					if (log->tick_records[log->records_count & 63].animations_updated)
						Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(170, 255, 170, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, sxor("R"));

					if (log->tick_records[log->records_count & 63].animations_index != 10)
						Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, Color(170, 255, 170, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, sxor("RESOLVER %d %d"), log->tick_records[log->records_count & 63].resolver_index, log->tick_records[log->records_count & 63].resolver_type);

					if (entity->m_flDuckSpeed() >= 4.f && entity->m_flDuckAmount() > 0.f && entity->m_flDuckAmount() < 1 && entity->m_fFlags() & FL_ONGROUND) 
					{
						++r_log->duck_ticks;

						if (r_log->duck_ticks > 30 || r_log->duck_ticks > 4 && (entity->OBBMaxs().DistanceSquared(csgo.m_movement()->GetPlayerMaxs(false)) < 5.f))
							Drawing::DrawString(F::OtherVisualsFont, x + w + 3, y + right++ * size_info, entity->m_flDuckAmount() < 0.4f ? Color(255, 0, 0, min(200, dormant_alpha[idx] - 10)) : Color(255, 255, 255, min(200, dormant_alpha[idx] - 10)), FONT_LEFT, sxor("FD"));
					}
					else
						r_log->duck_ticks = 0;

				
				}
			}
		}
		else
		{
			draw_items(entity);
			feature::grenade_prediction->grenade_warning(entity);
		}
	}

	feature::grenade_prediction->get_local_data().draw();
}

void c_visuals::draw_items(C_BasePlayer* m_entity)
{
	int x, y, w, h;

	static ConVar* max_bombtime = csgo.m_engine_cvars()->FindVar("mp_c4timer");
	static auto scale_bomb_damage = [](float flDamage, int armor_value) -> float
	{
		const float flArmorRatio = 0.5f;
		const float flArmorBonus = 0.5f;
		if (armor_value > 0) {
			float flNew = flDamage * flArmorRatio;
			float flArmor = (flDamage - flNew) * flArmorBonus;

			if (flArmor > static_cast<float>(armor_value)) {
				flArmor = static_cast<float>(armor_value) * (1.f / flArmorBonus);
				flNew = flDamage - flArmor;
			}

			flDamage = flNew;
		}
		return flDamage;
	};

	if (m_entity == nullptr)
		return;

	const auto& client_class = m_entity->GetClientClass();

	if (client_class == nullptr)
		return;

	Vector screen;
	const Vector origin = m_entity->get_abs_origin();
	const auto class_id = client_class->m_ClassID;

	if (!Drawing::WorldToScreen(origin, screen) && class_id != class_ids::CPlantedC4)
		return;
	auto model = m_entity->GetModel();

	switch (class_id)
	{
	case class_ids::CPlantedC4: //BOMB
	{
		if (!ctx.m_settings.other_esp_bomb || !m_entity->m_bBombTicking())
			return;

		break;
	}
	case class_ids::CInferno:
	{
		const auto owner = m_entity->m_hOwnerEntity();

		const auto& eowner = csgo.m_entity_list()->GetClientEntityFromHandle(owner);

		if (!ctx.m_settings.other_esp_grenades || !get_espbox(m_entity, x, y, w, h) || !eowner || !ctx.m_local() || (eowner->m_iTeamNum() == ctx.m_local()->m_iTeamNum() && eowner != ctx.m_local()) && *(float*)(uintptr_t(m_entity) + 0x20) > 0)
			return;

		const double spawn_time = *(float*)(uintptr_t(m_entity) + 0x20);
		const double factor = ((spawn_time + 7.031) - csgo.m_globals()->curtime) / 7.031;

		if (factor <= 0)
			break;

		const int red = max(min(255 * factor, 255), 0);
		const int green = max(min(255 * (1.0 - factor), 255), 0);

		static auto text_size = Drawing::GetTextSize(F::ESPInfo, sxor("inferno"));
		//Drawing::DrawRect(x - 50, y, 100, 15, { 0, 0, 0, 205 });
		//Drawing::Draw3DFilledCircle(origin, 150.f, ctx.flt2color(ctx.m_settings.other_esp_grenades_color));

		Drawing::DrawString(F::ESPInfo, x, y, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, sxor("inferno"));

		Drawing::DrawString(F::ESPInfo, x, y + 8.0, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, "%.0f", (spawn_time + 7.031) - csgo.m_globals()->curtime);
		break;
	}
	case class_ids::CC4:
	{
		const auto owner = m_entity->m_hOwnerEntity();

		if (!ctx.m_settings.other_esp_bomb || !get_espbox(m_entity, x, y, w, h) || owner != -1)
			return;

		Drawing::DrawString(F::ESPInfo, x + w / 2, y + h / 2, ctx.flt2color(ctx.m_settings.other_esp_bomb_color), FONT_CENTER, sxor("bomb"));

		break;
	}
	default:
	{
		const auto owner = m_entity->m_hOwnerEntity();

		if (owner == -1)
		{

			auto* const weapon = reinterpret_cast<C_WeaponCSBaseGun*>(m_entity);

			if (!weapon || !weapon->is_weapon())
				return;

			const auto& name = weapon_names[weapon->m_iItemDefinitionIndex()];

			auto* const wdata = weapon->GetCSWeaponData();


			auto st = std::string(wdata->weapon_name + 7);

			if (weapon->m_iItemDefinitionIndex() == 64)
				st = sxor("revolver");

			std::wstring wstr(st.begin(), st.end());

			weapon_names[weapon->m_iItemDefinitionIndex()] = wstr.c_str();

			auto dist = ctx.m_local()->get_abs_origin().DistanceSquared(m_entity->m_vecOrigin()) * 0.01905f;

			if (ctx.m_local()->IsDead())
				dist = csgo.m_input()->m_vecCameraOffset.DistanceSquared(m_entity->m_vecOrigin()) * 0.01905f;

			const auto cl_dist = Math::clamp(dist - 700.f, 0, 700);
			const auto alpha = min(255, 255 - dist / 75);

			if (alpha <= 0 || name == nullptr || wcslen(name) <= 1 || !get_espbox(m_entity, x, y, w, h)) return;

			const auto text_size = Drawing::GetTextSize(F::OtherVisualsFont, name);
			int offset = 0;
			if (ctx.m_settings.other_esp_dropped_weapons.at(0))
			{
				std::transform(st.begin(), st.end(), st.begin(), ::toupper);

				Drawing::DrawString(F::OtherVisualsFont, x + w / 2, y + h / 2.0f, ctx.flt2color(ctx.m_settings.other_esp_dropped_weapons_color).alpha(alpha), FONT_CENTER, st.c_str());
				if (ctx.m_settings.other_esp_dropped_weapons.at(3))
				{
					if (ctx.m_settings.other_esp_dropped_weapons.at(1))
						offset += text_size.bottom;
					else
						offset += 8;
				}
				else
				{
					offset += text_size.bottom;
				}
			}
			if (ctx.m_settings.other_esp_dropped_weapons.at(1))
			{
				const auto name = weapon->get_icon();
				const auto icon_size = Drawing::GetTextSize(F::Icons, name.c_str());

				if (name.size() > 0)
					Drawing::DrawString(F::Icons, x + w / 2, y + h / 2.0f + offset, ctx.flt2color(ctx.m_settings.other_esp_dropped_weapons_color).alpha(alpha), FONT_CENTER, name.c_str());
				if (ctx.m_settings.other_esp_dropped_weapons.at(3))
				{
					offset += 8;
				}
				else
				{
					offset += icon_size.bottom;
				}
			}
			if (ctx.m_settings.other_esp_dropped_weapons.at(3))
			{
				auto diste = ctx.m_local()->get_abs_origin().Distance(m_entity->m_vecOrigin()) * 0.01905f;
				Drawing::DrawString(F::OtherVisualsFont, x + w / 2, y + h / 2.0f + offset, ctx.flt2color(ctx.m_settings.other_esp_dropped_weapons_color).alpha(alpha), FONT_CENTER, "%i FT", (int)diste);
				offset += text_size.bottom;
			}
			if (ctx.m_settings.other_esp_dropped_weapons_ammo && !weapon->IsGrenade() && weapon->GetCSWeaponData() != nullptr)
			{
				auto clip = weapon->m_iClip1();
				auto maxclip = weapon->GetCSWeaponData()->max_clip;
				clip = std::clamp(clip, 0, 1000);
				maxclip = std::clamp(maxclip, 1, 1000);

				const auto nx = x + w / 2 - text_size.right / 2;

				w = text_size.right;

				const auto width = Math::clamp(w * clip / maxclip, 0, w);

				Drawing::DrawRect(nx, (y + h / 2.f) + offset, w, 3, Color(80, 80, 80, alpha * 0.49));
				Drawing::DrawOutlinedRect(nx - 1, y + h / 2.f - 1 + offset, w + 2, 4, Color(10, 10, 10, (alpha * 0.5)));
				Drawing::DrawRect(nx, (y + h / 2.f) + offset, width, 2, ctx.flt2color(ctx.m_settings.other_esp_dropped_weapons_color).alpha(alpha));

				//Drawing::DrawString(F::ESPInfo, x + max(min((w - text_size.right - 2), width), (1 + text_size.right)) - text_size.right, y + h - 1, Color(255, 255, 255, dormant_alpha[idx] - 55.f), FONT_LEFT, ammostr);
			}
		}
		if (ctx.m_settings.other_esp_grenades)
		{
			//auto name = hash_32_fnv1a(client_class->m_pNetworkName, 8);

			if (class_id == class_ids::CMolotovProjectile
				|| class_id == class_ids::CDecoyProjectile
				|| class_id == class_ids::CSmokeGrenadeProjectile
				|| class_id == class_ids::CSnowballProjectile
				|| class_id == class_ids::CBreachChargeProjectile
				|| class_id == class_ids::CBumpMineProjectile
				|| class_id == class_ids::CBaseCSGrenadeProjectile
				|| class_id == class_ids::CSensorGrenadeProjectile)
			{

				if (!get_espbox(m_entity, x, y, w, h)) return;

				// draw decoy.
				if (class_id == class_ids::CDecoyProjectile)
					Drawing::DrawString(F::ESPInfo, x + w / 2, y + h, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, sxor("decoy"));
				else if (class_id == class_ids::CBaseCSGrenadeProjectile) {
					if (model) {
						std::string name{ model->szName };

						if (name.find(sxor("flashbang")) != std::string::npos)
							Drawing::DrawString(F::ESPInfo, x + w / 2, y + h, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, sxor("flash"));
						else if (name.find(sxor("fraggrenade")) != std::string::npos && m_entity->m_nExplodeEffectTickBegin() < 1)
							Drawing::DrawString(F::ESPInfo, x + w / 2, y + h, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, sxor("frag"));
					}
				}
				else if (class_id == class_ids::CMolotovProjectile)
					Drawing::DrawString(F::ESPInfo, x + w / 2, y + h, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, sxor("fire"));
				else if (class_id == class_ids::CSmokeGrenadeProjectile)
				{
					const auto owner = m_entity->m_hOwnerEntity();

					const auto& eowner = csgo.m_entity_list()->GetClientEntityFromHandle(owner);

					if (!ctx.m_settings.other_esp_grenades || !get_espbox(m_entity, x, y, w, h) || !eowner || !ctx.m_local()) return;

					const float spawn_time = TICKS_TO_TIME(m_entity->m_nSmokeEffectTickBegin());
					const double factor = ((spawn_time + 18.041) - csgo.m_globals()->curtime) / 18.041;

					if (factor <= 0)
					{
						Drawing::DrawString(F::ESPInfo, x + w / 2, y + h, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, sxor("smoke"));
					}
					else
					{


						const int red = max(min(255 * factor, 255), 0);
						const int green = max(min(255 * (1.0 - factor), 255), 0);

						static auto text_size = Drawing::GetTextSize(F::ESPInfo, sxor("smoke"));
						//Drawing::DrawRect(x - 50, y, 100, 15, { 0, 0, 0, 205 });

						//Drawing::Draw3DFilledCircle(origin, 144.f, ctx.flt2color(ctx.m_settings.other_esp_grenades_color));

						if (spawn_time > 0.f) {
							Drawing::DrawString(F::ESPInfo, x, y + 8, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, "%.0f", (spawn_time + 18.04125) - csgo.m_globals()->curtime);
						}
						Drawing::DrawString(F::ESPInfo, x, y, ctx.flt2color(ctx.m_settings.other_esp_grenades_color), FONT_CENTER, sxor("smoke"));
					}
				}
			}
			break;
		}
	}
	}
}