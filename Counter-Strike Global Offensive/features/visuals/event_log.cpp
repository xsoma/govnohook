#include "visuals.hpp"

void c_visuals::logs()
{
	int bottom = 0;

	if (_events.empty())
		return;

	int x = 8;
	int y = 5;
	auto count = 0;
	const int fontTall = csgo.m_surface()->GetFontTall(F::Events) + 1;

	for (auto& event : _events)
	{
		if (_events.back()._time < csgo.m_globals()->realtime)
			_events.pop_back();

		if (event._time < csgo.m_globals()->realtime && event._displayticks > 0)
			continue;

		if (count > 10)
			_events.pop_back();

		if (!event._msg.empty())
		{
			if (!event._displayed)
			{
				csgo.m_engine_cvars()->ConsoleColorPrintf(Color::White(), sxor("["));
				csgo.m_engine_cvars()->ConsoleColorPrintf(Color::LightBlue(), sxor("gaysex"));
				csgo.m_engine_cvars()->ConsoleColorPrintf(Color::White(), sxor("] %s\n"), event._msg.c_str());

				event._displayed = true;
			}
			if (event._msg[0] == 'f' && event._msg[2] == 'r' && event._msg[6] == 's')
				continue;

			Color clr = Color::White();

			const float timeleft = fabs(event._time - csgo.m_globals()->realtime);

			if (timeleft < .5f)
			{
				float f = Math::clamp(timeleft, 0.0f, .5f) / .5f;

				clr[3] = (int)(f * 255);

				if (count == 0 && f < 0.2f)
				{
					y -= (1.0f - f / 0.2f) * fontTall;
				}
			}
			else
			{
				clr[3] = 255;
			}

			Drawing::DrawString(F::Events, x, y, clr, FONT_LEFT, "%s", event._msg.c_str());

			y += fontTall;

			count++;
		}
	}
}
