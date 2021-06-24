#include "visuals.hpp"

void c_visuals::damage_esp()
{
	if (!ctx.m_settings.player_esp_hit_marker)
		return;

	for (size_t i{ }; i < world_hitmarker.size(); ++i) {
		WorldHitmarkerData_t& info = world_hitmarker[i];
		// If the delta between the current time and hurt time is larger than 2 seconds then we should erase
		if (csgo.m_globals()->curtime - info.m_time > 2.0f) {
			info.m_alpha -= (1.0f / 2.0f) * csgo.m_globals()->frametime;
			info.m_alpha = std::clamp<float>(info.m_alpha, 0.0f, 1.0f);
		}

		if (info.m_alpha <= 0.0f) {
			continue;
		}
		info.m_world_to_screen = Drawing::WorldToScreen(Vector(info.m_pos_x, info.m_pos_y, info.m_pos_z), info.m_world_pos);

		if (info.m_world_to_screen) {
			constexpr int line{ 8 };

			auto draw_lines = [&](Vector pos_custom, Color clr) -> void {
				Drawing::DrawLine(
					pos_custom.x - 6, pos_custom.y - 6,
					pos_custom.x - 2, pos_custom.y - 2,
					clr.malpha(info.m_alpha), true);

				Drawing::DrawLine(
					pos_custom.x - 6, pos_custom.y + 6,
					pos_custom.x - 2, pos_custom.y + 2,
					clr.malpha(info.m_alpha), true);

				Drawing::DrawLine(
					pos_custom.x + 6, pos_custom.y + 6,
					pos_custom.x + 2, pos_custom.y + 2,
					clr.malpha(info.m_alpha), true);

				Drawing::DrawLine(
					pos_custom.x + 6, pos_custom.y - 6,
					pos_custom.x + 2, pos_custom.y - 2,
					clr.malpha(info.m_alpha), true);
			};
			// hitmarker
			draw_lines(info.m_world_pos, Color(255, 255, 255, 255));
		}
	}
}
