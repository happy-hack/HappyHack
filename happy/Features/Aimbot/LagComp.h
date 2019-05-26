#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include <deque>
#include <array>
#include "../../Hooks.h"
#define TIME_TO_TICKS( dt )		( (int)( 0.5 + (float)(dt) / g_pGlobalVars->intervalPerTick ) )
#define TICKS_TO_TIME( t )		( g_pGlobalVars->intervalPerTick *( t ) )

struct PlayerRecords
{
	matrix3x4_t Matrix[128];
	float Velocity;
	float SimTime;
	bool Shot;
	Vector origin;
};
struct backtrack_tick {
	float simulation_time;
	Vector hitbox_position;
	Vector origin;
};
struct lc_record : public backtrack_tick {
	lc_record(C_BaseEntity* e) {
		if (e != nullptr) {
			simulation_time = e->GetSimulationTime();
			hitbox_position = e->GetHitboxPosition(8, str_matrix.matrix[64]);
			origin = e->GetAbsOrigin();
		}
	}
};
class LagComp
{
public:
	int ShotTick[65];
	std::deque<PlayerRecords> PlayerRecord[65] = {  };
	void StoreRecord(C_BaseEntity* pEnt);
	void ClearRecords(int i);
	float LerpTime();
	void handle();
	bool ValidTick(int tick);

	template<class T, class U>
	T clamp(T in, U low, U high);
private:
	Vector angle_vector(Vector to_convert) {
		auto y_sin = sin(to_convert.y / 180.f * static_cast<float>(M_PI));
		auto y_cos = cos(to_convert.y / 180.f * static_cast<float>(M_PI));

		auto x_sin = sin(to_convert.x / 180.f * static_cast<float>(M_PI));
		auto x_cos = cos(to_convert.x / 180.f * static_cast<float>(M_PI));

		return Vector(x_cos * y_cos, x_cos * y_sin, -x_sin);
	}
	float distance_point_to_line(Vector point, Vector origin, Vector direction) {
		auto delta = point - origin;

		auto temp = delta.Dot(direction) / (direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		if (temp < 0.000001f)
			return FLT_MAX;

		auto temp_meme = origin + (direction * temp);
		return (point - temp_meme).Length();
	}
};
extern LagComp g_LagComp;
extern backtrack_tick backtrack[64][12];
extern std::array<std::deque<lc_record>, 64> player_records;