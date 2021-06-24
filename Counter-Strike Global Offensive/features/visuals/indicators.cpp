#include "visuals.hpp"
#include <string>
int g_Math(float num, float max, int arrsize)
{
	if (num > max)
		num = max;

	auto tmp = max / num;
	auto i = (arrsize / tmp);

	//i = (i >= 0 && floor(i + 0.5) || ceil(i - 0.5));

	if (i >= 0)
		i = floor(i + 0.5f);
	else
		i = ceil(i - 0.5);

	return i;
}
Color g_ColorByInt(float number, float max) {
	static Color Colors[] = {
		{ 255, 0, 0 },
		{ 237, 27, 3 },
		{ 235, 63, 6 },
		{ 229, 104, 8 },
		{ 228, 126, 10 },
		{ 220, 169, 16 },
		{ 213, 201, 19 },
		{ 176, 205, 10 },
		{ 124, 195, 13 }
	};

	auto i = g_Math(number, max, ARRAYSIZE(Colors) - 1);
	return Colors[Math::clamp(i, 0, (int)ARRAYSIZE(Colors) - 1)];
}

int indicators_count = 0;
class c_indicator
{
public:
	c_indicator()
	{
		text = "";
		max_value = 0.f;
	};

	c_indicator(const float _max_value, const char* _text)
	{
		text = _text;
		max_value = _max_value;
	};

	float get_max_value(const float meme = FLT_MAX)
	{
		if (meme < FLT_MAX)
			max_value = meme;

		return max_value;
	}

