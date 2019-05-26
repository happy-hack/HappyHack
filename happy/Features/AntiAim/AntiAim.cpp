#include "AntiAim.h"
#include "..\..\Hooks.h"
#include "..\Aimbot\Autowall.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\Utils\Math.h"
#include "..\..\Menu\Menu.h"
#include <DirectXMath.h>
AntiAim g_AntiAim;
bool m_bBreakLowerBody = false, auto_directing;
float m_flNextBodyUpdate = 0;

bool Swtich = false;
float maxdesyncdelta() {
	auto animstate = uintptr_t(g::local->AnimState());
	float duckammount = *(float*)(animstate + 0xA4);
	float speedfraction = max(0, min(*reinterpret_cast<float*>(animstate + 0xF8), 1));
	float speedfactor = max(0, min(1, *reinterpret_cast<float*> (animstate + 0xFC)));
	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.f;
	float unk3;
	if (duckammount > 0) {
		unk2 += ((duckammount * speedfactor) * (0.5f - unk2));
	}
	unk3 = *(float*)(animstate + 0x334) * unk2;
	return unk3;

}
void FreeStanding() {
	static float FinalAngle;
	bool bside1 = false;
	bool bside2 = false;
	bool autowalld = false;
	for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i) {
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);
		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| pPlayerEntity->IsDormant()
			|| pPlayerEntity == g::local
			|| pPlayerEntity->GetTeam() == g::local->GetTeam())
			continue;

		float angToLocal = g_Math.CalcAngle(g::local->GetOrigin(), pPlayerEntity->GetOrigin()).y;
		Vector ViewPoint = pPlayerEntity->GetOrigin() + Vector(0, 0, 90);
		Vector2D Side1 = { (45 * sin(g_Math.GRD_TO_BOG(angToLocal))),(45 * cos(g_Math.GRD_TO_BOG(angToLocal))) };
		Vector2D Side2 = { (45 * sin(g_Math.GRD_TO_BOG(angToLocal + 180))) ,(45 * cos(g_Math.GRD_TO_BOG(angToLocal + 180))) };
		Vector2D Side3 = { (50 * sin(g_Math.GRD_TO_BOG(angToLocal))),(50 * cos(g_Math.GRD_TO_BOG(angToLocal))) };
		Vector2D Side4 = { (50 * sin(g_Math.GRD_TO_BOG(angToLocal + 180))) ,(50 * cos(g_Math.GRD_TO_BOG(angToLocal + 180))) };
		Vector Origin = g::local->GetOrigin();
		Vector2D OriginLeftRight[] = { Vector2D(Side1.x, Side1.y), Vector2D(Side2.x, Side2.y) };
		Vector2D OriginLeftRightLocal[] = { Vector2D(Side3.x, Side3.y), Vector2D(Side4.x, Side4.y) };
		for (int side = 0; side < 2; side++) {
			Vector OriginAutowall = { Origin.x + OriginLeftRight[side].x,  Origin.y - OriginLeftRight[side].y , Origin.z + 80 };
			Vector OriginAutowall2 = { ViewPoint.x + OriginLeftRightLocal[side].x,  ViewPoint.y - OriginLeftRightLocal[side].y , ViewPoint.z };
			if (g_Autowall.CanHitFloatingPoint(OriginAutowall, ViewPoint)) {
				if (side == 0) {
					bside1 = true;
					FinalAngle = angToLocal + 90;
				}
				else if (side == 1) {
					bside2 = true;
					FinalAngle = angToLocal - 90;
				}
				autowalld = true;
			}
			else {
				for (int side222 = 0; side222 < 2; side222++) {
					Vector OriginAutowall222 = { Origin.x + OriginLeftRight[side222].x,  Origin.y - OriginLeftRight[side222].y , Origin.z + 80 };
					if (g_Autowall.CanHitFloatingPoint(OriginAutowall222, OriginAutowall2)) {
						if (side222 == 0) {
							bside1 = true;
							FinalAngle = angToLocal + 90;
						}
						else if (side222 == 1) {
							bside2 = true;
							FinalAngle = angToLocal - 90;
						}
						autowalld = true;
					}
				}
			}
		}
	}
	if (!autowalld || (bside1 && bside2))
		g::cmd->viewangles.y += 180;
	else
		g::cmd->viewangles.y = FinalAngle;
}
float randnum(float Min, float Max) {
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}
void update_lowerbody_breaker() {
	float server_time = g::local->GetTickBase() * g_pGlobalVars->intervalPerTick, speed = g::local->GetVelocity().Length2DSqr(), oldCurtime = g_pGlobalVars->curtime;

	if (speed > 0.1) {
		m_flNextBodyUpdate = server_time + 0.22;
	}

	m_bBreakLowerBody = false;


	if (m_flNextBodyUpdate <= server_time) {
		m_flNextBodyUpdate = server_time + 1.1;
		m_bBreakLowerBody = true;
	}


	if (!(g::local->GetFlags() & FL_ONGROUND)) {
		m_bBreakLowerBody = false;
	}
}
void JitterDesync() {
	auto feetdelta = g::local->AnimState()->m_flGoalFeetYaw - g::cmd->viewangles.y;
	float desync = g::local->get_max_desync_delta();
	float lby_delta = 180.0f - desync + 10.0f;
	float desync_length = 180.0f - lby_delta - 10.f;
	if (m_bBreakLowerBody) {
		if (g_pClientState->choked_commands >= 2) {
			g::cmd->viewangles.y = g_Math.NormalizeYaw(g::cmd->viewangles.y);
			return;
		}
		g::cmd->viewangles.y -= 100.0f;
	}
	if (g::bSendPacket && g::cmd->command_number % 3) {
		if (feetdelta < g::local->get_max_desync_delta())
		{
			g::cmd->viewangles.y = g::RealAngle.y + desync_length;
		}
		else {
			g::cmd->viewangles.y = g::RealAngle.y - desync_length;
		}
	}
	else {
		FreeStanding();
	}
}
void Real()
{
	if (g::local->GetVelocity().Length() <= g_menu.config.aimbot.stand_velocity_threshold) {
		switch (g_menu.config.aimbot.yaw_stand) {
		case 1: g::cmd->viewangles.y += 179; break; //backwards
		case 2: FreeStanding(); break; //free standing
		case 3: JitterDesync(); break; //desync
		}

		switch (g_menu.config.aimbot.pitch_stand) {
		case 1: g::cmd->viewangles.x = 85; break; //emotion
		case 2: g::cmd->viewangles.x = -89; break; //up
		case 3: g::cmd->viewangles.x = 0; break; //zero
		case 4: g::cmd->viewangles.x = -540; break; //no spread
		}
	}
	else {
		switch (g_menu.config.aimbot.yaw_move) {
		case 1: g::cmd->viewangles.y += 179; break; //backwards
		case 2: FreeStanding(); break; //free standing
		}

		switch (g_menu.config.aimbot.pitch_move) {
		case 1: g::cmd->viewangles.x = 85; break; //emotion
		case 2: g::cmd->viewangles.x = -89; break; //up
		case 3: g::cmd->viewangles.x = 0; break; //zero
		case 4: g::cmd->viewangles.x = -540; break; //no spread
		}
	}
}
void AntiAim::OnCreateMove()
{

	if (!g::local->IsAlive())
		return;

	if (!g::local->GetActiveWeapon() || g::local->IsNade())
		return;

	if ((g::cmd->buttons& IN_ATTACK) || (g::cmd->buttons & IN_ATTACK2) || g::local->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
		return;

	Vector eye;
	g_pEngine->GetViewAngles(eye);

	if (g_menu.config.aimbot.Antiaim) {
		update_lowerbody_breaker();
		Real();
		g::stored_angles = g::cmd->viewangles;
	}
	else
		g::stored_angles = eye;
}