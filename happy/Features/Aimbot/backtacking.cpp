#include "LagComp.h"
#include "../../Hooks.h"
void LagComp::handle() {
	int best_target = -1;
	float best_fov = 90.f;

	if (g::local->GetHealth() <= 0)
		return;

	for (int i = 0; i <= 64; i++) {
		auto e = (C_BaseEntity*)g_pEntityList->GetClientEntity(i);

		if (!e) continue;
		if (e->GetHealth() < 0) continue;
		if (e->IsDormant()) continue;
		if (e->GetTeam() == g::local->GetTeam()) continue;

		if (e->GetHealth() > 0) {

			lc_record record = lc_record(e);
			player_records[i].push_front(lc_record(record));
			while (player_records[i].size() >= 12)
				player_records[i].pop_back();

			backtrack[i][g::cmd->command_number % 12] = backtrack_tick
			{
				e->GetSimulationTime(),
				e->GetHitboxPosition(8, str_matrix.matrix[i]),
				e->GetAbsOrigin()
			};

			Vector view_direction = angle_vector(g::cmd->viewangles);
			float fov = distance_point_to_line(e->GetHitboxPosition(8, str_matrix.matrix[i]), g::local->GetHitboxPosition(8, str_matrix.local_matrix), view_direction);

			if (best_fov > fov) {
				best_fov = fov;
				best_target = i;
			}
		}
	}
	float best_target_simulation_time = 0.f;

	if (best_target != -1) {
		float temp = FLT_MAX;
		Vector view_direction = angle_vector(g::cmd->viewangles);

		for (int t = 0; t < 14; ++t) {
			float fov = distance_point_to_line(backtrack[best_target][t].hitbox_position, g::local->GetHitboxPosition(8, str_matrix.local_matrix), view_direction);
			if (temp > fov && backtrack[best_target][t].simulation_time > g::local->GetSimulationTime() - 1) {
				temp = fov;
				best_target_simulation_time = backtrack[best_target][t].simulation_time;
			}
		}

		if (g::cmd->buttons& IN_ATTACK)
			g::cmd->tick_count = (int)(0.5f + (float)(best_target_simulation_time) / g_pGlobalVars->intervalPerTick);

	}
}
std::array<std::deque<lc_record>, 64> player_records;
backtrack_tick backtrack[64][12];