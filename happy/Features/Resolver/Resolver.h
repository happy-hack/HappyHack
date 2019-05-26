#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\SDK\IClientMode.h"
#include <deque>
class Resolver
{
public:
	void Resolve(ClientFrameStage_t stage);
};
extern Resolver g_Resolver;