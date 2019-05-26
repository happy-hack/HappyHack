#include "Legitbot.hpp"

legitbot g_legitbot;
matrix3x4_t matrix[65][128];
matrix3x4_t localmatrix[65][128];
void legitbot::run_aim() {
	for (int i = 0; i <= 64; i++) {
		C_BaseEntity* e = (C_BaseEntity*)g_pEntityList->GetClientEntity(i);
		if (!e) continue;
		if (e->IsDormant() || !e->GetHealth()) continue;
		if (e->IsImmune() || !g_menu.config.legit.active) continue;
		if (g::local->GetTeam() == e->GetTeam()) continue;

		Vector point, origin;

		//ignore this garabge
		e->SetupBones(matrix[i], 128, 0x0100, 0.f);
		g::local->SetupBones(localmatrix[i], 128, 0x0100, 0.f);

		//initalizing
		point = e->GetHitboxPosition(HITBOX_HEAD, matrix[i]);
		origin = g::local->GetHitboxPosition(HITBOX_HEAD, localmatrix[i]);

		//definetly a better calcangle out there
		Vector enemy_direction = g_Math.CalcAngle(origin, point);

		if ((g::cmd->viewangles - enemy_direction).Length2D() <= g_menu.config.legit.fov && enemy_direction != Vector(0, 0, 0)) {
			if (hitchance(g_menu.config.legit.hitchance)) {

				if (g::cmd->buttons & IN_ATTACK)
					g::cmd->viewangles = enemy_direction;
			}
		}
	}
}