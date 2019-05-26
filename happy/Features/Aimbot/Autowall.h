#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\SDK\IEngineTrace.h"

struct FireBulletData
{
	FireBulletData(const Vector &eyePos, C_BaseEntity* entity) : src(eyePos) , filter(entity)
	{
	}

	Vector          src;
	C_Trace         enter_trace;
	Vector          direction;
	C_TraceFilter   filter;
	float           trace_length;
	float           trace_length_remaining;
	float           current_damage;
	int             penetrate_count;
};

class Autowall
{
public:
	bool CanHitFloatingPoint(const Vector &point, const Vector &source);
	float Damage(const Vector &point);
	bool CanWallbang(float& dmg);
	float get_thickness(Vector& start, Vector& end, float distance);
private:
};
extern Autowall g_Autowall;