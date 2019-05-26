#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\Utils\Math.h"
#include "..\..\Menu\Menu.h"
#include "../../SDK/Hitboxes.h"
#include "Autowall.h"
class legitbot {
public:
	void run_aim();

	bool hitchance(int hitchance) {
		float inaccuracy = g::local->GetActiveWeapon()->GetInaccuracy();

		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;

		return inaccuracy > hitchance * 1.5;
	}
	void TraceLine(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, C_BaseEntity* ignore, C_Trace* ptr)
	{
		C_TraceFilter filter(ignore);
		g_pTrace->TraceRay(C_Ray(vecAbsStart, vecAbsEnd), mask, &filter, ptr);
	}
	bool player_visible(Vector src, Vector point)
	{
		C_Trace TraceInit;
		TraceLine(src, point, mask_solid, g::local, &TraceInit);
		C_Trace Trace;
		TraceLine(src, point, mask_solid, TraceInit.m_pEnt, &Trace);

		if (Trace.flFraction == 1.0f || TraceInit.flFraction == 1.0f)
			return true;

		return false;
	}
};
extern legitbot g_legitbot;