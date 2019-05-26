#pragma once
#include <Windows.h>
#include <string>
#include <filesystem>
#include "..\Utils\GlobalVars.h"
#include "..\SDK\ISurface.h"
namespace fs = std::experimental::filesystem;
#define menurgba Color(30, 144, 255)
#define menurgb( x ) Color(30, 144, x)
struct ColorV2
{
	float red, green, blue, alpha;
};

enum
{
	check_box,
	slider,
	combo_box,
	multi_box
};

class Menu
{
public:
	bool menuOpened;
	void Render();
	int alpha;
	int MenuAlpha_Main;
	int MenuAlpha_Text;
	bool in_drag;
	std::vector< std::string > itemname = { "Config 1", "Config 2", "Config 3" };
	struct
	{
		struct {
			bool active;
			int fov;
			int hitchance;
			bool hitboxes[5] = { false, false, false, false, false };

			bool trigactive;
			int trighitchance;
			bool trighitboxes[5] = { false, false, false, false, false };
		}legit;

		struct {
			bool Aimbot;
			int AimFOV;
			bool AutoFire;
			bool RCS;
			int Hitchance;

			//hitchance
			int HitchanceRifle;
			int HitchancePistol;
			int HitchanceSniper;
			int HitchanceAutosniper;
			int HitchanceHeavy;
			int HitchanceShotgun;
			int HitchanceSMG;
			 
			//min damage
			int MinimumDMGRifle;
			int MinimumDMGPistol;
			int MinimumDMGSniper;
			int MinimumDMGAutosniper;
			int MinimumDMGHeavy;
			int MinimumDMGShotgun;
			int MinimumDMGSMG;

			bool Resolver;
			int BodyScale;
			int HeadScale;
			bool MultiPoint;
			bool DelayShot;
			bool IgnoreLimbs;
			bool Autostop;
			bool FixShotPitch;
			bool PosBacktrack;
			bool ShotBacktrack;
			bool baim_lethal;
			bool BaimPitch;
			bool baim_air;

			bool hitboxes[5] = { false, false, false, false, false };
			int priority_hitbox;
			bool Antiaim;
			bool DesyncAngle;
			bool StandDesyncAngle;
			bool MoveDesyncAngle;

			int yaw_stand;
			int pitch_stand;
			int yaw_move;
			int pitch_move;
			int desync_mode;
			int stand_velocity_threshold;

			bool DesyncAngle2;
			bool RandJitterInRange;
			int	JitterRange;
		}aimbot;

		struct {
			int Viewmodel;
			int 		ViewmodelX;
			int 		ViewmodelY;
			int 		ViewmodelZ;
			int 		nViewmodelX;
			int 		nViewmodelY;
			int 		nViewmodelZ;
			int         roll;
			int         swing;
			int         tilt;
			int Fov;
			int	nightmode;
			int	propalpha;
			bool updateworld;
			bool AutowallCross;
			int worldalpha;
			bool Esp;
			int Font;
			ColorV2 FontColor = { 255.f,255.f,255.f,255.f };
			bool Name;
			bool HealthVal[2] = { false, false };
			bool Weapon;
			bool pene_dot;
			bool player_flags;
			bool flags[4] = { false, false, false, false }; //"Scoped", "Crouched", "Helmet-Kev"
			bool Box;
			bool ammo[2] = { false,false };;
			ColorV2 BoxColor = { 255.f,255.f,255.f,255.f };
			bool HealthBar;
			bool Skeleton;
			ColorV2 SkeletonColor = { 255.f,255.f,255.f,255.f };
			bool HitboxPoints;
			bool fovarrow;
			int fov_arrows_dia;
			int fov_arrows_size;
			bool Chams;
			int chammat;
			bool ChamsXQZ;
			bool localpulse;
			bool local_chams;
			int chams_opacity;
			int local_chams_opacity;
			ColorV2 localchamscolor = { 255.f, 255.f, 255.f, 255.f };
			int localchammat;

			bool glow;
			int glow_alpha;
			ColorV2 ChamsColor = { 255.f,255.f,255.f,255.f };
			ColorV2 ChamsColorXQZ = { 255.f,255.f,255.f,255.f };
			ColorV2 glow_color = { 255.f, 255.f, 255.f, 255.f };
			ColorV2 fov_arrows_color = { 255.f, 255.f, 255.f, 255.f };
			ColorV2 desynccolor = { 255.f, 255.f, 255.f, 255.f };
			bool btchams;
			ColorV2 btchamcolor = { 255.f, 255.f, 255.f, 255.f };
			int btchams_opacity;
			bool desync_chams;
			int fakelagchams_opacity;
			bool fakelagchams;
			ColorV2 fakelagcolor = { 255.f, 255.f, 255.f, 255.f };
		}visuals;

