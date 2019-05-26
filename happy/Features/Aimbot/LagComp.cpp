#include "LagComp.h"
#include "Aimbot.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\SDK\ICvar.h"
#include "..\..\Utils\Math.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\Menu\Menu.h"

LagComp g_LagComp;

float LagComp::LerpTime() // GLAD https://github.com/sstokic-tgm/Gladiatorcheatz-v2.1/blob/eaa88bbb4eca71f8aebfed32a5b86300df8ce6a3/features/LagCompensation.cpp
{
	int updaterate = g_pCvar->FindVar("cl_updaterate")->GetInt();
	ConVar* minupdate = g_pCvar->FindVar("sv_minupdaterate");
	ConVar* maxupdate = g_pCvar->FindVar("sv_maxupdaterate");

	if (minupdate && maxupdate)
		updaterate = maxupdate->GetInt();

	float ratio = g_pCvar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = g_pCvar->FindVar("cl_interp")->GetFloat();
	ConVar* cmin = g_pCvar->FindVar("sv_client_min_interp_ratio");
	ConVar* cmax = g_pCvar->FindVar("sv_client_max_interp_ratio");

	if (cmin && cmax && cmin->GetFloat() != 1)
		ratio = clamp(ratio, cmin->GetFloat(), cmax->GetFloat());

	return max(lerp, (ratio / updaterate));
}

bool LagComp::ValidTick(int tick) // gucci i think cant remember
{
	auto nci = g_pEngine->GetNetChannelInfo();

	if (!nci)
		return false;

	auto PredictedCmdArrivalTick = g::cmd->tick_count + 1 + TIME_TO_TICKS(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING));
	auto Correct = clamp(LerpTime() + nci->GetLatency(FLOW_OUTGOING), 0.f, 1.f) - TICKS_TO_TIME(PredictedCmdArrivalTick + TIME_TO_TICKS(LerpTime()) - (tick + TIME_TO_TICKS(LerpTime())));

	return (abs(Correct) <= 0.2f);
}

void LagComp::StoreRecord(C_BaseEntity* pEnt) // best lag comp in the world
{
	PlayerRecords Setup;

	static float ShotTime[65];
	static float OldSimtime[65];

	if (pEnt != g::local)
		pEnt->FixSetupBones(g_Aimbot.Matrix[pEnt->EntIndex()]);

	if (PlayerRecord[pEnt->EntIndex()].size() == 0)
	{
		Setup.Velocity = abs(pEnt->GetVelocity().Length2D());

		Setup.SimTime = pEnt->GetSimulationTime();

		Setup.origin = pEnt->GetAbsOrigin();

		memcpy(Setup.Matrix, g_Aimbot.Matrix[pEnt->EntIndex()], (sizeof(matrix3x4_t) * 128));

		Setup.Shot = false;

		PlayerRecord[pEnt->EntIndex()].push_back(Setup);
	}

	if (OldSimtime[pEnt->EntIndex()] != pEnt->GetSimulationTime())
	{
		Setup.Velocity = abs(pEnt->GetVelocity().Length2D());

		Setup.SimTime = pEnt->GetSimulationTime();

		Setup.origin = pEnt->GetAbsOrigin();

		if (pEnt == g::local)
			pEnt->FixSetupBones(g_Aimbot.Matrix[pEnt->EntIndex()]);

		memcpy(Setup.Matrix, g_Aimbot.Matrix[pEnt->EntIndex()], (sizeof(matrix3x4_t) * 128));

		if (pEnt->GetActiveWeapon() && !pEnt->IsKnife() || pEnt->IsNade())
		{
			if (ShotTime[pEnt->EntIndex()] != pEnt->GetActiveWeapon()->GetLastShotTime())
			{
				Setup.Shot = true;
				ShotTime[pEnt->EntIndex()] = pEnt->GetActiveWeapon()->GetLastShotTime();
			}
			else
				Setup.Shot = false;
		}
		else
		{
			Setup.Shot = false;
			ShotTime[pEnt->EntIndex()] = 0.f;
		}

		PlayerRecord[pEnt->EntIndex()].push_back(Setup);

		OldSimtime[pEnt->EntIndex()] = pEnt->GetSimulationTime();
	}

	ShotTick[pEnt->EntIndex()] = -1;

	if (PlayerRecord[pEnt->EntIndex()].size() > 0)
		for (int tick = 0; tick < PlayerRecord[pEnt->EntIndex()].size(); tick++)
			if (!ValidTick(TIME_TO_TICKS(PlayerRecord[pEnt->EntIndex()].at(tick).SimTime)))
				PlayerRecord[pEnt->EntIndex()].erase(PlayerRecord[pEnt->EntIndex()].begin() + tick);
			else if (PlayerRecord[pEnt->EntIndex()].at(tick).Shot)
				ShotTick[pEnt->EntIndex()] = tick; // gets the newest shot tick
}

void LagComp::ClearRecords(int i)
{
	if (PlayerRecord[i].size() > 0)
	{
		for (int tick = 0; tick < PlayerRecord[i].size(); tick++)
		{
			PlayerRecord[i].erase(PlayerRecord[i].begin() + tick);
		}
	}
}

template<class T, class U>
T LagComp::clamp(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}