	void draw(const float factor, Color color = Color(123, 194, 21, 250), const float max_factor = -1.f)
	{
		const auto max = max_factor > 0.f ? max_factor : max_value;

		if (csgo.m_client()->IsChatRaised())
			color._a() = 10;

		const auto text_size = Drawing::GetTextSize(F::LBY, text);
		Drawing::DrawString(F::LBY, 10, ctx.screen_size.y / 2 + 20 * indicators_count, Color(30, 30, 30, csgo.m_client()->IsChatRaised() ? 10 : 250), FONT_CENTER, text);
		const auto draw_factor = Math::clamp((factor / max), 0, 1);
		*reinterpret_cast<bool*>(uintptr_t(csgo.m_surface()) + 0x280) = true;
		int x, y, x1, y1;
		Drawing::GetDrawingArea(x, y, x1, y1);
		Drawing::LimitDrawingArea(0, ctx.screen_size.y / 2 + 20 * indicators_count, int((text_size.right + 15) * draw_factor), (int)text_size.bottom);
		Drawing::DrawString(F::LBY, 10, ctx.screen_size.y / 2 + 20 * indicators_count++, color, FONT_CENTER, text);
		Drawing::LimitDrawingArea(x, y, x1, y1);
		*reinterpret_cast<bool*>(uintptr_t(csgo.m_surface()) + 0x280) = false;
	};
private:
	const char* text = "";
	float max_value = 0.f;
};
void c_visuals::indicators(bool reset)
{
	const int centerX = ctx.screen_size.x / 2, centerY = ctx.screen_size.y / 2;

	if (m_weapon() != nullptr && !ctx.m_local()->IsDead() && (DWORD)ctx.m_local()->get_animation_state() > 0x5000)
	{
		if (ctx.m_local()->m_bIsScoped() && ctx.m_settings.effects_remove_scope_overlay)
		{
			Drawing::DrawLine(0, centerY, centerX + 1, centerY, Color::Black());
			Drawing::DrawLine(centerX - 1, centerY, ctx.screen_size.x, centerY, Color::Black());

			Drawing::DrawLine(centerX, 0, centerX, centerY + 1, Color::Black());
			Drawing::DrawLine(centerX, centerY - 1, centerX, ctx.screen_size.y, Color::Black());

		}

		if (ctx.fakeducking)
			if (m_weapon() != nullptr && m_weapon()->can_shoot())
				Drawing::DrawString(F::LBY, ctx.screen_size.x / 2, ctx.screen_size.y / 2 + 50 + 20 * indicators_count++, ctx.m_local()->m_flDuckSpeed() >= 4.0f ? Color(255, 255, 255, csgo.m_client()->IsChatRaised() ? 5 : 250) : Color::Red(csgo.m_client()->IsChatRaised() ? 5 : 250), FONT_CENTER, "DUCK");

		if (ctx.has_exploit_toggled && (ctx.exploit_allowed || !ctx.fakeducking))
		{
			static float draw_factor = 0.f;

			if (ctx.charged_commands > 1 && (csgo.m_client_state()->m_iChockedCommands < 2 || ctx.is_charging)) {
				draw_factor += min(1.f - draw_factor, csgo.m_globals()->frametime * 3.0f);

				if (draw_factor > 1)
					draw_factor = 1;
			}
			else
			{
				draw_factor -= min(draw_factor, csgo.m_globals()->frametime * 4.0f);

				if (draw_factor < 0)
					draw_factor = 0;
			}

			const auto text_size = Drawing::GetTextSize(F::LBY, ctx.main_exploit == 1 ? sxor("HS") : sxor("DT"));
			Drawing::DrawString(F::LBY, ctx.screen_size.x / 2, ctx.screen_size.y / 2 + 50 + 20 * indicators_count++, Color(255, 255, 255, csgo.m_client()->IsChatRaised() ? 5 : 250), FONT_CENTER, ctx.main_exploit == 1 ? sxor("HS") : sxor("DT"));

			if (ctx.main_exploit >= 2)
			{
				static float prev_alpha = 1.f;
				static float prev_alpha_first_bullet = 1.f;

				if (ctx.m_local() && !ctx.m_local()->IsDead() && m_weapon() && m_weapon()->can_shoot() && ctx.latest_weapon_data) {
					prev_alpha_first_bullet += min(1.f - prev_alpha_first_bullet, csgo.m_globals()->frametime * 2.0f);

					if (m_weapon()->can_exploit(ctx.latest_weapon_data->flCycleTime + 0.01f))
						prev_alpha_first_bullet = 1;

					if (prev_alpha_first_bullet > 1)
						prev_alpha_first_bullet = 1;
				}
				else
				{
					prev_alpha_first_bullet -= min(prev_alpha_first_bullet, csgo.m_globals()->frametime * 5.0f);

					if (prev_alpha_first_bullet < 0)
						prev_alpha_first_bullet = 0;
				}

				if (ctx.has_exploit_toggled && ctx.exploit_allowed && ctx.main_exploit >= 2) {
					if (ctx.charged_commands > 1 && prev_alpha_first_bullet >= 1) {
						prev_alpha += min(1.f - prev_alpha, csgo.m_globals()->frametime * 2.0f);

						if (prev_alpha > 1)
							prev_alpha = 1;
					}
					else
					{
						prev_alpha -= min(prev_alpha, csgo.m_globals()->frametime * 4.0f);

						if (prev_alpha < 0)
							prev_alpha = 0;
					}

					Drawing::DrawString(F::Icons, ctx.screen_size.x / 2 + 10, ctx.screen_size.y / 2 + 55 + 20 * (indicators_count - 1), Color::White(prev_alpha * 255), FONT_LEFT, "u");
				}
				Drawing::DrawString(F::Icons, ctx.screen_size.x / 2 + 10 + 8, ctx.screen_size.y / 2 + 55 + 20 * (indicators_count - 1), Color::White(prev_alpha_first_bullet * 255), FONT_LEFT, "u");
			}
		}

		//Drawing::DrawString(F::LBY, 10, ctx.screen_size.y / 2 + 20 * indicators_count++, Color(255, 255, 255, csgo.m_client()->IsChatRaised() ? 5 : 250), FONT_LEFT, "%.5f", Engine::Prediction::Instance()->m_flInaccuracy);
		/*for unscope shoot*/

		if (ctx.get_key_press(ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_key))
		{
			Drawing::DrawString(F::LBY, ctx.screen_size.x / 2, ctx.screen_size.y / 2 + 20 * indicators_count++, Color(255, 255, 255, csgo.m_client()->IsChatRaised() ? 5 : 250), FONT_CENTER, "SAFE");
		}

	/*	if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_anti_aim_corrections)
		{
			static int old_fade_alpha = 1;
			static bool sw = false;

			if (old_fade_alpha <= 0)
				sw = true;
			else if (old_fade_alpha >= 1)
				sw = false;

			if (sw)
				old_fade_alpha = Math::clamp(old_fade_alpha + csgo.m_globals()->frametime * 0.1f, 0, 5);
			else
				old_fade_alpha = Math::clamp(old_fade_alpha - csgo.m_globals()->frametime * 0.1f, 0, 5);

			Drawing::DrawString(F::LBY, 10, ctx.screen_size.y / 2 + 20 * indicators_count++, Color(255, 255, 255, old_fade_alpha), FONT_LEFT, "RESOLVER");
		}*/


		static bool filled[3] = { false,false,false };

		auto left_pos = Vector2D(ctx.screen_size.x / 2 - 45, ctx.screen_size.y / 2), right_pos = Vector2D(ctx.screen_size.x / 2 + 45, ctx.screen_size.y / 2), down_pos = Vector2D(ctx.screen_size.x / 2, ctx.screen_size.y / 2 + 45), siz = Vector2D(8, 8);

		static std::vector< Vertex_t > vertices_left =
		{
			Vertex_t{ Vector2D(left_pos.x - siz.x, left_pos.y + siz.y), Vector2D() },
			Vertex_t{ Vector2D(left_pos.x, left_pos.y - siz.y), Vector2D() },
			Vertex_t{ left_pos + siz, Vector2D() }
		};

		static std::vector< Vertex_t > vertices_right =
		{
			Vertex_t{ Vector2D(right_pos.x - siz.x, right_pos.y + siz.y), Vector2D() },
			Vertex_t{ Vector2D(right_pos.x, right_pos.y - siz.y), Vector2D() },
			Vertex_t{ right_pos + siz, Vector2D() }
		};

		static std::vector< Vertex_t > vertices_down =
		{
			Vertex_t{ Vector2D(down_pos.x - siz.x, down_pos.y + siz.y), Vector2D() },
			Vertex_t{ Vector2D(down_pos.x, down_pos.y - siz.y), Vector2D() },
			Vertex_t{ down_pos + siz, Vector2D() }
		};

		if (reset) {
			filled[0] = false;
			filled[1] = false;
			filled[2] = false;
		}

		if (!filled[0]) {
			vertices_left =
			{
				Vertex_t{ Vector2D(left_pos.x - siz.x, left_pos.y + siz.y), Vector2D() },
				Vertex_t{ Vector2D(left_pos.x, left_pos.y - siz.y), Vector2D() },
				Vertex_t{ left_pos + siz, Vector2D() }
			};

			for (unsigned int p = 0; p < vertices_left.size(); p++) {
				Drawing::rotate_point(vertices_left[p].m_Position, left_pos, false, 3.15);
			}
			filled[0] = true;
		}

		if (!filled[1]) {
			vertices_right =
			{
				Vertex_t{ Vector2D(right_pos.x - siz.x, right_pos.y + siz.y), Vector2D() },
				Vertex_t{ Vector2D(right_pos.x, right_pos.y - siz.y), Vector2D() },
				Vertex_t{ right_pos + siz, Vector2D() }
			};

			for (unsigned int p = 0; p < vertices_right.size(); p++) {
				Drawing::rotate_point(vertices_right[p].m_Position, right_pos, false, 0);
			}
			filled[1] = true;
		}

		auto fc = (ctx.side == 0 ? ctx.flt2color(ctx.m_settings.menu_color).alpha(160) : Color::Black(130));
		auto sc = (ctx.side == 1 ? ctx.flt2color(ctx.m_settings.menu_color).alpha(160) : Color::Black(130));
		auto tc = (ctx.side == 2 ? ctx.flt2color(ctx.m_settings.menu_color).alpha(160) : Color::Black(130));
		auto fr = (ctx.fside == 1 ? ctx.flt2color(ctx.m_settings.menu_color).alpha(160) : Color::Black(130));
		auto fl = (ctx.fside == -1 ? ctx.flt2color(ctx.m_settings.menu_color).alpha(160) : Color::Black(130));

		Drawing::TexturedPolygon(vertices_left.size(), vertices_left, fc);
		Drawing::TexturedPolygon(vertices_right.size(), vertices_right, sc);
		Drawing::DrawRect(left_pos.x + siz.x + 2, left_pos.y - siz.y, 2, siz.y * 2, fl);
		Drawing::DrawRect(right_pos.x - siz.x - 4, right_pos.y - siz.y, 2, siz.y * 2, fr);


		if (ctx.get_key_press(ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_min_damage_override))
			Drawing::DrawString(F::LBY, ctx.screen_size.x / 2, ctx.screen_size.y / 2 + 50 + 20 * indicators_count++, Color(255, 255, 255, csgo.m_client()->IsChatRaised() ? 5 : 250), FONT_CENTER, ("DMG"));

		static float alpha = 0;

		if (ctx.hurt_time > csgo.m_globals()->realtime)
			alpha = 255;
		else
			alpha = alpha - 255 / 0.9 * csgo.m_globals()->frametime;

		if (alpha > 0 && ctx.m_settings.player_esp_hit_marker) {
			Drawing::DrawLine(ctx.screen_size.x / 2 - linesize, ctx.screen_size.y / 2 - linesize, ctx.screen_size.x / 2 - linedec, ctx.screen_size.y / 2 - linedec, Color(200, 200, 200, alpha));
			Drawing::DrawLine(ctx.screen_size.x / 2 - linesize, ctx.screen_size.y / 2 + linesize, ctx.screen_size.x / 2 - linedec, ctx.screen_size.y / 2 + linedec, Color(200, 200, 200, alpha));
			Drawing::DrawLine(ctx.screen_size.x / 2 + linesize, ctx.screen_size.y / 2 + linesize, ctx.screen_size.x / 2 + linedec, ctx.screen_size.y / 2 + linedec, Color(200, 200, 200, alpha));
			Drawing::DrawLine(ctx.screen_size.x / 2 + linesize, ctx.screen_size.y / 2 - linesize, ctx.screen_size.x / 2 + linedec, ctx.screen_size.y / 2 - linedec, Color(200, 200, 200, alpha));
		}
	}

	indicators_count = 0;
}