#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\SDK\IGameEvent.h"
#include "..\..\SDK\IEngineTrace.h"
#include "..\..\SDK\IClientMode.h"
#include "..\..\SDK\ICVar.h"
#include "..\..\SDK\ISurface.h"
#include <deque>
#include <string>

extern int showtime;
static std::string prefix;

class CMessage
{
public:
	CMessage(std::string str, float time, Color clr, int a)
	{
		this->str = str;
		this->time = time;
		this->clr = clr;
		alpha = a;
	}
	std::string str;
	float time;
	Color clr;
	int alpha;
};

namespace events
{
	extern std::vector <CMessage> messages;
	extern void log(std::string str);
	extern void print(std::string prefix = "");
}