#include "engine_prediction.h"
#include "..\..\SDK\CInput.h"
#include "..\..\SDK\CEntity.h"
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CPrediction.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\Menu\Menu.h"

float flOldCurtime;
float flOldFrametime;
int* m_nPredictionRandomSeed;
int* m_pSetPredictionPlayer;
static bool bInit = false;

void engine_prediction::RunEnginePred()
{
	if (!g::local->IsAlive() || !g_pMoveHelper || !g_menu.config.aimbot.Aimbot)
		return;

	if (!m_nPredictionRandomSeed || !m_pSetPredictionPlayer) {
		m_nPredictionRandomSeed = *reinterpret_cast<int**>(Utils::FindSignature("client_panorama.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);
		m_pSetPredictionPlayer = *reinterpret_cast<int**>(Utils::FindSignature("client_panorama.dll", "89 35 ? ? ? ? F3 0F 10 46") + 2);
	}

	CMoveData data;
	memset(&data, 0, sizeof(CMoveData));
	g::local->SetCurrentCommand(g::cmd);
	g_pMoveHelper->SetHost(g::local);

	*m_nPredictionRandomSeed = g::cmd->random_seed;
	*m_pSetPredictionPlayer = uintptr_t(g::local);

	*reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(g::local) + 0x3314) = reinterpret_cast<uint32_t>(g::cmd); // lol
	*reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(g::local) + 0x326C) = reinterpret_cast<uint32_t>(g::cmd); // lol

	flOldCurtime = g_pGlobalVars->curtime;
	flOldFrametime = g_pGlobalVars->frametime;

	g::uRandomSeed = *m_nPredictionRandomSeed;
	g_pGlobalVars->curtime = g::local->GetTickBase() * g_pGlobalVars->intervalPerTick;
	g_pGlobalVars->frametime = g_pGlobalVars->intervalPerTick;

	g_pMovement->StartTrackPredictionErrors(g::local);

	g_pPrediction->SetupMove(g::local, g::cmd, g_pMoveHelper, &data);
	g_pMovement->ProcessMovement(g::local, &data);
	g_pPrediction->FinishMove(g::local, g::cmd, &data);

	if (g::local->GetActiveWeapon())
		g::local->GetActiveWeapon()->AccuracyPenalty();
}

void engine_prediction::EndEnginePred()
{
	if (!g::local->IsAlive() || !g_pMoveHelper || !g_menu.config.aimbot.Aimbot)
		return;

	g_pMovement->FinishTrackPredictionErrors(g::local);
	g_pMoveHelper->SetHost(nullptr);

	if (m_nPredictionRandomSeed || m_pSetPredictionPlayer)
	{
		*m_nPredictionRandomSeed = -1;
		*m_pSetPredictionPlayer = 0;
	}

	g_pGlobalVars->curtime = flOldCurtime;
	g_pGlobalVars->frametime = flOldFrametime;

	g::local->SetCurrentCommand(NULL);
}