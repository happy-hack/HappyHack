#include "Autowall.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\SDK\ISurfaceData.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\SDK\bspflags.h"
#include "..\..\SDK\ICvar.h"
#include "..\..\Utils\Math.h"
#include "..\..\SDK\ClientClass.h"
#include <algorithm>

// esoterik ftw

Autowall g_Autowall;

void TraceLine(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, C_BaseEntity *ignore, C_Trace *ptr)
{
	C_TraceFilter filter(ignore);
	g_pTrace->TraceRay(C_Ray(vecAbsStart, vecAbsEnd), mask, &filter, ptr);
}

bool VectortoVectorVisible(Vector src, Vector point)
{
	C_Trace TraceInit;
	TraceLine(src, point, mask_solid, g::local, &TraceInit);
	C_Trace Trace;
	TraceLine(src, point, mask_solid, TraceInit.m_pEnt, &Trace);

	if (Trace.flFraction == 1.0f || TraceInit.flFraction == 1.0f)
		return true;

	return false;
}

float GetHitgroupDamageMult(int iHitGroup)
{
	switch (iHitGroup)
	{
	case HITGROUP_HEAD:
	return 4.f;
	case HITGROUP_STOMACH:
	return 1.25f;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
	return 0.75f;
	}

	return 1.0f;
}

bool HandleBulletPenetration(WeaponInfo_t *wpn_data, FireBulletData &data, bool extracheck);

void ScaleDamage(int hitgroup, C_BaseEntity *enemy, float weapon_armor_ratio, float &current_damage)
{
	current_damage *= GetHitgroupDamageMult(hitgroup);

	if (enemy->ArmorValue() > 0.0f && hitgroup < HITGROUP_LEFTLEG)
	{
		if (hitgroup == HITGROUP_HEAD && !enemy->HasHelmet())
			return;

		float armorscaled = (weapon_armor_ratio * 0.5f) * current_damage;
		if ((current_damage - armorscaled) * 0.5f > enemy->ArmorValue())
			armorscaled = current_damage - (enemy->ArmorValue() * 2.0f);
		current_damage = armorscaled;
	}
}

void UTIL_ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, C_Trace* tr)
{
	static DWORD dwAddress = Utils::FindPattern("client_panorama.dll", (BYTE*)"\x53\x8B\xDC\x83\xEC\x08\x83\xE4\xF0\x83\xC4\x04\x55\x8B\x6B\x04\x89\x6C\x24\x04\x8B\xEC\x81\xEC\x00\x00\x00\x00\x8B\x43\x10", "xxxxxxxxxxxxxxxxxxxxxxxx????xxx");

	if (!dwAddress)
		return;

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, vecAbsEnd
		LEA		ECX, vecAbsStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}

bool SimulateFireBullet(C_BaseEntity *local, C_BaseCombatWeapon *weapon, FireBulletData &data)
{
	data.penetrate_count = 4;
	data.trace_length = 0.0f;
	auto *wpn_data = weapon->GetCSWpnData();
	data.current_damage = (float)wpn_data->damage;
	while ((data.penetrate_count > 0) && (data.current_damage >= 1.0f))
	{
		data.trace_length_remaining = wpn_data->range - data.trace_length;
		Vector End_Point = data.src + data.direction * data.trace_length_remaining;
		TraceLine(data.src, End_Point, 0x4600400B, local, &data.enter_trace);
		UTIL_ClipTraceToPlayers(data.src, End_Point * 40.f, 0x4600400B, &data.filter, &data.enter_trace);
		if (data.enter_trace.flFraction == 1.0f) 
			break;
		if ((data.enter_trace.hitGroup <= 7) && (data.enter_trace.hitGroup > 0) && (local->GetTeam() != data.enter_trace.m_pEnt->GetTeam()))
		{
			data.trace_length += data.enter_trace.flFraction * data.trace_length_remaining;
			data.current_damage *= pow(wpn_data->range_modifier, data.trace_length * 0.002);
			ScaleDamage(data.enter_trace.hitGroup, data.enter_trace.m_pEnt, wpn_data->armor_ratio, data.current_damage);
			return true;
		}
		if (!HandleBulletPenetration(wpn_data, data, false)) 
			break;
	}
	return false;
}

bool TraceToExitalt(Vector& end, C_Trace& tr, Vector start, Vector vEnd, C_Trace* trace)
{
	typedef bool(__fastcall* TraceToExitFn)(Vector&, C_Trace&, float, float, float, float, float, float, C_Trace*);
	static TraceToExitFn oTraceToExit = (TraceToExitFn)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 EC 30 F3 0F 10 75");

	if (!oTraceToExit)
		return false;

	return oTraceToExit(end, tr, start.x, start.y, start.z, vEnd.x, vEnd.y, vEnd.z, trace);
}
static Vector umomaim;
bool HandleBulletPenetration(WeaponInfo_t *wpn_data, FireBulletData &data, bool extracheck)
{
	surfacedata_t *enter_surface_data = g_pSurfaceData->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
	data.trace_length += data.enter_trace.flFraction * data.trace_length_remaining;
	data.current_damage *= pow(wpn_data->range_modifier, (data.trace_length * 0.002));
	if ((data.trace_length > 3000.f) || (enter_surf_penetration_mod < 0.1f))
		data.penetrate_count = 0;
	if (data.penetrate_count <= 0)
		return false;
	Vector dummy;
	C_Trace trace_exit;
	if (!TraceToExitalt(dummy, data.enter_trace, data.enter_trace.end, data.direction, &trace_exit))
		return false;
	surfacedata_t *exit_surface_data = g_pSurfaceData->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_mod = exit_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;
	if (((data.enter_trace.contents & contents_grate) != 0) || (enter_material == 89) || (enter_material == 71)) 
	{
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f; 
	}
	else 
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;
	if (enter_material == exit_material)
	{
		if (exit_material == 87 || exit_material == 85)combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)combined_penetration_modifier = 2.0f;
	}
	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / wpn_data->penetration) * 1.25f);
	float thickness = VectorLength(trace_exit.end - data.enter_trace.end);
	if (extracheck) {
		if (!VectortoVectorVisible(trace_exit.end, umomaim))
			return false;
	}
	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;
	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)
		return false;
	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;
	if (data.current_damage < 1.0f)
		return false;
	data.src = trace_exit.end;
	data.penetrate_count--;

	return true;
}

