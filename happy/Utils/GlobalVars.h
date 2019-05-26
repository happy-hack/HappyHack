#pragma once
#include "..\SDK\CInput.h"
#include "..\SDK\CEntity.h"

namespace g
{
	extern CUserCmd* cmd;
	extern C_BaseEntity* local;
	extern std::uintptr_t uRandomSeed;
	extern Vector         OriginalView;
	extern bool           bSendPacket;
	extern Vector         stored_angles;
	extern int            choked_commands;
	extern int            config_size;
	extern bool           slow_walk;
	extern bool           flip_bool;
	extern int            config_slct;
	extern bool           desync_menu;
	extern bool			  LagPeek;
	extern bool           should_choke;
	extern bool           fake_duck;
	extern float          pose_param_11[65];
	extern bool           desync;
	extern int            TargetIndex;
	extern Vector         EnemyEyeAngs[65];
	extern Vector         AimbotHitbox[65][28];
	extern Vector         RealAngle;
	extern Vector         FakeAngle;
	extern bool           Shot[65];
	extern bool           Hit[65];
	extern int            balance;
	extern int            MissedShots[65];
	extern DWORD Verdana;
	extern DWORD CourierNew;
	extern DWORD Tahoma;
	extern DWORD menu_font;
	extern DWORD menu_font2;
	extern DWORD icon_font;
	extern DWORD watermark_font;
	extern DWORD tab_font;
	extern DWORD title_font;
	extern DWORD esp_font;
	extern DWORD esp_font2;
	extern DWORD misc_font;
	extern DWORD money_font;
	extern DWORD subtab_font;
}