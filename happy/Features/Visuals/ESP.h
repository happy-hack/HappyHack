#pragma once
#include "..\..\Utils\GlobalVars.h"

struct TestPos
{
	Vector Pos;
	float SimulationTime;
};

class ESP
{
public:
	void Render();
private:
	struct
	{
		int left, top, right, bottom;
	}Box;

	Color color;
	Color textcolor;
	Color skelecolor;
	DWORD font;
	int offsetY;
	void RenderSkeleton(C_BaseEntity* pEnt);
	float esp_fov_fade[64];
	float esp_alpha_fade[64];
	void BoundBox(C_BaseEntity* pEnt, int alpha, int outline_alpha);
	void RenderBox(int alpha, int outline_alpha);
	void RenderWeaponName(C_BaseEntity* pEnt, int alpha, int outline_alpha);
	void ammo(C_BaseEntity* ent, int alpha, int outline_alpha);
	void RenderName(C_BaseEntity* pEnt, int iterator, int alpha, int outline_alpha);
	void RenderHealth(C_BaseEntity* pEnt, int alpha, int outline_alpha);
	void player_flags(C_BaseEntity* ent, int alpha, int outline_alpha);
	void fov_arrow(C_BaseEntity* ent, int alpha);
};
extern ESP g_ESP;