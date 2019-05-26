#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include <deque>

class Aimbot
{
public:
	matrix3x4_t Matrix[65][128];
	int Sequence;
	void OnCreateMove();
	int bestEntDmg;

private:
	bool Backtrack[65];
	bool ShotBacktrack[65];
	Vector Hitscan(C_BaseEntity* pEnt);
	void Autostop();
	float damage_to_hitbox;
	int menu_mindamage;

	bool hitchance(int hitchance) {
		float inaccuracy = g::local->GetActiveWeapon()->GetInaccuracy();

		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;

		return inaccuracy > hitchance;
	}

};
extern Aimbot g_Aimbot;