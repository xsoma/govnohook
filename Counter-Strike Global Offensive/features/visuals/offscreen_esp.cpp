#include "visuals.hpp"

void c_visuals::offscreen_esp(C_BasePlayer* entity, float alpha)
{
	Vector vEnemyOrigin = entity->get_abs_origin();
	Vector vLocalOrigin = ctx.m_local()->get_abs_origin();

	if (ctx.m_local()->IsDead())
		vLocalOrigin = csgo.m_input()->m_vecCameraOffset;

	auto is_on_screen = [](Vector origin, Vector& screen) -> bool
	{
		if (!Drawing::WorldToScreen(origin, screen))
			return false;

		return (screen.x > 0 && screen.x < ctx.screen_size.x) && (ctx.screen_size.y > screen.y && screen.y > 0);
	};

	Vector screenPos;

	if (!entity->IsDormant())
	{
		if (entity->m_bone_count() <= 10 || is_on_screen(entity->get_bone_pos(6), screenPos)) //TODO (?): maybe a combo/checkbox to turn this on/off
			return;
	}
	else
	{
		if (is_on_screen(vEnemyOrigin, screenPos))
			return;
	}

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

	const float wm = ctx.screen_size.x / 2, hm = ctx.screen_size.y / 2;

	auto position = Vector2D(wm, hm);
	position.x -= ctx.m_settings.player_esp_out_of_fov_arrow_distance * 5.25f;

	Drawing::rotate_point(position, Vector2D(wm, hm), false, entity_angle.y);

	const auto size = static_cast<int>(ctx.m_settings.player_esp_out_of_fov_arrow_size);//std::clamp(100 - int(vEnemyOrigin.Distance(vLocalOrigin) / 6), 10, 25);

	static float old_fade_alpha = 1;
	static bool sw = false;

	if (old_fade_alpha <= 0)
		sw = true;
	else if (old_fade_alpha >= 1)
		sw = false;

	if (sw)
		old_fade_alpha = Math::clamp(old_fade_alpha + csgo.m_globals()->frametime * 1.2f, 0, 1);
	else
		old_fade_alpha = Math::clamp(old_fade_alpha - csgo.m_globals()->frametime * 1.2f, 0, 1);

	float fade_alpha = old_fade_alpha;

	if (alpha < fade_alpha)
		fade_alpha = alpha;

	Drawing::filled_tilted_triangle(position, Vector2D(size - 1, size), position, true, -entity_angle.y,
		ctx.flt2color(ctx.m_settings.player_esp_out_of_fov_arrow_color).malpha(fade_alpha * 0.6), true,
		ctx.flt2color(ctx.m_settings.player_esp_out_of_fov_arrow_color).malpha(alpha));
}