		struct {
			int	Fakelag;

			int TpDIST;
			int Hitmarkers;
			bool Thirdperson;

			bool NoZoom;
			bool NoScope;
			bool NoRecoil;
			int aspect_ratio;
			bool nosmoke;
			bool noflash;
			int nosmokeset;
			bool preservekillfeed;
			bool killsay;
			bool EventLog[2] = { false,false };
			bool EventLogs;

			bool bullet_tracer;
			ColorV2 bullet_tracecol = { 255.f, 255.f, 255.f, 255.f };
			int RadarY;
			bool hitmarker;
			int RadarX;
			bool slowwalk;
			int spec_x;
			bool bomb_timer;
			bool fakeduck;
			int spec_y;
			bool clantag;
			bool spectators;
			bool bullet_impacts;
			int RadarScale;
			bool Radar;
			int ConfigList;
			bool ConfigLoad;
			bool ConfigSave;
			bool ConfigAdd;
			bool ConfigOpen;
			bool ResetCFG;
			bool Bhop;
			bool watermark;
			bool AutoStrafe;

			ButtonCode_t tp_key;
		}misc;

		struct {
			bool forceupdate;
			bool active;
		}skins;
	}config;

	bool get_key_press(unsigned int key) {
		if (keys[key] == true && old_keys[key] == false)
			return true;
		else
			return false;
	}
private:
	struct
	{
		float x = 500.f, y = 200.f;
	}Pos; // lol

	enum
	{
		check_box,
		slider,
		combo_box,
		multi_box,
		button
	};

	enum {
		subtab1 = 0,
		subtab2 = 1,
		subtab3 = 0,
		subtab4 = 1,
		subtab5 = 0,
		subtab6 = 1,

		tab_rifle = 0,
		tab_sniper = 1,
		tab_pistol = 2,
		tab_autosniper = 3,
		tab_heavy = 4,
		tab_shotgun = 5,
		tab_smg = 6,

		tab_lrifle = 0,
		tab_lsniper = 1,
		tab_lpistol = 2,
		tab_lautosniper = 3,
		tab_lheavy = 4,
		tab_lshotgun = 5,
		tab_lsmg = 6
	};

	bool keys[256];
	bool old_keys[256];
	int ControlsX;
	int GroupTabBottom;
	int OffsetY;
	int OldOffsetY;
	int TabOffset;
	int SubTabOffset;
	int tab_selected_v2, tab_selected_v3, tab_selected_visuals, tab_selected_weapon, tab_selected_lweapon;
	float SubTabSize;
	float TabSize;
	int GroupTabPos[4];

	int TabNum = 0;
	int SubTabNum = 0;
	int PreviousControl = -1;

	void draw_radar();
	void spectators_list();

	void Tab(std::string name);
	void SubTab(std::string name);
	void CheckBox(std::string name, bool* item);
	void Slider(int max, std::string name, int* item, int step = 1);
	//void ComboBox();
	void MultiComboBox(std::string name, std::vector< std::string > itemname, bool* item);
	void ColorPicker(std::string name, ColorV2& item, int y = 0);
	void group_box(std::string name, int x, int y, int w, int h);
	void Button(std::string name, bool* item, int plus_this);
	void text(std::string name, int x, int y);
	void Listbox(std::string name, std::vector< std::string > itemname, int* item);
	//void KeyBindButton(ButtonCode_t* key, int y);
	void ComboBox(std::string name, std::vector< std::string > itemname, int* item);
	void ThinkComboBox();
	void DrawComboBox();
	int* cfg_item;
	std::string control_name;
	std::vector<std::string> items;

	void second_tabs(std::string str, std::string str_v2, int x, int y, int gap, int additive);
	void third_tabs(std::string str, std::string str_v2, int x, int y, int gap, int additive);
	void visuals_tabs(std::string str, std::string str_v2, int x, int y, int gap, int additive);
	void weapon_tabs();
	void legit_weapon_tabs();

	void run_game();

};
//extern float ratio = 0.5;
extern Menu g_menu;