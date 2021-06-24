#include "visuals.hpp"

void c_visuals::other_visuals(bool reset)
{
	if (!ctx.auto_peek_spot.IsZero() && Math::VectorDistance(ctx.auto_peek_spot, ctx.m_local()->get_abs_origin()) < 500.f) {
		Drawing::Draw3DFilledCircle(ctx.auto_peek_spot, 12.f, Color(ctx.m_settings.menu_color[0] * 255, ctx.m_settings.menu_color[1] * 255, ctx.m_settings.menu_color[2] * 255, ctx.m_settings.menu_color[3] * 255));
	}
}
