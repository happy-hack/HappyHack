#include "event logs.h"

std::vector <CMessage> events::messages;
int showtime = 3;

namespace events
{
	void log(std::string str)
	{
		Color clr = Color(255, 255, 255);
		messages.push_back(CMessage(str, g_pGlobalVars->curtime, clr, 255));
		g_pEngine->ExecuteClientCmd(("echo " + str).c_str());
	}

	void print(std::string prefix)
	{
		for (int i = messages.size() - 1; i >= 0; i--)
		{
			auto get_txtsize = g_pSurface->get_text_size(g::misc_font, (char*)messages[i].str.c_str());
			auto get_txtsizepr = g_pSurface->get_text_size(g::misc_font, prefix.c_str());

			if (messages[i].time + 2 <= g_pGlobalVars->curtime) messages[i].alpha -= 2;

			g_pSurface->FilledRect(8, 5 + (25 * i), get_txtsize.right + get_txtsizepr.right + 20, 22, Color(45, 45, 45, messages[i].alpha));
			g_pSurface->FilledRect(8, 5 + (25 * i), get_txtsize.right + get_txtsizepr.right + 20, 22, Color(45, 45, 45, messages[i].alpha));
			g_pSurface->FilledRect(get_txtsize.right + get_txtsizepr.right + 28, 5 + (25 * i), 15, 22, Color(30, 144, 255, messages[i].alpha));
			g_pSurface->OutlinedRect(8, 5 + (25 * i), get_txtsize.right + get_txtsizepr.right + 35, 22, Color(45, 45, 45, messages[i].alpha));

			g_pSurface->DrawT(13, 7 + (25 * i), Color(30, 144, 255, messages[i].alpha), g::misc_font, false, prefix.c_str());
			g_pSurface->DrawT(43, 7 + (25 * i), Color(180, 180, 180, messages[i].alpha), g::misc_font, false, (char*)messages[i].str.c_str());

			if (messages[i].alpha <= 0)	messages.erase(messages.begin() + i);
			if (messages.size() > 15) messages.erase(messages.begin());
		}
	}
}