#pragma once

#include "..\Aimbot\Autowall.h"
#include "..\Aimbot\Aimbot.h"
#include "..\Aimbot\LagComp.h"
#include "..\..\Utils\GlobalVars.h"
#include "..\..\Utils\Math.h"
#include "..\..\SDK\ICvar.h"
#include "..\..\SDK\CPrediction.h"
#include "..\..\Menu\Menu.h"
#include <iostream>
#include <algorithm>
#define square( x ) ( x * x )
template<class T, class U>
inline T clamp_value(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
class Misc
{
public:		
	int choke;
    void OnCreateMove()
    {

		this->DoAutostrafe();
		this->DoBhop();
		this->DoFakeLag();
		this->slow_walk();

		if (GetAsyncKeyState(VK_MENU))
			choke = 12;
		else
			choke = std::min<int>(g_menu.config.misc.Fakelag, 12);

		this->fakeduck();
    };
	void slow_walk() {
		if (!g_menu.config.misc.slowwalk) return;

		auto velocity = g::local->GetVelocity().Length2D();
		auto weapon = g::local->GetActiveWeapon(); 
		if (!weapon) return;

		if (velocity >= (weapon->GetCSWpnData()->max_speed * .34)) {
			if (GetAsyncKeyState(VK_SHIFT)) {
				g::cmd->buttons |= IN_WALK;
				g::cmd->sidemove = -g::cmd->sidemove;
				g::cmd->forwardmove = -g::cmd->forwardmove;
				g::cmd->upmove = 0;
			}
		}
	}
	void fakeduck()
	{
		if (!g_menu.config.misc.fakeduck) return;
		auto NetChannel = g_pEngine->GetNetChannel();
		int limit = choke / 2;
		bool crouch = NetChannel->m_nChokedPackets > limit;

		if (GetAsyncKeyState(VK_MENU)) {
			g::cmd->buttons |= IN_BULLRUSH;
			if (crouch)
				g::cmd->buttons |= IN_DUCK;
			else
				g::cmd->buttons &= ~IN_DUCK;
		}
	}
	void start_move_fix() {
		m_oldangle = g::cmd->viewangles;
		m_oldforward = g::cmd->forwardmove;
		m_oldsidemove = g::cmd->sidemove;
	}

	void end_move_fix() {
		float f1, f2, yaw_delta = g::cmd->viewangles.y - m_oldangle.y;
		if (m_oldangle.y < 0.f)
			f1 = 360.0f + m_oldangle.y;
		else
			f1 = m_oldangle.y;

		if (g::cmd->viewangles.y < 0.0f)
			f2 = 360.0f + g::cmd->viewangles.y;
		else
			f2 = g::cmd->viewangles.y;

		if (f2 < f1)
			yaw_delta = abs(f2 - f1);
		else
			yaw_delta = 360.0f - abs(f1 - f2);

		yaw_delta = 360.0f - yaw_delta;
		g::cmd->forwardmove = cos(DEG2RAD(yaw_delta)) * m_oldforward + cos(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove;
		g::cmd->sidemove = sin(DEG2RAD(yaw_delta)) * m_oldforward + sin(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove;
	}
	void ThirdPerson(ClientFrameStage_t curStage)
	{
		if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected() || !g::local || !g_menu.config.misc.Thirdperson)
			return;

		static bool init = false;

		if (GetKeyState(VK_MBUTTON) && g::local->IsAlive())
		{
			if (init)
			{
				ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
				sv_cheats->SetValue(1);
				g_pEngine->ExecuteClientCmd("thirdperson");
			}
			init = false;
		}
		else
		{
			if (!init)
			{
				ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
				sv_cheats->SetValue(1);
				g_pEngine->ExecuteClientCmd("firstperson");
			}
			init = true;
		}

		ConVar* cam_dist = g_pCvar->FindVar("cam_idealdist");
		cam_dist->SetValue(g_menu.config.misc.TpDIST);

		if (curStage == FRAME_RENDER_START && GetKeyState(VK_MBUTTON) && g::local->IsAlive())
		{
			g_pPrediction->SetLocalViewAngles(Vector(g::stored_angles.x, g::stored_angles.y, 0)); // lol
		}
	}

private:
    void DoBhop() const
    {
		if (!g::cmd || !g::local) return;
        if (!g_menu.config.misc.Bhop)
            return;

		if (!g::local->IsAlive())
			return;

        static bool bLastJumped = false;
        static bool bShouldFake = false;

        if (!bLastJumped && bShouldFake)
        {
            bShouldFake = false;
			g::cmd->buttons |= IN_JUMP;
        }
        else if (g::cmd->buttons & IN_JUMP)
        {
            if (g::local->GetFlags() & FL_ONGROUND)
                bShouldFake = bLastJumped = true;
            else 
            {
                g::cmd->buttons &= ~IN_JUMP;
                bLastJumped = false;
            }
        }
        else
            bShouldFake = bLastJumped = false;
    }
	static vec_t normalize_yaw(vec_t ang)
	{
		while (ang < -180.0f)
			ang += 360.0f;
		while (ang > 180.0f)
			ang -= 360.0f;
		return ang;
	}
	void DoAutostrafe() const {
		if (!g::cmd) return;
		if (!g_menu.config.misc.AutoStrafe)
			return;

		if (!g::local->IsAlive())
			return;

		if (!(g::local->GetFlags() & FL_ONGROUND) && !(g::local->GetMoveType() == MoveType_t::MOVETYPE_LADDER)) {
			static float cl_sidespeed = g_pCvar->FindVar("cl_sidespeed")->GetFloat();
			if (GetAsyncKeyState('S')) {
				g::cmd->viewangles.y -= 180;
			}
			else if (GetAsyncKeyState('D')) {
				g::cmd->viewangles.y -= 90;
			}
			else if (GetAsyncKeyState('A')) {
				g::cmd->viewangles.y += 90;
			}
			if (!g::local->GetVelocity().Length2D() > 0.5 || g::local->GetVelocity().Length2D() == NAN || g::local->GetVelocity().Length2D() == INFINITE)
			{
				g::cmd->forwardmove = 400;
				return;
			}

			g::cmd->forwardmove = clamp_value(5850.f / g::local->GetVelocity().Length2D(), -400, 400);
			if ((g::cmd->forwardmove < -400 || g::cmd->forwardmove > 400))
				g::cmd->forwardmove = 0;

			const auto vel = g::local->GetVelocity();
			const float y_vel = RAD2DEG(atan2(vel.y, vel.x));
			const float diff_ang = normalize_yaw(g::cmd->viewangles.y - y_vel);

			g::cmd->sidemove = (diff_ang > 0.0) ? -cl_sidespeed : cl_sidespeed;
			g::cmd->viewangles.y = normalize_yaw(g::cmd->viewangles.y - diff_ang);
		}
	}
	Vector m_oldangle;
	float m_oldforward, m_oldsidemove;
	void DoFakeLag() {
		if (!g::local->IsAlive() || g_pEngine->IsVoiceRecording())
			return;

		if (g::cmd->buttons& IN_ATTACK && !GetAsyncKeyState(VK_MENU) && !g_menu.config.misc.fakeduck)
			return;

		if (!GetAsyncKeyState(VK_MENU)) {
			if (g::local->GetVelocity().Length() < g_menu.config.aimbot.stand_velocity_threshold + 1)
				return;
		}

		static int choked = 0;
		if (choked > choke) {
			g::bSendPacket = true;
			choked = 0;
		}
		else {
			g::bSendPacket = false;
			choked++;
		}
		g::choked_commands = choked;
	}
};

extern Misc g_Misc;