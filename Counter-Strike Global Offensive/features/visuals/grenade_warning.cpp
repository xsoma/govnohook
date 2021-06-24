#include "grenade_warning.h"
#include <ragebot/autowall.hpp>
void c_grenade_prediction::on_create_move(CUserCmd* cmd) {
	m_data = {};

	if (ctx.m_local()->IsDead() || !ctx.m_settings.other_esp_grenade_trajectory)
		return;

	const auto weapon = reinterpret_cast<C_WeaponCSBaseGun*>(csgo.m_entity_list()->GetClientEntityFromHandle(ctx.m_local()->m_hActiveWeapon()));
	if (!weapon
		|| !weapon->m_bPinPulled() && weapon->m_fThrowTime() == 0.f)
		return;

	const auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data
		|| weapon_data->type != 9)//weapon_type_grenade
		return;

	m_data.m_owner = ctx.m_local();
	m_data.m_index = weapon->m_iItemDefinitionIndex();

	auto view_angles = cmd->viewangles;

	if (view_angles.x < -90.f) {
		view_angles.x += 360.f;
	}
	else if (view_angles.x > 90.f) {
		view_angles.x -= 360.f;
	}

	view_angles.x -= (90.f - std::fabsf(view_angles.x)) * 10.f / 90.f;

	auto direction = Vector();

	Math::AngleVectors(view_angles, direction);

	const auto throw_strength = std::clamp< float >(weapon->m_flThrowStrength(), 0.f, 1.f);
	const auto eye_pos = ctx.m_local()->GetEyePosition();
	const auto src = Vector(eye_pos.x, eye_pos.y, eye_pos.z + (throw_strength * 12.f - 12.f));

	auto trace = CGameTrace();

	csgo.m_engine_trace()->TraceHull(
		src, src + direction * 22.f, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f },
		MASK_SOLID | CONTENTS_CURRENT_90, ctx.m_local(), COLLISION_GROUP_NONE, &trace
	);

	m_data.predict(
		trace.endpos - direction * 6.f,
		direction * (std::clamp< float >(
			weapon_data->throw_velocity * 0.9f, 15.f, 750.f
			) * (throw_strength * 0.7f + 0.3f)) + ctx.m_local()->m_vecVelocity() * 1.25f,
		csgo.m_globals()->curtime,
		0
	);
}
const char* index_to_grenade_name(int index)
{
	switch (index)
	{
	case WEAPON_SMOKEGRENADE: return "smoke"; break;
	case WEAPON_HEGRENADE: return "he grenade"; break;
	case WEAPON_MOLOTOV:return "molotov"; break;
	case WEAPON_INCGRENADE:return "molotov"; break;
	}
}
const char* index_to_grenade_name_icon(int index)
{
	switch (index)
	{
	case WEAPON_SMOKEGRENADE: return "k"; break;
	case WEAPON_HEGRENADE: return "j"; break;
	case WEAPON_MOLOTOV:return "l"; break;
	case WEAPON_INCGRENADE:return "n"; break;
	}
}
void DrawBeamPaw(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS; //TE_BEAMPOINTS
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	beamInfo.m_pszHaloName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	beamInfo.m_flHaloScale = 0;//0
	beamInfo.m_flWidth = ctx.m_settings.other_esp_grenade_warning_beam_width;//11
	beamInfo.m_flEndWidth = ctx.m_settings.other_esp_grenade_warning_beam_end_width;//11
	beamInfo.m_flFadeLength = 1.0f;
	beamInfo.m_flAmplitude = 2.3;
	beamInfo.m_flBrightness = 255.f;
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0.0;
	beamInfo.m_flFrameRate = 0.0;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;//40
	beamInfo.m_bRenderable = true;
	beamInfo.m_flLife = 0.03f;
	beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM; //FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM

	Beam_t* myBeam = csgo.m_beams()->CreateBeamPoints(beamInfo);
	if (myBeam)
		csgo.m_beams()->DrawBeam(myBeam);
}
void draw_arc(int x, int y, int radius, int start_angle, int percent, int thickness, Color color) {
	auto precision = (2 * M_PI) / 30;
	auto step = M_PI / 180;
	auto inner = radius - thickness;
	auto end_angle = (start_angle + percent) * step;
	auto start_angles = (start_angle * M_PI) / 180;

	for (; radius > inner; --radius) {
		for (auto angle = start_angles; angle < end_angle; angle += precision) {
			auto cx = std::round(x + radius * std::cos(angle));
			auto cy = std::round(y + radius * std::sin(angle));

			auto cx2 = std::round(x + radius * std::cos(angle + precision));
			auto cy2 = std::round(y + radius * std::sin(angle + precision));

			Drawing::DrawLine(cx, cy, cx2, cy2, color);
		}
	}
}
bool c_grenade_prediction::data_t::draw() const
{
	if (m_path.size() <= 1u
		|| csgo.m_globals()->curtime >= m_expire_time)
		return false;

	float distance = ctx.m_local()->m_vecOrigin().Distance(m_origin) / 12;

	if (distance > 200.f)
		return false;

	auto prev_screen = Vector();
	auto prev_on_screen = Drawing::WorldToScreen(std::get< Vector >(m_path.front()), prev_screen);

	for (auto i = 1u; i < m_path.size(); ++i) {
		auto cur_screen = Vector();
		const auto cur_on_screen = Drawing::WorldToScreen(std::get< Vector >(m_path.at(i)), cur_screen);

		if (prev_on_screen && cur_on_screen)
		{
			//Drawing::DrawLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, { 244, 182, 255, 200 });
			if (ctx.m_settings.other_esp_grenade_warning_beam_rainbow)
			{
				auto rainbow_col = Color::FromHSB((360 / m_path.size() * i) / 360.f, 0.9f, 0.8f);
				DrawBeamPaw(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), rainbow_col);
			}
			else
			{
				DrawBeamPaw(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), ctx.flt2color(ctx.m_settings.other_esp_grenade_warning_beam_color));
			}
			
		}

		prev_screen = cur_screen;
		prev_on_screen = cur_on_screen;
	}

	auto textsizeicon = Drawing::GetTextSize(F::GrenadeWarningFont, index_to_grenade_name_icon(m_index));
	auto textsize = Drawing::GetTextSize(F::ESPFont, index_to_grenade_name(m_index));

	float percent = ((m_expire_time - csgo.m_globals()->curtime) / TICKS_TO_TIME(m_tick));
	int alpha_damage = 0;

	if (m_index == WEAPON_HEGRENADE && distance <= 20)
	{
		alpha_damage = 255 - 255 * (distance / 20);
	}

	if ((m_index == WEAPON_MOLOTOV || m_index == WEAPON_INCGRENADE) && distance <= 15)
	{
		alpha_damage = 255 - 255 * (distance / 15);
	}

	/*Drawing::DrawRect(prev_screen.x - textsizeicon.right / 2 - 3, prev_screen.y - textsizeicon.bottom / 2 - 3, textsizeicon.right + textsize.right + 6 + 3, textsizeicon.bottom + 6, Color(10, 10, 14, 150));
	Drawing::DrawRect(prev_screen.x - textsizeicon.right / 2 - 2, prev_screen.y - textsizeicon.bottom / 2 - 2, textsizeicon.right + textsize.right + 4 + 3, textsizeicon.bottom + 4, Color(36, 36, 40, 150));

	Drawing::DrawRect(prev_screen.x - textsizeicon.right / 2 - 2, prev_screen.y - textsizeicon.bottom / 2 - 2, textsizeicon.right + textsize.right + 4 + 3, textsizeicon.bottom + 4, Color(255, 0, 0, alpha_damage));
	Drawing::DrawRect(prev_screen.x - textsizeicon.right / 2 - 3, prev_screen.y - textsizeicon.bottom / 2 - 3, percent, 3, Color(100, 100, 255, 150));

	Drawing::DrawString(F::GrenadeWarningFont, prev_screen.x, prev_screen.y - 12, Color(255, 255, 255, 255), FONT_CENTER, index_to_grenade_name_icon(m_index));
	Drawing::DrawString(F::ESPFont, prev_screen.x + textsizeicon.right - 8, prev_screen.y - textsize.bottom / 2, Color(255, 255, 255, 255), FONT_LEFT, index_to_grenade_name(m_index));*/

	Drawing::DrawFilledCircle(prev_screen.x, prev_screen.y - 10, 30, 180, Color(26, 26, 30, 255));
	Drawing::DrawFilledCircle(prev_screen.x, prev_screen.y - 10, 30, 180, Color(26, 26, 30, alpha_damage));
	draw_arc(prev_screen.x, prev_screen.y - 10, 30, 0, 360 * percent, 2, ctx.flt2color(ctx.m_settings.other_esp_grenade_warning_timer_color));
	Drawing::DrawString(F::GrenadeWarningFont, prev_screen.x, prev_screen.y - 25, Color(255, 255, 255, 255), FONT_CENTER, index_to_grenade_name_icon(m_index));


	auto is_on_screen = [](Vector origin, Vector& screen) -> bool
	{
		if (!Drawing::WorldToScreen(origin, screen))
			return false;

		return (screen.x > 0 && screen.x < ctx.screen_size.x) && (ctx.screen_size.y > screen.y && screen.y > 0);
	};

	Vector screenPos;
	Vector vEnemyOrigin = m_origin;
	Vector vLocalOrigin = ctx.m_local()->get_abs_origin();
	if (ctx.m_local()->IsDead())
		vLocalOrigin = csgo.m_input()->m_vecCameraOffset;

	if (!is_on_screen(vEnemyOrigin, screenPos)) //out_of_fov_grenade_warning
	{
		const float wm = ctx.screen_size.x / 2, hm = ctx.screen_size.y / 2;
		Vector last_pos = std::get< Vector >(m_path.at(m_path.size() - 1));
		
		Vector dir;

		csgo.m_engine()->GetViewAngles(dir);

		float view_angle = dir.y;

		if (view_angle < 0)
			view_angle += 360;

		view_angle = DEG2RAD(view_angle);

		auto entity_angle = Math::CalcAngle(vLocalOrigin, vEnemyOrigin);
		entity_angle.Normalize();

		if (entity_angle.y < 0.f)
			entity_angle.y += 360.f;

		entity_angle.y = DEG2RAD(entity_angle.y);
		entity_angle.y -= view_angle;

		auto position = Vector2D(wm, hm);
		position.x -= std::clamp(vLocalOrigin.Distance(vEnemyOrigin), 100.f, hm - 100);

		Drawing::rotate_point(position, Vector2D(wm, hm), false, entity_angle.y);

		const auto size = static_cast<int>(ctx.m_settings.player_esp_out_of_fov_arrow_size);//std::clamp(100 - int(vEnemyOrigin.Distance(vLocalOrigin) / 6), 10, 25);

		Drawing::DrawFilledCircle(position.x, position.y, 30, 180, Color(26, 26, 30, 255));
		draw_arc(position.x, position.y, 30, 0, 360 * percent, 2, ctx.flt2color(ctx.m_settings.other_esp_grenade_warning_timer_color));
		Drawing::DrawString(F::GrenadeWarningFont, position.x, position.y - 15, Color(255, 255, 255, 255), FONT_CENTER, index_to_grenade_name_icon(m_index));
	}
	return true;
}

