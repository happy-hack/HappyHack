#include "Aimbot.h"
#include "Autowall.h"
#include "LagComp.h"
#include "..\AntiAim\AntiAim.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\SDK\ICvar.h"
#include "..\..\Utils\Math.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\Menu\Menu.h"

Aimbot g_Aimbot;
void Aimbot::Autostop()
{
	if (!g_menu.config.aimbot.Autostop || !(g::local->GetFlags() & FL_ONGROUND))
		return;

	auto velocity = g::local->GetVelocity().Length2D();
	auto weapon = g::local->GetActiveWeapon(); 
	if (!weapon) return;

	if (velocity >= (weapon->GetCSWpnData()->max_speed * .34)) {
		g::cmd->buttons |= IN_WALK;
		g::cmd->sidemove = -g::cmd->sidemove;
		g::cmd->forwardmove = -g::cmd->forwardmove;
		g::cmd->upmove = 0;
	}
}
#define square( x ) ( x * x )
void ClampMovement(float fMaxSpeed) {
	if (!g_menu.config.aimbot.Autostop || !(g::local->GetFlags() & FL_ONGROUND)) return;
	if (fMaxSpeed <= 0.f) return;
	float fSpeed = (float)(FastSqrt(square(g::cmd->forwardmove) + square(g::cmd->sidemove) + square(g::cmd->upmove)));
	if (fSpeed <= 0.f) return;
	if (g::cmd->buttons& IN_DUCK) fMaxSpeed *= 2.94117647f;
	if (fSpeed <= fMaxSpeed)return;
	float fRatio = fMaxSpeed / fSpeed;
	g::cmd->forwardmove *= fRatio;
	g::cmd->sidemove *= fRatio;
	g::cmd->upmove *= fRatio;
}
bool ShouldBaim(C_BaseEntity* pEnt) {
	static float oldSimtime[65];
	static float storedSimtime[65];

	static float ShotTime[65];
	static float NextShotTime[65];
	static bool BaimShot[65];

	if (storedSimtime[pEnt->EntIndex()] != pEnt->GetSimulationTime())
	{
		oldSimtime[pEnt->EntIndex()] = storedSimtime[pEnt->EntIndex()];
		storedSimtime[pEnt->EntIndex()] = pEnt->GetSimulationTime();
	}

	float simDelta = storedSimtime[pEnt->EntIndex()] - oldSimtime[pEnt->EntIndex()];

	bool Shot = false;

	if (pEnt->GetActiveWeapon() && !pEnt->IsKnife() && !pEnt->IsNade())
	{
		if (ShotTime[pEnt->EntIndex()] != pEnt->GetActiveWeapon()->GetLastShotTime())
		{
			Shot = true;
			BaimShot[pEnt->EntIndex()] = false;
			ShotTime[pEnt->EntIndex()] = pEnt->GetActiveWeapon()->GetLastShotTime();
		}
		else
			Shot = false;
	}
	else
	{
		Shot = false;
		ShotTime[pEnt->EntIndex()] = 0.f;
	}

	if (Shot)
	{
		NextShotTime[pEnt->EntIndex()] = pEnt->GetSimulationTime() + pEnt->FireRate();

		if (simDelta >= pEnt->FireRate())
			BaimShot[pEnt->EntIndex()] = true;
	}

	if (BaimShot[pEnt->EntIndex()])
	{
		if (pEnt->GetSimulationTime() >= NextShotTime[pEnt->EntIndex()])
			BaimShot[pEnt->EntIndex()] = false;
	}

	if (g_menu.config.aimbot.baim_air && !(pEnt->GetFlags() & FL_ONGROUND))
		return true;

	if (pEnt->GetVelocity().Length2D() > 0 && pEnt->GetVelocity().Length2D() < 100 && !(pEnt->GetFlags()& FL_DUCKING))
		return true;

	return false;
}
Vector Aimbot::Hitscan(C_BaseEntity * pEnt)
{
	float dmgtohitbox_[28];
	float temp_dmg = 0.f;
	Vector tempHitbox = { 0,0,0 };

	float player_vel = abs(pEnt->GetVelocity().Length2D());
	std::vector<int> scan;
	int HeadHeight = 0.5, BodyHeight = 0.5;

	float angToLocal = g_Math.CalcAngle(g::local->GetOrigin(), pEnt->GetOrigin()).y;
	Vector2D MutipointXY = { (sin(g_Math.GRD_TO_BOG(angToLocal))),(cos(g_Math.GRD_TO_BOG(angToLocal))) };
	Vector2D MutipointXY180 = { (sin(g_Math.GRD_TO_BOG(angToLocal + 180))) ,(cos(g_Math.GRD_TO_BOG(angToLocal + 180))) };
	Vector2D Mutipoint[] = { Vector2D(MutipointXY.x, MutipointXY.y), Vector2D(MutipointXY180.x, MutipointXY180.y) };

	bool body_aim = ShouldBaim(pEnt);

	if (g::local->GetActiveWeapon()->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_TASER) {
		if (!body_aim) {
			if (g_menu.config.aimbot.hitboxes[0]) { //Head, Chest, Pelvis/Arms, Legs/Feet
				scan.push_back(HITBOX_HEAD);
				scan.push_back(HITBOX_NECK);
			}
			if (g_menu.config.aimbot.hitboxes[1]) {
				scan.push_back(HITBOX_LOWER_CHEST);
				scan.push_back(HITBOX_UPPER_CHEST);
			}
			if (g_menu.config.aimbot.hitboxes[2]) {
				scan.push_back(HITBOX_PELVIS);
				scan.push_back(HITBOX_THORAX);
				scan.push_back(HITBOX_LEFT_FOREARM);
				scan.push_back(HITBOX_RIGHT_FOREARM);
				scan.push_back(HITBOX_LEFT_HAND);
				scan.push_back(HITBOX_RIGHT_HAND);
				scan.push_back(HITBOX_RIGHT_UPPER_ARM);
				scan.push_back(HITBOX_LEFT_UPPER_ARM);
			}
			if (g_menu.config.aimbot.hitboxes[3]) {
				scan.push_back(HITBOX_LEFT_CALF);
				scan.push_back(HITBOX_RIGHT_CALF);
				scan.push_back(HITBOX_LEFT_THIGH);
				scan.push_back(HITBOX_RIGHT_THIGH);
			}
			if (g_menu.config.aimbot.MultiPoint) {
				HeadHeight = g_menu.config.aimbot.HeadScale;
			}
		}
		else {
			scan.push_back(HITBOX_PELVIS);
			scan.push_back(HITBOX_THORAX);
			scan.push_back(HITBOX_LOWER_CHEST);
			scan.push_back(HITBOX_UPPER_CHEST);
			scan.push_back(HITBOX_BELLY);

			if (g_menu.config.aimbot.MultiPoint) {
				scan.push_back(19);//pelvis
				scan.push_back(20);

				scan.push_back(21);//thorax
				scan.push_back(22);

				scan.push_back(23);//upperchest
				scan.push_back(24);

				BodyHeight = g_menu.config.aimbot.BodyScale;
			}
		}
		if (player_vel > 150.f) {
			scan.push_back(HITBOX_PELVIS);
			scan.push_back(HITBOX_LEFT_HAND);
			scan.push_back(HITBOX_LEFT_UPPER_ARM);
			scan.push_back(HITBOX_NECK);
			scan.push_back(HITBOX_THORAX);
			scan.push_back(HITBOX_LOWER_CHEST);
			scan.push_back(HITBOX_BELLY);
			scan.push_back(HITBOX_LEFT_THIGH);
		}
	}
	else {
		scan.push_back(HITBOX_PELVIS);
		scan.push_back(HITBOX_THORAX);
		scan.push_back(HITBOX_LOWER_CHEST);
		scan.push_back(HITBOX_UPPER_CHEST);
		scan.push_back(HITBOX_BELLY);
	}

	Vector exit_hitbox;
	int best_hitbox = 0;

	for (int hitbox : scan)
	{
		bool aim_head_area = true ? (hitbox == HITBOX_HEAD || hitbox == HITBOX_NECK || hitbox == HITBOX_UPPER_CHEST) : false;
		bool aim_body_area = true ? 
			(hitbox == HITBOX_LOWER_CHEST || hitbox == HITBOX_PELVIS || hitbox == HITBOX_BELLY || 
			hitbox == HITBOX_THORAX || hitbox == HITBOX_UPPER_CHEST || hitbox == HITBOX_LEFT_UPPER_ARM || 
			hitbox == HITBOX_RIGHT_UPPER_ARM || hitbox == HITBOX_LEFT_CALF || hitbox == HITBOX_LEFT_THIGH || 
			hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_RIGHT_CALF) : false;

		if (aim_head_area)
		{
			float Radius = 0;
			exit_hitbox = pEnt->GetHitboxPosition(hitbox, Matrix[pEnt->EntIndex()], &Radius);
			Radius *= (HeadHeight / 100.f);
			exit_hitbox = Vector(exit_hitbox.x + (Radius * Mutipoint[((hitbox - 25) % 2)].x), exit_hitbox.y - (Radius * Mutipoint[((hitbox - 25) % 2)].y), exit_hitbox.z);
		}
		else if (aim_body_area)
		{
			float radius = 0;
			exit_hitbox = pEnt->GetHitboxPosition(hitbox, Matrix[pEnt->EntIndex()], &radius);
			radius *= (BodyHeight / 100.f);
			exit_hitbox = Vector(exit_hitbox.x + (radius * Mutipoint[((hitbox - 19) % 2)].x), exit_hitbox.y - (radius * Mutipoint[((hitbox - 19) % 2)].y), exit_hitbox.z);
		}

		float damage = g_Autowall.Damage(exit_hitbox);
		if (damage > 0.f)
			dmgtohitbox_[hitbox] = damage;
		else
			dmgtohitbox_[hitbox] = 0;

		if (g_menu.config.aimbot.baim_lethal && damage >= pEnt->GetHealth()) {
			dmgtohitbox_[hitbox] = 400;
			body_aim = true;
		}

		if (dmgtohitbox_[hitbox] > temp_dmg) {
			tempHitbox = exit_hitbox;
			best_hitbox = hitbox;
			temp_dmg = dmgtohitbox_[hitbox];
		}
	}

	PlayerRecords pPlayerEntityRecord = g_LagComp.PlayerRecord[pEnt->EntIndex()].at(0);

	Backtrack[pEnt->EntIndex()] = false;
	ShotBacktrack[pEnt->EntIndex()] = false;

	int menu_mindamage = 0;
	if (g::local->GetActiveWeapon()->GetCSWpnData()->type == c_weapon_types::type_rifle)
		menu_mindamage = g_menu.config.aimbot.MinimumDMGRifle;
	else if (g::local->GetActiveWeapon()->GetCSWpnData()->type == c_weapon_types::type_pistol)
		menu_mindamage = g_menu.config.aimbot.MinimumDMGPistol;
	else if (g::local->GetActiveWeapon()->GetCSWpnData()->type == c_weapon_types::type_sniper &&
		(g::local->GetActiveWeapon()->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_SCAR20
			&& g::local->GetActiveWeapon()->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_G3SG1))
		menu_mindamage = g_menu.config.aimbot.MinimumDMGSniper;
	else if ((g::local->GetActiveWeapon()->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SCAR20
		|| g::local->GetActiveWeapon()->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_G3SG1))
		menu_mindamage = g_menu.config.aimbot.MinimumDMGAutosniper;
	else if (g::local->GetActiveWeapon()->GetCSWpnData()->type == c_weapon_types::type_machinegun)
		menu_mindamage = g_menu.config.aimbot.MinimumDMGHeavy;
	else if (g::local->GetActiveWeapon()->GetCSWpnData()->type == c_weapon_types::type_shotgun)
		menu_mindamage = g_menu.config.aimbot.MinimumDMGShotgun;
	else if (g::local->GetActiveWeapon()->GetCSWpnData()->type == c_weapon_types::type_smg)
		menu_mindamage = g_menu.config.aimbot.MinimumDMGSMG;

	auto hitbox_priority = 0;
	switch (g_menu.config.aimbot.priority_hitbox) {
	case 0:
		hitbox_priority = (HITBOX_HEAD);
	break;
	case 1: {
		hitbox_priority = (HITBOX_LOWER_CHEST);
		hitbox_priority = (HITBOX_UPPER_CHEST);
	}
	break;
	case 2: {
		hitbox_priority = (HITBOX_PELVIS);
		hitbox_priority = (HITBOX_THORAX);
		hitbox_priority = (HITBOX_LEFT_FOREARM);
		hitbox_priority = (HITBOX_RIGHT_FOREARM);
		hitbox_priority = (HITBOX_LEFT_HAND);
		hitbox_priority = (HITBOX_RIGHT_HAND);
		hitbox_priority = (HITBOX_RIGHT_UPPER_ARM);
		hitbox_priority = (HITBOX_LEFT_UPPER_ARM);
	}
	break;
	case 3: {
		hitbox_priority = (HITBOX_LEFT_CALF);
		hitbox_priority = (HITBOX_RIGHT_CALF);
		hitbox_priority = (HITBOX_LEFT_FOOT);
		hitbox_priority = (HITBOX_RIGHT_FOOT);
		hitbox_priority = (HITBOX_LEFT_THIGH);
		hitbox_priority = (HITBOX_RIGHT_THIGH);
		}
	}

	Backtrack[pEnt->EntIndex()] = false;

	float damage_to_priority = g_Autowall.Damage(pEnt->GetHitboxPosition(hitbox_priority, str_matrix.matrix[pEnt->EntIndex()]));
	if (damage_to_priority > 1) {
		return pEnt->GetHitboxPosition(hitbox_priority, str_matrix.matrix[pEnt->EntIndex()]);
	}
	else if (temp_dmg >= menu_mindamage)
	{
		if ((best_hitbox == 25 || best_hitbox == 26 || best_hitbox == 27) && abs(dmgtohitbox_[HITBOX_HEAD] - dmgtohitbox_[best_hitbox]) <= 10.f)
			return pEnt->GetHitboxPosition(HITBOX_HEAD, str_matrix.matrix[pEnt->EntIndex()]);
		else if ((best_hitbox == 19 || best_hitbox == 20) && dmgtohitbox_[HITBOX_PELVIS] > 30)
			return pEnt->GetHitboxPosition(HITBOX_PELVIS, str_matrix.matrix[pEnt->EntIndex()]);
		else if ((best_hitbox == 21 || best_hitbox == 22) && dmgtohitbox_[HITBOX_THORAX] > 30)
			return pEnt->GetHitboxPosition(HITBOX_THORAX, str_matrix.matrix[pEnt->EntIndex()]);
		else if ((best_hitbox == 23 || best_hitbox == 24) && dmgtohitbox_[HITBOX_UPPER_CHEST] > 30)
			return pEnt->GetHitboxPosition(HITBOX_UPPER_CHEST, str_matrix.matrix[pEnt->EntIndex()]);

		return tempHitbox;
	}
	else if (g_menu.config.aimbot.PosBacktrack && pPlayerEntityRecord.Velocity >= 29.f && g_Autowall.CanHitFloatingPoint(pEnt->GetHitboxPosition(HITBOX_HEAD, pPlayerEntityRecord.Matrix), g::local->GetEyePosition()))
	{
		bestEntDmg = (100000.f - fabs(g_Math.Distance(Vector2D(g::local->GetOrigin().x, g::local->GetOrigin().y), Vector2D(pEnt->GetOrigin().x, pEnt->GetOrigin().y))));
		Backtrack[pEnt->EntIndex()] = true;
		return pEnt->GetHitboxPosition(HITBOX_HEAD, pPlayerEntityRecord.Matrix);
	}

	return Vector(0, 0, 0);
}
void vector_angles(const Vector& forward, Vector& angles) {
	float tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0) {
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else {
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
Vector calc_angle_v2(Vector src, Vector dst)
{
	Vector ret;
	vector_angles(dst - src, ret);
	return ret;
}
void Aimbot::OnCreateMove()
{
	if (!g_pEngine->IsInGame())
		return;

	Vector Aimpoint = { 0,0,0 };
	C_BaseEntity * Target = nullptr;

	int target_id = 0;

	if (!g_menu.config.aimbot.Aimbot) return;

	for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i)
	{
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| pPlayerEntity->IsDormant())
		{
			g_LagComp.ClearRecords(i);
			continue;
		}

		g_LagComp.StoreRecord(pPlayerEntity);

		if (pPlayerEntity == g::local || pPlayerEntity->GetTeam() == g::local->GetTeam())
			continue;

		g::EnemyEyeAngs[i] = pPlayerEntity->GetEyeAngles();

		if (g_LagComp.PlayerRecord[i].size() == 0 || !g::local->IsAlive())
			continue;

		if (!g::local->GetActiveWeapon() || g::local->IsKnife() || g::local->IsNade())
			continue;

		Vector Hitbox = Hitscan(pPlayerEntity);

		if (Hitbox != Vector(0, 0, 0)) {
			Aimpoint = Hitbox;
			Target = pPlayerEntity;
			target_id = Target->EntIndex();
		}
	}

	float flServerTime = g::local->GetTickBase() * g_pGlobalVars->intervalPerTick;
	bool canShoot = (g::local->GetActiveWeapon()->GetNextPrimaryAttack() <= flServerTime);

	if (Target)
	{
		g::TargetIndex = target_id;

		Vector Angle = calc_angle_v2(g::local->GetEyePosition(), Aimpoint);


		float SimulationTime = 0.f;

		if (Backtrack[target_id])
			SimulationTime = g_LagComp.PlayerRecord[target_id].at(0).SimTime;
		else
			SimulationTime = g_LagComp.PlayerRecord[target_id].at(g_LagComp.PlayerRecord[target_id].size() - 1).SimTime;

		int menu_hitchance = 0;
		if (g::local->GetActiveWeapon()->GetCSWpnData_2()->WeaponType == c_weapon_types::type_rifle)
			menu_hitchance = g_menu.config.aimbot.HitchanceRifle;
		else if (g::local->GetActiveWeapon()->GetCSWpnData_2()->WeaponType == c_weapon_types::type_pistol)
			menu_hitchance = g_menu.config.aimbot.HitchancePistol;
		else if (g::local->GetActiveWeapon()->GetCSWpnData_2()->WeaponType == c_weapon_types::type_sniper &&
			(g::local->GetActiveWeapon()->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_SCAR20
				&& g::local->GetActiveWeapon()->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_G3SG1))
			menu_hitchance = g_menu.config.aimbot.HitchanceSniper;
		else if ((g::local->GetActiveWeapon()->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SCAR20
			|| g::local->GetActiveWeapon()->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_G3SG1))
			menu_hitchance = g_menu.config.aimbot.HitchanceAutosniper;
		else if (g::local->GetActiveWeapon()->GetCSWpnData_2()->WeaponType == c_weapon_types::type_machinegun)
			menu_hitchance = g_menu.config.aimbot.HitchanceHeavy;
		else if (g::local->GetActiveWeapon()->GetCSWpnData_2()->WeaponType == c_weapon_types::type_shotgun)
			menu_hitchance = g_menu.config.aimbot.HitchanceShotgun;
		else if (g::local->GetActiveWeapon()->GetCSWpnData_2()->WeaponType == c_weapon_types::type_smg)
			menu_hitchance = g_menu.config.aimbot.HitchanceSMG;

		if (!GetAsyncKeyState(VK_SPACE) && !Target->IsImmune())
			ClampMovement(25);

		if (!(g::cmd->buttons & IN_ATTACK) && canShoot && hitchance(menu_hitchance) && !Target->IsImmune())
		{
			g::cmd->viewangles = Angle - (g::local->GetAimPunchAngle() * g_pCvar->FindVar("weapon_recoil_scale")->GetFloat());
			g::cmd->buttons |= IN_ATTACK;
			g::cmd->tick_count = TIME_TO_TICKS(SimulationTime + g_LagComp.LerpTime());
		}
	}
}