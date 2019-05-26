#include "GlobalVars.h"

namespace g
{
	CUserCmd* cmd = nullptr;
	C_BaseEntity* local = nullptr;
	std::uintptr_t uRandomSeed = NULL;
	Vector         OriginalView;
	bool           bSendPacket = true;
	bool		   LagPeek = false;
	bool           fake_duck;
	int            config_size = 3;
	int            choked_commands;
	bool           flip_bool;
	int            config_slct;
	bool           slow_walk;
	bool           desync_menu;
	Vector         stored_angles;
	float          pose_param_11[65];
	bool           desync;
	bool           should_choke;
	int            TargetIndex = -1;
	Vector         EnemyEyeAngs[65];
	Vector         AimbotHitbox[65][28];
	Vector         RealAngle;
	Vector         FakeAngle;
	int            balance;
	bool           Shot[65];
	bool           Hit[65];
	int            MissedShots[65];
	DWORD Verdana;
	DWORD CourierNew;
	DWORD money_font;
	DWORD Tahoma;
	DWORD menu_font;
	DWORD watermark_font;
	DWORD menu_font2;
	DWORD title_font;
	DWORD tab_font;
	DWORD icon_font;
	DWORD esp_font;
	DWORD subtab_font;
	DWORD misc_font;
	DWORD esp_font2;
}