void c_grenade_prediction::grenade_warning(C_BasePlayer* entity)
{
	auto& predicted_nades = c_grenade_prediction::Get().get_list();

	static auto last_server_tick = csgo.m_client_state()->m_clockdrift_manager.m_nServerTick;
	if (last_server_tick != csgo.m_client_state()->m_clockdrift_manager.m_nServerTick) {
		predicted_nades.clear();

		last_server_tick = csgo.m_client_state()->m_clockdrift_manager.m_nServerTick;
	}

	if (entity->IsDormant() || !ctx.m_settings.other_esp_grenade_proximity_warning)
		return;

	const auto client_class = entity->GetClientClass();
	if (!client_class
		|| client_class->m_ClassID != 114 && client_class->m_ClassID != CBaseCSGrenadeProjectile)
		return;

	if (client_class->m_ClassID == CBaseCSGrenadeProjectile) {
		const auto model = entity->GetModel();
		if (!model)
			return;

		const auto studio_model = csgo.m_model_info()->GetStudioModel(model);
		if (!studio_model
			|| std::string_view(studio_model->name).find("fraggrenade") == std::string::npos)
			return;
	}

	const auto handle = entity->GetRefEHandle().ToLong();

	if (entity->m_nExplodeEffectTickBegin()) {
		predicted_nades.erase(handle);

		return;
	}

	if (predicted_nades.find(handle) == predicted_nades.end()) {
		predicted_nades.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(handle),
			std::forward_as_tuple(
				reinterpret_cast<C_BaseCombatWeapon*>(entity)->m_hThrower(),
				client_class->m_ClassID == 114 ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
				entity->m_vecOrigin(), reinterpret_cast<C_BasePlayer*>(entity)->m_vecVelocity(),
				entity->get_creation_time(), TIME_TO_TICKS(reinterpret_cast<C_BasePlayer*>(entity)->m_flSimulationTime() - entity->get_creation_time())
			)
		);
	}

	if (predicted_nades.at(handle).draw())
		return;

	predicted_nades.erase(handle);
}