float Autowall::Damage(const Vector &point)
{
	auto data = FireBulletData(g::local->GetEyePosition(), g::local);

	Vector angles;
	angles = g_Math.CalcAngle(data.src, point);
	g_Math.AngleVectors(angles, &data.direction);
	VectorNormalize(data.direction);

	if (SimulateFireBullet(g::local, g::local->GetActiveWeapon(), data))
		return data.current_damage;

	return 0.f;
}
void CalcAngle(Vector src, Vector dst, Vector& angles)
{
	Vector delta = src - dst;
	double hyp = delta.Length2D(); //delta.Length
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.00;

	if (delta.x >= 0.0)
		angles.y += 180.0f;
}
bool Autowall::CanWallbang(float& dmg)
{
	if (!g::local)
		return false;

	FireBulletData data = FireBulletData(g::local->GetEyePosition(), g::local);
	data.filter = C_TraceFilter(g::local);

	Vector EyeAng;
	g_pEngine->GetViewAngles(EyeAng);

	Vector dst, forward;

	g_Math.AngleVectors(EyeAng, &forward);
	dst = data.src + (forward * 8196.f);

	Vector angles = g_Math.CalcAngle(data.src, dst);
	g_Math.AngleVectors(angles, &data.direction);
	g_Math.VectorNormalize(data.direction);

	data.penetrate_count = 1;
	data.trace_length = 0.0f;

	WeaponInfo_t * weaponData = (WeaponInfo_t * )g::local->GetActiveWeapon()->GetCSWpnData();

	if (!weaponData)
		return false;

	data.current_damage = (float)weaponData->damage;

	data.trace_length_remaining = weaponData->range - data.trace_length;

	Vector end = data.src + data.direction * data.trace_length_remaining;

	TraceLine(data.src, end, mask_shot | contents_grate, g::local, &data.enter_trace);

	if (data.enter_trace.flFraction == 1.0f)
		return false;

	if (HandleBulletPenetration(weaponData, data, false))
	{
		dmg = data.current_damage;
		return true;
	}

	return false;
}
bool Autowall::CanHitFloatingPoint(const Vector &point, const Vector &source) // ez
{
	umomaim = point;

	if (!g::local)
		return false;

	FireBulletData data = FireBulletData(source, g::local);

	Vector angles = g_Math.CalcAngle(data.src, point);
	g_Math.AngleVectors(angles, &data.direction);
	VectorNormalize(data.direction);

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)g::local->GetActiveWeapon();

	if (!pWeapon)
		return false;

	data.penetrate_count = 1;
	data.trace_length = 0.0f;

	WeaponInfo_t *weaponData = pWeapon->GetCSWpnData();

	if (!weaponData)
		return false;

	data.current_damage = (float)weaponData->damage;
	data.trace_length_remaining = weaponData->range - data.trace_length;
	Vector end = data.src + (data.direction * data.trace_length_remaining);
	TraceLine(data.src, end, mask_shot | contents_hitbox, g::local, &data.enter_trace);

	if (VectortoVectorVisible(data.src, point) || HandleBulletPenetration(weaponData, data, true))
		return true;

	return false;
}
bool trace_to_exit_short(Vector& point, Vector& dir, const float step_size, float max_distance)
{
	float flDistance = 0;

	while (flDistance <= max_distance)
	{
		flDistance += step_size;

		point += dir * flDistance;
		int point_contents = g_pTrace->GetPointContents(point, mask_shot_hull);
		if (!(point_contents& mask_shot_hull))
		{
			// found first free point
			return true;
		}
	}

	return false;
}
float Autowall::get_thickness(Vector& start, Vector& end, float distance) {
	Vector dir = end - start;
	Vector step = start;
	if (dir.Length() > distance&& distance != -1)
		return -1;

	dir.NormalizeInPlace();
	CTraceFilterWorldOnly filter;
	C_Trace trace;
	C_Ray ray(start, end);
	float thickness = 0;
	while (true) {
		g_pTrace->TraceRay(ray, mask_shot_hull, &filter, &trace);

		if (!trace.DidHit())
			return thickness;

		const Vector lastStep = trace.end;
		step = trace.end;

		if ((end - start).Length() <= (step - start).Length())
			break;

		if (!trace_to_exit_short(step, dir, 5, 90))
			return FLT_MAX;


		thickness += (step - lastStep).Length();
	}
	return thickness;
}