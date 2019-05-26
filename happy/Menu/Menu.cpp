#include "Menu.h"
#include "cfgsaving.h"
#include "..\SDK\Vector.h"
#include "..\Utils\Color.h"
#include "..\Utils\XorStr.h"
#include "..\SDK\CGlobalVarsBase.h"
#include <random>
Menu g_menu;

enum game_states {
	main_menu,
	in_play,
	in_credits,
	in_pause
};
float random_float(static float min, static float max)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(min, max);

	return dis(gen);
}

game_states state = main_menu;
void Menu::run_game() {
	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	static int tickcount = g_pGlobalVars->tickcount;

	static std::string status;
	static Color play_color = Color(43, 43, 43);
	static Color credits_color = Color(43, 43, 43);

	g_pSurface->FilledRect(Pos.x + 128, Pos.y + 40, 299, 332, Color(55, 55, 55)); //body
	g_pSurface->OutlinedRect(Pos.x + 128, Pos.y + 40, 299, 332, Color(0, 0, 0));

	if (state == main_menu) {
		status = "MAIN MENU";


		g_pSurface->draw_text(Pos.x + 200, Pos.y + 50, g::title_font, "Welcome to", false, Color(180, 180, 180));
		g_pSurface->draw_text(Pos.x + 200, Pos.y + 85, g::title_font, "happy place", false, Color(180, 180, 180));


		g_pSurface->RoundedFilledRect(Pos.x + 205, Pos.y + 135, 135, 135, 67.5, Color(0, 0, 0)); //credits button
		g_pSurface->RoundedFilledRect(Pos.x + 210, Pos.y + 140, 125, 125, 62.5, Color(170, 0, 0)); //credits button
		g_pSurface->draw_text(Pos.x + 225, Pos.y + 175, g::money_font, "100", false, Color(180, 180, 180));

		g_pSurface->FilledRect(Pos.x + 175, Pos.y + 290, 85, 38, play_color); //play button
		g_pSurface->OutlinedRect(Pos.x + 175, Pos.y + 290, 85, 38, Color(0, 0, 0));
		g_pSurface->draw_text(Pos.x + 195, Pos.y + 300, g::tab_font, "PLAY", false, Color(180, 180, 180));
		if (g_pSurface->MouseInRegion(Pos.x + 175, Pos.y + 290, 85, 38)) {
			play_color = Color(50, 50, 50);
			if (GetAsyncKeyState(VK_LBUTTON)) {
				state = in_play;
				status = "PLAY";
			}
		}
		else
			play_color = Color(43, 43, 43);

		g_pSurface->FilledRect(Pos.x + 280, Pos.y + 290, 85, 38, credits_color); //credits button
		g_pSurface->OutlinedRect(Pos.x + 280, Pos.y + 290, 85, 38, Color(0, 0, 0));
		g_pSurface->draw_text(Pos.x + 285, Pos.y + 300, g::tab_font, "CREDITS", false, Color(180, 180, 180));

		if (g_pSurface->MouseInRegion(Pos.x + 280, Pos.y + 290, 85, 38)) {
			credits_color = Color(50, 50, 50);
			if (GetAsyncKeyState(VK_LBUTTON)) {
				state = in_credits;
				status = "CREDITS";
			}
		}
		else
			credits_color = Color(43, 43, 43);
	}
	else if (state == in_play) {
		static int duration = 100;
		static int duation2 = 100;
		if (GetAsyncKeyState(VK_ESCAPE)) {
			state = main_menu;
		}	
		g_pSurface->draw_text(Pos.x + 138, Pos.y + 50, g::misc_font, std::to_string(g::balance).c_str() , false, Color(180, 180, 180));
	}
	else if (state == in_credits) {
		if (GetAsyncKeyState(VK_ESCAPE)) {
			state = main_menu;
		}
	}
	g_pSurface->draw_text(Pos.x + 128, Pos.y + 10, g::misc_font, status.c_str(), false, Color(180, 180, 180));
}
void OpenCFG()
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + "\\happyhack\\";
		ShellExecute(NULL, "open", folder.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}

}
void Menu::Render()
{
	static bool Pressed = false;

	if (!Pressed && GetAsyncKeyState(VK_INSERT))
		Pressed = true;

	else if (Pressed && !GetAsyncKeyState(VK_INSERT))
	{
		Pressed = false;
		menuOpened = !menuOpened;
	}

	// Update menu opacity.
	constexpr float fade_ratio = 1.0f / 0.70f;
	const     float  cur_ratio = fade_ratio * g_pGlobalVars->frametime;
	const     int    cur_inc = (int)(cur_ratio * 255.0f);

	int new_alpha = g_menu.alpha;
	new_alpha += g_menu.menuOpened ? cur_inc : -cur_inc;
	new_alpha = std::clamp< int >(new_alpha, 0, 255);

	g_menu.alpha = (size_t)new_alpha;

	static Vector2D oldPos;
	static Vector2D mousePos;

	static int dragX = 300;
	static int dragY = 300;
	static int Width = 500;
	static int Height = 400;
	static int Width2 = 552;
	static int Height2 = 402;
	static int _drag_x = 300;
	static int _drag_y = 300;
	static int _width = 475;
	static int _height = 520;

	static int iScreenWidth, iScreenHeight;

	static bool Dragging = false;
	bool click = false;

	if (config.misc.Radar)
		draw_radar();
	
	if (config.misc.spectators)
		spectators_list();
	

	if (menuOpened)
	{
		MenuAlpha_Main = min(MenuAlpha_Main + 25, 255);
		MenuAlpha_Text = min(MenuAlpha_Text + 15, 255);

		if (GetAsyncKeyState(VK_LBUTTON))
			click = true;

		g_pEngine->GetScreenSize(iScreenWidth, iScreenHeight);
		Vector2D MousePos = g_pSurface->GetMousePosition();

		if (Dragging && !click)
		{
			Dragging = false;
		}

		if (Dragging && click)
		{
			Pos.x = MousePos.x - dragX;
			Pos.y = MousePos.y - dragY;
		}

		if (g_pSurface->MouseInRegion(Pos.x, Pos.y, Width, 20))
		{
			Dragging = true;
			dragX = MousePos.x - Pos.x;
			dragY = MousePos.y - Pos.y;
		}

		if (Pos.x < 0)
			Pos.x = 0;

		if (Pos.y < 0)
			Pos.y = 0;

		if ((Pos.x + Width) > iScreenWidth)
			Pos.x = iScreenWidth - Width;

		if ((Pos.y + Height) > iScreenHeight)
			Pos.y = iScreenHeight - Height;

		/*static bool resizing = false;
		if (resizing && !click)
			resizing = false;

		if (resizing && click) {
			_width = MousePos.x - Pos.x;
			_height = MousePos.y - Pos.y;

			if (_width < 455) _width = 455;
			if (_height < 400) _height = 400;

			if (_width > 960) _width = 960;
			if (_height > 680) _height = 680;
		}

		if (g_pSurface->MouseInRegion(Pos.x + _width - 20, Pos.y + _height - 20, 20, 20))
			resizing = true;*/

		//background window
		g_pSurface->FilledRect(Pos.x - 5, Pos.y - 5, _width + 10, _height + 28, Color(52, 52, 52, MenuAlpha_Main));
		g_pSurface->OutlinedRect(Pos.x - 5, Pos.y - 5, _width + 10, _height + 28, Color(0, 0, 0));
		
		//entire window
		g_pSurface->FilledRect(Pos.x, Pos.y, _width, _height, Color(35, 35, 35, MenuAlpha_Main));
		g_pSurface->OutlinedRect(Pos.x - 1, Pos.y - 1, _width + 2, _height + 2, Color(0, 0, 0));

		//tab background
		g_pSurface->FilledRect(Pos.x + 13, Pos.y + 13, _width - 26, 38, Color(28, 28, 28, MenuAlpha_Main));

		/*for (int i = 0; i < 10; i++) {
			g_pSurface->Line(
				Pos.x + ((_width - 10) + i), Pos.y + _height - 1,
				Pos.x + _width, Pos.y + ((_height - 11) + i),
				menurgba); //resize line
		}*/
		g_pSurface->OutlinedRect(Pos.x + 13, Pos.y + 13, _width - 26, 38, Color(0, 0, 0));

		//g_pSurface->DrawT(Pos.x + 18, Pos.y + _height - 56, menurgba, g::title_font, false, "polia");
		//g_pSurface->DrawT(Pos.x + 15, Pos.y + _height - 32, Color(180, 180, 180), g::title_font, false, "cheat");


		//g_pSurface->FilledRect(Pos.x - 1, Pos.y + _height + 2, _width + 1, 22, Color(45, 45, 45));
		//g_pSurface->OutlinedRect(Pos.x - 1, Pos.y + _height + 2, _width + 2, 23, Color(10, 10, 10));
		g_pSurface->DrawT(Pos.x + 4, Pos.y + _height + 3, Color(180, 180, 180), g::misc_font, false, "happyhack v2.9");

		//line below tabs
		g_pSurface->FilledRect(Pos.x + 13, Pos.y + 65, _width - 26, 1, menurgba);
		g_pSurface->OutlinedRect(Pos.x + 13, Pos.y + 64, _width - 26, 3, Color(0, 0, 0));


		//control window
		g_pSurface->FilledRect(Pos.x + 13, Pos.y + 80, _width - 26, _height - 88, Color(43, 43, 43, MenuAlpha_Main));
		g_pSurface->OutlinedRect(Pos.x + 13, Pos.y + 80, _width - 26, _height - 88, Color(0, 0, 0));

		if (g_pSurface->MouseInRegion(Pos.x + _width - 150, Pos.y + _height + 3, 150, 30)) {
			if (GetAsyncKeyState(VK_LBUTTON))
				ShellExecute(NULL, "open", "happyhack", NULL, NULL, SW_SHOWNORMAL);

			g_pSurface->DrawT(Pos.x + _width - 150, Pos.y + _height + 3, Color(215, 215, 215), g::misc_font, false, "happyhack");
		}
		else
			g_pSurface->DrawT(Pos.x + _width - 150, Pos.y + _height + 3, Color(180, 180, 180), g::misc_font, false, "happyhack");

		TabOffset = 0;
		SubTabOffset = 0;
		PreviousControl = -1;
		OldOffsetY = 0;

		int gwidth = (_width - 60) / 2;
		int gheight = 400;
		int group1 = Pos.x + 23;
		int group2 = Pos.x + 240;
		int gy = Pos.y + 13;
		Tab("Aimbot");
		{
			group_box("Aimbot", group1, gy + 40, gwidth, 257);

			CheckBox("Active", &config.aimbot.Aimbot);
			weapon_tabs();
			if (tab_selected_weapon == tab_rifle) {
				Slider(100, "Hitchance", &config.aimbot.HitchanceRifle);
				Slider(100, "Minimum Damage", &config.aimbot.MinimumDMGRifle);
			}
			else if (tab_selected_weapon == tab_sniper) {
				Slider(100, "Hitchance", &config.aimbot.HitchanceSniper);
				Slider(100, "Minimum Damage", &config.aimbot.MinimumDMGSniper);
			}
			else if (tab_selected_weapon == tab_pistol) {
				Slider(100, "Hitchance", &config.aimbot.HitchancePistol);
				Slider(100, "Minimum Damage", &config.aimbot.MinimumDMGPistol);
			}
			else if (tab_selected_weapon == tab_autosniper) {
				Slider(100, "Hitchance", &config.aimbot.HitchanceAutosniper);
				Slider(100, "Minimum Damage", &config.aimbot.MinimumDMGAutosniper);
			}
			else if (tab_selected_weapon == tab_heavy) {
				Slider(100, "Hitchance", &config.aimbot.HitchanceHeavy);
				Slider(100, "Minimum Damage", &config.aimbot.MinimumDMGHeavy);
			}
			else if (tab_selected_weapon == tab_shotgun) {
				Slider(100, "Hitchance", &config.aimbot.HitchanceShotgun);
				Slider(100, "Minimum Damage", &config.aimbot.MinimumDMGShotgun);
			}
			else if (tab_selected_weapon == tab_smg) {
				Slider(100, "Hitchance", &config.aimbot.HitchanceSMG);
				Slider(100, "Minimum Damage", &config.aimbot.MinimumDMGSMG);
			}
			MultiComboBox("Hitboxes", { "Head", "Chest", "Pelvis/Arms", "Legs/Feet" }, config.aimbot.hitboxes);
			ComboBox("Piriority Hibox", { "Head", "Chest", "Pelvis/Arms", "Legs/Feet" }, &config.aimbot.priority_hitbox);
			CheckBox("Multipoint", &config.aimbot.MultiPoint);
			if (config.aimbot.MultiPoint) {
				Slider(100, "Head scale", &config.aimbot.HeadScale);
				Slider(100, "Body Scale", &config.aimbot.BodyScale);
			}
			CheckBox("Autostop", &config.aimbot.Autostop);

			group_box("Extras", group1, Pos.y + 323, gwidth, 96);

			CheckBox("Backtracking", &config.aimbot.PosBacktrack);
			CheckBox("Baim If Lethal", &config.aimbot.baim_lethal);
			CheckBox("Baim In Air", &config.aimbot.baim_air);
			CheckBox("Recoil Control", &config.aimbot.RCS);
			CheckBox("Resolver", &config.aimbot.Resolver);

			group_box("Anti-Aim", group2, Pos.y + 90, gwidth, 329); //66.138.202.220

			CheckBox("Active", &config.aimbot.Antiaim);
			second_tabs("Stand", "Move", 256, 108, 50, 10);
			if (tab_selected_v2 == subtab1)
			{
				ComboBox("Pitch", { "Off", "Emotion", "Up", "Zero", "No Spread" }, &config.aimbot.pitch_stand);
				ComboBox("Yaw", { "Off", "Backwards", "Auto-Direction", "Jitter Desync" }, & config.aimbot.yaw_stand);
				Slider(249, "Velocity Tolerance", &config.aimbot.stand_velocity_threshold);
				CheckBox("Slow Walk", &config.misc.slowwalk);
			}
			else if (tab_selected_v2 == subtab2)
			{
				ComboBox("Pitch", { "Off", "Emotion", "Up", "Zero", "No Spread" }, &config.aimbot.pitch_move);
				ComboBox("Yaw", { "Off", "Backwards", "Auto-Direction" }, & config.aimbot.yaw_move);
			}

			//CheckBox("Fakelag on Peek", &config.FakeLagOnPeek);
			//if (config.FakeLagOnPeek)
			//	CheckBox("Choke Shot", &config.ChokeShotOnPeek);
		}

		Tab("Visuals");
		{
			visuals_tabs("Main", "Chams", 37, 68, 46, 10);
			if (tab_selected_visuals == subtab5) {
				
				third_tabs("Main", "World", 259, 68, 45, 10);

				group_box("Extras", group2, Pos.y + 48, gwidth, 373); //66.168.202.220
				if (tab_selected_v3 == subtab3)
				{
					Slider(150, "Field of View", &config.visuals.Fov);
					static int value = 0;
					Listbox("Viewmodel", { "View X", "View Y", "View Z", "View -X", "View -Y" }, & config.visuals.Viewmodel);
					switch (config.visuals.Viewmodel) {
					case 0: 
						value = 10;
						Slider(value, "Viewmodel", &config.visuals.ViewmodelX); 
						break;
					case 1: 
						value = 10;
						Slider(value, "Viewmodel", &config.visuals.ViewmodelY);
						break;
					case 2: 
						value = 20;
						Slider(value, "Viewmodel", &config.visuals.ViewmodelZ);
						break;
					case 3: 
						value = 10;
						Slider(value, "Viewmodel", &config.visuals.nViewmodelX);
						break;
					case 4: 
						value = 10;
						Slider(value, "Viewmodel", &config.visuals.nViewmodelY);
						break;
					}
					Slider(360, "Viewmodel Roll", &config.visuals.roll);
					Slider(360, "Viewmodel Swing", &config.visuals.swing);
					Slider(360, "Viewmodel Tilt", &config.visuals.tilt);
					CheckBox("Penetration Dot", &config.visuals.pene_dot);
					CheckBox("Crosshair", &config.visuals.AutowallCross);
				}
				else if (tab_selected_v3 == subtab4)
				{
					Slider(100, "Nightmode", &config.visuals.nightmode);
					Slider(100, "Prop Alpha", &config.visuals.propalpha);
					Slider(100, "World Alpha", &config.visuals.worldalpha);
					Button("Update", &config.visuals.updateworld, 40);
				}
				group_box("Players", group1, Pos.y + 48, gwidth, 373);

				CheckBox("Active", &config.visuals.Esp);
				CheckBox("Draw Name", &config.visuals.Name);
				MultiComboBox("Draw Health", { "Text", "Bar" }, config.visuals.HealthVal);
				CheckBox("Draw Weapon", &config.visuals.Weapon);
				MultiComboBox("Draw Ammo", { "Bar", "Text" }, config.visuals.ammo);
				CheckBox("Draw Box", &config.visuals.Box);
				CheckBox("Draw Skeleton", &config.visuals.Skeleton);
				ColorPicker("Skeleton Color", config.visuals.SkeletonColor);
				CheckBox("Glow", &config.visuals.glow);
				ColorPicker("Glow Color", config.visuals.glow_color);
				if (config.visuals.glow)
					Slider(100, "Glow Alpha", &config.visuals.glow_alpha);

				CheckBox("Player Flags", &config.visuals.player_flags);
				if (config.visuals.player_flags) {
					MultiComboBox("Flags", { "Scoped", "Crouched", "Helmet-Kev" }, config.visuals.flags);
				}
				CheckBox("Player Arrows", &config.visuals.fovarrow);   
				ColorPicker("Player Arrows Color", config.visuals.fov_arrows_color);
				if (config.visuals.fovarrow) {
					Slider(100, "Size", &config.visuals.fov_arrows_size);
					Slider(100, "Diameter", &config.visuals.fov_arrows_dia);
				}

				CheckBox("Bullet Tracers", &config.misc.bullet_tracer);
				ColorPicker("Bullet Trace color", config.misc.bullet_tracecol);
				CheckBox("Bullet Impacts", &config.misc.bullet_impacts);
			}
			else if (tab_selected_visuals == subtab6) {
				group_box("Local", group2, gy, gwidth, gheight / 2); //66.168.202.220

				CheckBox("Local Chams", &config.visuals.local_chams);
				ColorPicker("LChams Color", config.visuals.localchamscolor);
				if (config.visuals.local_chams) {
					ComboBox("Local Material", { "Texture", "Flat", "Pulsing" }, &config.visuals.localchammat);
					Slider(100, "Opacity", &config.visuals.local_chams_opacity);
				}
				group_box("Other", group2, gy + gheight / 2 + 10, gwidth, gheight / 2.1 - 1); //66.168.202.220

				CheckBox("Fakelag Chams", &config.visuals.fakelagchams);
				ColorPicker("fLagchm Color", config.visuals.fakelagcolor);

				//CheckBox("Desync Chams", &config.visuals.desync_chams);
				//ColorPicker("desync Color", config.visuals.desynccolor);

				group_box("Main", group1, Pos.y + 48, gwidth, 373);

				CheckBox("Chams", &config.visuals.Chams);
				ColorPicker("Chams Color", config.visuals.ChamsColor);
				if (config.visuals.Chams) {
					CheckBox("Chams XQZ", &config.visuals.ChamsXQZ);
					ColorPicker("XQZ Color", config.visuals.ChamsColorXQZ);
					ComboBox("Material", { "Texture", "Flat" }, & config.visuals.chammat);
					Slider(100, "Opacity", &config.visuals.chams_opacity);
				}
			}
		}

		Tab("Misc");
		{
			group_box("Config", group2, gy, gwidth, gheight / 2 );

			ComboBox("Configs", { "Legit", "Rage", "Custom" }, &config.misc.ConfigList);
			Button("Save", &config.misc.ConfigSave, 43);
			Button("Load", &config.misc.ConfigLoad, 43);
			Button("Reset", &config.misc.ResetCFG, 42);
			Button("Open Dir", &config.misc.ConfigOpen, 35);
			group_box("Removals", group2, gy + gheight / 2 + 10, gwidth, gheight / 2.1 - 1);

			CheckBox("No Zoom", &config.misc.NoZoom);
			CheckBox("No Scope", &config.misc.NoScope);
			CheckBox("No Recoil", &config.misc.NoRecoil);
			CheckBox("No Smoke", &config.misc.nosmoke); 
			//CheckBox("No Flash", &config.misc.noflash);
			if (config.misc.nosmoke) {
				ComboBox("Type", { "Wireframe", "No Draw" }, &config.misc.nosmokeset);
			}
			if (g_Config == nullptr) return;

			if (config.misc.ConfigSave) {
				g_Config->Save();
				config.misc.ConfigSave = false;
			}
			if (config.misc.ConfigLoad) {
				g_Config->Load();
				config.misc.ConfigLoad = false;
			}
			if (g_menu.config.misc.ResetCFG) {
				g_Config->Setup();
				g_menu.config.misc.ResetCFG = false;
			}
			if (g_menu.config.misc.ConfigOpen) {
				OpenCFG();
				g_menu.config.misc.ConfigOpen = false;
			}
			

			group_box("Main", group1, gy, gwidth, gheight);

			CheckBox("Bunnyhop", &config.misc.Bhop);
			CheckBox("Autostrafe", &config.misc.AutoStrafe);
			CheckBox("Killsay", &config.misc.killsay);
			CheckBox("Hitmarker", &config.misc.hitmarker);
			CheckBox("Spoof Clantag", &config.misc.clantag);
			if (config.misc.hitmarker)
				ComboBox("Sounds", { "Off", "Paintcan", "Arena" }, & config.misc.Hitmarkers);

			CheckBox("Thirdperson (Mid Mouse)", &config.misc.Thirdperson);
		//	KeyBindButton(&config.misc.tp_key, 10);
			if (config.misc.Thirdperson) 
				Slider(200, "Distance", &config.misc.TpDIST);

			Slider(14, "Fakelag", &config.misc.Fakelag);
			CheckBox("Fakeduck (Alt)", &config.misc.fakeduck);

			CheckBox("Radar", &config.misc.Radar);
			if (config.misc.Radar) {
				Slider(2000, "Scale (Units)", &config.misc.RadarScale);
			}
			CheckBox("Spectators List", &config.misc.spectators);

			CheckBox("Eventlogs", &config.misc.EventLogs);
			if (config.misc.EventLogs) {
				MultiComboBox("To Log", { "Damage", "Purchase" }, config.misc.EventLog);
			}
			CheckBox("Bomb/Defuse Timer", &config.misc.bomb_timer);
			Slider(200, "Aspect Ratio", &config.misc.aspect_ratio);
			CheckBox("Watermark", &config.misc.watermark);
		}
		Tab("Skins");
		{
			text("Coming Soon.", 23, 110);
			//run_game();
		}

		TabSize = TabOffset;
		SubTabSize = SubTabOffset;
	}
	else
	{
		MenuAlpha_Main = max(MenuAlpha_Main - 25, 0);
		MenuAlpha_Text = max(MenuAlpha_Text - 35, 0);
	}
}
void Menu::spectators_list() {

	if (g_pEngine->IsConnected()) {
		int pos_x = g_menu.config.misc.spec_x;
		int pos_y = g_menu.config.misc.spec_y;
		static int _drag_x = 300;
		static int _drag_y = 300;

		static bool _dragging = false;
		bool _click = false;

		if (GetAsyncKeyState(VK_LBUTTON))
			_click = true;

		Vector2D _mouse_pos = g_pSurface->GetMousePosition();

		if (_dragging && !_click)
			_dragging = false;

		if (_dragging&& _click)
		{
			pos_x = _mouse_pos.x - _drag_x;
			pos_y = _mouse_pos.y - _drag_y;
			g_menu.config.misc.spec_x = pos_x;
			g_menu.config.misc.spec_y = pos_y;
		}

		int spec_width = 160;
		int spec_height = 160;

		if ((g_pSurface->MouseInRegion(pos_x + 400, pos_y, spec_width, spec_height - 140))) {
			_dragging = true;
			_drag_x = _mouse_pos.x - pos_x;
			_drag_y = _mouse_pos.y - pos_y;
		}

		int wa, ha;
		g_pEngine->GetScreenSize(wa, ha);

		g_pSurface->FilledRect(pos_x + 400, pos_y, spec_width, 20, Color(32, 32, 32));

		int loop = 0;

		for (int i = 0; i <= 64; i++) {
			C_BaseEntity* e = (C_BaseEntity*)g_pEntityList->GetClientEntity(i);
			PlayerInfo_t pinfo;

			if (e&& e != g::local && !e->IsDormant()) {
				g_pEngine->GetPlayerInfo(i, &pinfo);
				uintptr_t obs = e->observer_target();
				if (!obs) continue;

				C_BaseEntity * spec = (C_BaseEntity*)g_pEntityList->GetClientEntityFromHandle(obs);
				if (spec == nullptr) continue;
				PlayerInfo_t spec_info;
				g_pEngine->GetPlayerInfo(i, &spec_info);
				char buf[255]; sprintf_s(buf, "%s", pinfo.szName);
				if (spec->EntIndex() == g::local->EntIndex()) {
					g_pSurface->FilledRect(pos_x + 400, (pos_y + (18 * loop)) + 20, 160, 20, Color(50, 50, 50));
					g_pSurface->DrawT(pos_x + 408, (pos_y + (18 * loop)) + 22, Color(180, 180, 180), g::menu_font, false, buf);

					loop++;
				}
			}
		}
		char loop_str[24];
		sprintf_s(loop_str, "Spectators - (%d)", loop);
		g_pSurface->DrawT(pos_x + 408, pos_y + 2, Color(180, 180, 180), g::menu_font, false, loop_str);

		g_pSurface->FilledRect(pos_x + 400, pos_y + 20, spec_width, 1, menurgba);
	}
}
void Menu::draw_radar()
{
	int pos_x = this->config.misc.RadarX;
	int pos_y = this->config.misc.RadarY;
	static int _drag_x = 300;
	static int _drag_y = 300;

	int radar_width = 160;
	int radar_height = 160;

	static bool _dragging = false;
	bool _click = false;
	static bool _resizing = false;


	if (GetAsyncKeyState(VK_LBUTTON))
		_click = true;

	Vector2D _mouse_pos = g_pSurface->GetMousePosition();

	if (_dragging && !_click)
		_dragging = false;

	if (_resizing && !_click)
		_resizing = false;

	if (_dragging&& _click) {
		pos_x = _mouse_pos.x - _drag_x;
		pos_y = _mouse_pos.y - _drag_y;
		this->config.misc.RadarY = pos_y;
		this->config.misc.RadarX = pos_x;
	}

	if ((g_pSurface->MouseInRegion(pos_x, pos_y, radar_width, radar_height - 140))) {
		_dragging = true;
		_drag_x = _mouse_pos.x - pos_x;
		_drag_y = _mouse_pos.y - pos_y;
	}
	int wa, ha;
	g_pEngine->GetScreenSize(wa, ha);

	if (pos_x <= 0)
		pos_x = 0;

	if (pos_x + radar_width >= wa)
		pos_x = wa - radar_width;

	if (pos_y + radar_height >= ha)
		pos_y = ha - radar_height;

	if (pos_y <= 0)
		pos_y = 0;

	g_pSurface->FilledRect(pos_x, pos_y, radar_width, radar_height, Color(32, 32, 32));

	g_pSurface->FilledRect(pos_x, pos_y + 20, radar_width, radar_height, Color(40, 40, 40));

	g_pSurface->Line(pos_x + (radar_width / 2), pos_y + 20, pos_x + (radar_width / 2), radar_height + pos_y + 20, Color(110, 110, 110, 130));
	g_pSurface->Line(pos_x, pos_y + (radar_height / 2) + 20, pos_x + radar_width, pos_y + (radar_height / 2) + 20, Color(110, 110, 110, 130));

	g_pSurface->FilledRect(pos_x, pos_y + 20, radar_width, 1, menurgba);

	g_pSurface->DrawT(pos_x + 10, pos_y + 2, Color(180, 180, 180), g::menu_font, false, "Radar");

	if (g::local && !nullptr && g::local->IsAlive()) {
		Vector interface_view = g_pEngine->get_view_angles();
		float flAngle(interface_view.y);
		int cntrx = pos_x + radar_width / 2;
		int cntry = pos_y + radar_height / 2;
		for (int i = 0; i < 64; i++) {
			C_BaseEntity* e = (C_BaseEntity*)g_pEntityList->GetClientEntity(i);
			if (!e || !e->GetHealth()) continue;
			if (g::local->GetTeam() == e->GetTeam()) continue;

			float deltax = e->GetAbsOrigin().x - g::local->GetAbsOrigin().x;
			float deltay = e->GetAbsOrigin().y - g::local->GetAbsOrigin().y;

			float yaw = (flAngle) * (M_PI / 180.0);
			float view_angle_cos = cos(yaw);
			float view_angle_sin = sin(yaw);

			float x = deltay * (-view_angle_cos) + deltax * view_angle_sin;
			float y = deltax * (-view_angle_cos) - deltay * view_angle_sin;

			float scl = this->config.misc.RadarScale;

			int scrnx = cntrx + int(x / scl * 98);
			int scrny = cntry + int(y / scl * 98);

			if (fabs(x) > scl || fabs(y) > scl) {
				if (y > x) {
					if (y > -x) {
						x = scl * x / y;
						y = scl;
					}
					else {
						y = -scl * y / x;
						x = -scl;
					}
				}
				else {
					if (y > -x) {
						y = scl * y / x;
						x = scl;
					}
					else {
						x = -scl * x / y;
						y = -scl;
					}
				}
			}
			Color colour;
			if (e->IsDormant())
				colour = Color(255, 100, 100, 255);
			else
				colour = Color(255, 255, 255, 255);

			if (abs(cntrx - scrnx) < 75 && abs(cntry - scrny) < 75) {
				g_pSurface->RoundedFilledRect(scrnx, scrny + 20, 5, 5, 2, colour);
			}
		}
		g_pSurface->RoundedFilledRect(cntrx - 2, cntry + 18.5, 5, 5, 2, menurgba);
	}
}
void Menu::Tab(std::string name)
{
	static int tab_selected = 0;
	int TabArea[4] = { Pos.x + 40 + (TabOffset * 96), Pos.y + 20, 95, 26 };
	auto text_size = g_pSurface->get_text_size(g::tab_font, name.c_str());

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(TabArea[0], TabArea[1], TabArea[2], TabArea[3]))
		TabNum = TabOffset;

	for (int i = 0; i < 95 * 3; i += 95)
		g_pSurface->FilledRect(Pos.x + 135 + i, Pos.y + 21, 1, 22, Color(102, 102, 102));
	//g_pSurface->FilledRect(Pos.x + 185, Pos.y + 21, 1, 22, Color(102, 102, 102));
	//g_pSurface->FilledRect(Pos.x + 235, Pos.y + 21, 1, 22, Color(102, 102, 102));
	//g_pSurface->OutlinedRect(TabArea[0], TabArea[1], TabArea[2], TabArea[3], Color(102, 102, 102));

	if (TabNum == TabOffset)
		//g_pSurface->FilledRect(TabArea[0] + 4, TabArea[1] + 35, TabArea[2] - 10, 1, menurgba);
		g_pSurface->DrawT(TabArea[0] + (TabArea[2] / 2) - (text_size.right / 2), TabArea[1] + (TabArea[3] / 2) - (text_size.bottom / 2), Color(220, 220, 220), g::tab_font, false, name.c_str());
	else
		g_pSurface->DrawT(TabArea[0] + (TabArea[2] / 2) - (text_size.right / 2), TabArea[1] + (TabArea[3] / 2) - (text_size.bottom / 2), Color(142, 142, 142), g::tab_font, false, name.c_str());


	TabOffset += 1;
	PreviousControl = -1;
}
void Menu::text(std::string name, int x, int y) {
	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	g_pSurface->DrawT(Pos.x + x, Pos.y + y, Color(180, 180, 180), g::tab_font, false, name.c_str());

}
void Menu::group_box(std::string name, int x, int y, int w, int h) {
	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	y += 80;

	g_pSurface->OutlinedRect(x, y, w, h, Color(56, 56, 56));
	g_pSurface->OutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(25, 25, 25));

	RECT text_size = g_pSurface->get_text_size(g::menu_font, name.c_str());
	g_pSurface->FilledRect((((x)+(w / 2)) - (text_size.right / 2)) - 3, y - 1, text_size.right + 6, 2, Color(43, 43, 43));

	g_pSurface->FilledRect(x + 1, y + 1, w - 2, h - 2, Color(41, 41, 41));

	g_pSurface->DrawT((((x)+(w / 2)) - (text_size.right / 2)), (y - 8), Color(180, 180, 180), g::menu_font, false, name.c_str());

	OffsetY = y + 12;
	ControlsX = x + 8;
	GroupTabBottom = (y + h);
	PreviousControl = -1;
}
void Menu::second_tabs(std::string str, std::string str_v2, int x, int y, int gap, int additive) {	
	if (TabOffset - 1 != TabNum || TabOffset == 0) return;
	static int _height = 340;
	int in_sizew = _height - 32;
	static const int tab_amount_v2 = 2;
	static std::string tab_names_v2[tab_amount_v2] = { str, str_v2 };

	g_pSurface->FilledRect(Pos.x + x - 17, Pos.y + y + 19, 211, 30, Color(37, 37, 37));
	g_pSurface->OutlinedRect(Pos.x + x - 17, Pos.y + y + 19, 211, 30, Color(0, 0, 0));

	g_pSurface->FilledRect(Pos.x + x + 90, Pos.y + y + 24, 1, 20, Color(102, 102, 102));

	for (int i = 0; i < tab_amount_v2; i++) {
		RECT text_size = g_pSurface->get_text_size(g::subtab_font, tab_names_v2[i].c_str());
		int tab_area[4] = { ((Pos.x + i * ((in_sizew / tab_amount_v2) - gap))) + additive + x, (Pos.y + y) + 22, 60, 20 };
		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(tab_area[0], tab_area[1], tab_area[2], tab_area[3]))
			tab_selected_v2 = i;

		//g_pSurface->FilledRect(Pos.x + x, Pos.y + y, (475 - 60) / 2, 40, Color(53, 53, 53));

		if (tab_selected_v2 == i)
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2), tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2) + 2, menurgba, g::subtab_font, false, tab_names_v2[i].c_str());
		else
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2), tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2) + 2, Color(142, 142, 142), g::subtab_font, false, tab_names_v2[i].c_str());
	}
}
#define control_color Color(45, 45, 45), Color(45, 45, 45)
void Menu::third_tabs(std::string str, std::string str_v2, int x, int y, int gap, int additive) {
	if (TabOffset - 1 != TabNum || TabOffset == 0) return;
	static int _height = 340;
	int in_sizew = _height - 32;
	static const int tab_amount_v2 = 2;
	static std::string tab_names_v2[tab_amount_v2] = { str, str_v2 };

	g_pSurface->FilledRect(Pos.x + x - 17, Pos.y + y + 19, 211, 30, Color(37, 37, 37));
	g_pSurface->OutlinedRect(Pos.x + x - 17, Pos.y + y + 19, 211, 30, Color(0, 0, 0));
	g_pSurface->FilledRect(Pos.x + x + 90, Pos.y + y + 24, 1, 20, Color(102, 102, 102));

	for (int i = 0; i < tab_amount_v2; i++) {
		RECT text_size = g_pSurface->get_text_size(g::subtab_font, tab_names_v2[i].c_str());
		int tab_area[4] = { ((Pos.x + i * ((in_sizew / tab_amount_v2) - gap))) + additive + x, (Pos.y + y) + 22, 60, 20 };
		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(tab_area[0], tab_area[1], tab_area[2], tab_area[3]))
			tab_selected_v3 = i;

		if (tab_selected_v3 == i)
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2) - 4, tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2) + 2, menurgba, g::subtab_font, false, tab_names_v2[i].c_str());
		else
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2) - 4, tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2) + 2, Color(142, 142, 142), g::subtab_font, false, tab_names_v2[i].c_str());
	}
}
void Menu::visuals_tabs(std::string str, std::string str_v2, int x, int y, int gap, int additive) {
	if (TabOffset - 1 != TabNum || TabOffset == 0) return;
	static int _height = 340;
	int in_sizew = _height - 32;
	static const int tab_amount_v2 = 2;
	static std::string tab_names_v2[tab_amount_v2] = { str, str_v2 };
	g_pSurface->FilledRect(Pos.x + x - 17, Pos.y + y + 19, 211, 30, Color(37, 37, 37));
	g_pSurface->OutlinedRect(Pos.x + x - 17, Pos.y + y + 19, 211, 30, Color(0, 0, 0));
	g_pSurface->FilledRect(Pos.x + x + 90, Pos.y + y + 24, 1, 20, Color(102, 102, 102));
	for (int i = 0; i < tab_amount_v2; i++) {
		RECT text_size = g_pSurface->get_text_size(g::subtab_font, tab_names_v2[i].c_str());
		int tab_area[4] = { ((Pos.x + i * ((in_sizew / tab_amount_v2) - gap))) + additive + x, (Pos.y + y) + 22, 60, 20 };
		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(tab_area[0], tab_area[1], tab_area[2], tab_area[3]))
			tab_selected_visuals = i;

		if (tab_selected_visuals == i)
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2) - 5, tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2) + 2, menurgba, g::subtab_font, false, tab_names_v2[i].c_str());
		else
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2) - 5, tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2) + 2, Color(142, 142, 142), g::subtab_font, false, tab_names_v2[i].c_str());
	}
}
void Menu::weapon_tabs() {
	if (TabOffset - 1 != TabNum || TabOffset == 0) return;
	static int _height = 340;
	int in_sizew = _height - 32;
	static const int tab_amount_v2 = 7;
	//									"Rifle", "Sniper", "Pistol", "Auto-Sniper", "Heavy", "Shotgun", "SMG"
	//                                  "W", "Z", "G", "Y", "f", "b", "M"
	static std::string tab_names_v2[tab_amount_v2] = { "B", "R", " C", "I", " Z", "]", "Q" };

	g_pSurface->FilledRect(Pos.x + 20, Pos.y + 86, 430, 35, Color(37, 37, 37));
	g_pSurface->OutlinedRect(Pos.x + 20, Pos.y + 86, 430, 35, Color(0, 0, 0));
	for (int i = 0; i < tab_amount_v2; i++)
	{
		RECT text_size = g_pSurface->get_text_size(g::subtab_font, tab_names_v2[i].c_str());

		int tab_area[4] = { Pos.x + 20 + i * 60, Pos.y + 97, 70, 15 };

		//g_pSurface->OutlinedRect(tab_area[0], tab_area[1], tab_area[2] - 10, tab_area[3], Color(0, 0, 0));

		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(tab_area[0], tab_area[1], tab_area[2] - 10, tab_area[3]))
			tab_selected_weapon = i;

		if (tab_selected_weapon == i)
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2) - 12, tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2) - 6, menurgba, g::icon_font, false, tab_names_v2[i].c_str());
		else
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2) - 12, tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2) - 6, Color(145, 145, 145), g::icon_font, false, tab_names_v2[i].c_str());
	}
}
void Menu::legit_weapon_tabs() {
	if (TabOffset - 1 != TabNum || TabOffset == 0) return;
	static int _height = 340;
	int in_sizew = _height - 32;
	static const int tab_amount_v2 = 6;
	static std::string tab_names_v2[tab_amount_v2] = { "Rifle", "Sniper", "Pistol", "Heavy", "Shotgun", "SMG" };

	for (int i = 0; i < tab_amount_v2; i++)
	{
		RECT text_size = g_pSurface->get_text_size(g::subtab_font, tab_names_v2[i].c_str());

		int tab_area[4] = { Pos.x - 3, (Pos.y + i * 15) + 210, 100, 15 };

		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(tab_area[0], tab_area[1], tab_area[2] - 10, tab_area[3]))
			tab_selected_lweapon = i;

		if (tab_selected_lweapon == i)
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2), tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2), menurgba, g::subtab_font, false, tab_names_v2[i].c_str());
		else
			g_pSurface->DrawT(tab_area[0] + (tab_area[2] / 2) - (text_size.right / 2), tab_area[1] + (tab_area[3] / 2) - (text_size.bottom / 2), Color(145, 145, 145), g::subtab_font, false, tab_names_v2[i].c_str());
	}
}
void Menu::SubTab(std::string name)
{

	static int tab_selected = 0;
	int TabArea[4] = { Pos.x + 30 + (SubTabOffset * 40), Pos.y + 10, 80, 26 };
	auto text_size = g_pSurface->get_text_size(g::tab_font, name.c_str());

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(TabArea[0], TabArea[1], TabArea[2], TabArea[3]))
		SubTabNum = SubTabOffset;

	if (SubTabNum == SubTabOffset)
		g_pSurface->DrawT(TabArea[0] + (TabArea[2] / 2) - (text_size.right / 2), TabArea[1] + (TabArea[3] / 2) - (text_size.bottom / 2), menurgba, g::tab_font, false, name.c_str());
	else
		g_pSurface->DrawT(TabArea[0] + (TabArea[2] / 2) - (text_size.right / 2), TabArea[1] + (TabArea[3] / 2) - (text_size.bottom / 2), Color(180, 180, 180), g::tab_font, false, name.c_str());

	SubTabOffset += 1;
	PreviousControl = -1;
}

void Menu::CheckBox(std::string name, bool* item)
{
	if (GroupTabBottom <= OffsetY + 13)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	static bool pressed = false;

	if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 4, OffsetY, 12, 12))
	{
		if (pressed)
			* item = !*item;
		pressed = false;
	}

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 4, OffsetY, 12, 12) && !pressed)
		pressed = true;

	if (*item)
		g_pSurface->FilledRect(ControlsX + 6, OffsetY, 10, 10, menurgba);
	else
		g_pSurface->draw_gradient_rect(ControlsX + 6, OffsetY, 10, 10, control_color, gradient_type_t::horizontal);


	g_pSurface->OutlinedRect(ControlsX + 6, OffsetY, 10, 10, Color(25, 25, 25));
	g_pSurface->DrawT(ControlsX + 22, OffsetY - 2, Color(180, 180, 180), g::menu_font, false, name.c_str());

	OldOffsetY = OffsetY;
	OffsetY += 16;
	PreviousControl = check_box;
}

void Menu::Listbox(std::string name, std::vector< std::string > itemname, int* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 16, 100, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 16, 100, 16))
		clickRest = false;

	if (pressed) {
		if (!rest)
			selectedOpened = !selectedOpened;
		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;

	g_pSurface->DrawT(ControlsX + 6, OffsetY, Color(180, 180, 180), g::menu_font, false, name.c_str());

	g_pSurface->draw_gradient_rect(ControlsX + 6, OffsetY + 16, 100, (itemname.size() * 16), control_color, gradient_type_t::horizontal);
	g_pSurface->OutlinedRect(ControlsX + 6, OffsetY + 16, 100, (itemname.size() * 16), Color(25, 25, 25));

	for (int i = 0; i < itemname.size(); i++)
	{
		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 10 + i * 16, 140, 16))
			* item = i;

		if (*item == i)
			g_pSurface->DrawT(ControlsX + 11, OffsetY + 16 + (i * 16), menurgba, g::menu_font, false, itemname.at(i).c_str());
		else
			g_pSurface->DrawT(ControlsX + 11, OffsetY + 16 + (i * 16), Color(180, 180, 180), g::menu_font, false, itemname.at(i).c_str());
	}

	OldOffsetY = OffsetY;

	OffsetY += 30 + (itemname.size() * 16);

	PreviousControl = combo_box;
}

void Menu::Button(std::string name, bool* item, int plus_this)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	static bool pressed = false;

	if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 4, OffsetY, 84, 18))
	{
		if (pressed)
			* item = !*item;
		pressed = false;
	}

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 4, OffsetY, 84, 18) && !pressed)
		pressed = true;

	if (*item)
		g_pSurface->FilledRect(ControlsX + 6, OffsetY, 100, 16, menurgba);
	else
		g_pSurface->draw_gradient_rect(ControlsX + 6, OffsetY, 100, 16, control_color, gradient_type_t::horizontal);

	g_pSurface->OutlinedRect(ControlsX + 6, OffsetY, 100, 16, Color(25, 25, 25));
	g_pSurface->DrawT(ControlsX + plus_this, OffsetY, Color(180, 180, 180), g::menu_font, false, name.c_str());

	OldOffsetY = OffsetY;
	OffsetY += 18;
	PreviousControl = button;
}

void Menu::Slider(int max, std::string name, int* item, int step)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	int sub = 0;
	//if (PreviousControl == slider) sub = 4; else sub = 0;

	float pixelValue = max / 96.f;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 15 - sub, 100, 8))
		* item = (g_pSurface->GetMousePosition().x - (ControlsX + 8)) * pixelValue;

	if (*item > max)
		* item = max;

	if (*item < 0)
		* item = 0;

	static bool pressed_subtract = false;
	static bool pressed_add = false;

	//subtract
	if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX, OffsetY + 14, 9, 9)) {
		if (pressed_subtract)
			* item -= step;
		pressed_subtract = false;
	}
	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX, OffsetY + 14, 9, 9) && !pressed_subtract) pressed_subtract = true;

	//addition
	if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 108, OffsetY + 14, 9, 9)) {
		if (pressed_add)
			* item += step;

		pressed_add = false;
	}
	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 108, OffsetY + 14, 9, 9) && !pressed_add) pressed_add = true;


	std::string additives;
	additives += name;
	additives += ": ";
	additives += std::to_string(*item);
	g_pSurface->DrawT(ControlsX + 6, OffsetY - 2, Color(180, 180, 180), g::menu_font, false, additives.c_str());

	g_pSurface->draw_gradient_rect(ControlsX + 6, OffsetY + 15 - sub, 100, 8, control_color, gradient_type_t::horizontal);
	g_pSurface->FilledRect(ControlsX + 6, OffsetY + 15 - sub, (*item / pixelValue), 8, menurgba);
	g_pSurface->draw_gradient_rect(ControlsX + 6 + (*item / pixelValue), OffsetY + 15 - sub, 4, 8, control_color, gradient_type_t::horizontal);
	g_pSurface->OutlinedRect(ControlsX + 6, OffsetY + 15 - sub, 100, 8, Color(25, 25, 25));

	g_pSurface->DrawT(ControlsX, OffsetY + 11, Color(180, 180, 180), g::menu_font, false, "-");
	g_pSurface->DrawT(ControlsX + 110, OffsetY + 11, Color(180, 180, 180), g::menu_font, false, "+");

	OldOffsetY = OffsetY;
	OffsetY += 31 - sub;
	PreviousControl = slider;
}

void Menu::ComboBox(std::string name, std::vector< std::string > itemname, int* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 16, 100, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 16, 100, 16))
		clickRest = false;

	if (pressed) {
		if (!rest)
			selectedOpened = !selectedOpened;
		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;

	g_pSurface->DrawT(ControlsX + 7, OffsetY, Color(180, 180, 180), g::menu_font, false, name.c_str());
	g_pSurface->draw_gradient_rect(ControlsX + 6, OffsetY + 16, 100, 16, control_color, gradient_type_t::horizontal);
	g_pSurface->OutlinedRect(ControlsX + 6, OffsetY + 16, 100, 16, Color(25, 25, 25));

	if (open)
	{
		g_pSurface->draw_gradient_rect(ControlsX + 6, OffsetY + 40, 100, (itemname.size() * 16), control_color, gradient_type_t::horizontal);
		g_pSurface->OutlinedRect(ControlsX + 6, OffsetY + 40, 100, (itemname.size() * 16), Color(25, 25, 25));

		for (int i = 0; i < itemname.size(); i++)
		{
			if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 35 + i * 16, 100, 16))
				* item = i;

			//g_pSurface->OutlinedRect(ControlsX + 6, OffsetY + 35 + i * 16, 100, 16, Color(25, 25, 25));

			if (*item == i)
				g_pSurface->DrawT(ControlsX + 11, OffsetY + 40 + (i * 16), menurgba, g::menu_font, false, itemname.at(i).c_str());
			else
				g_pSurface->DrawT(ControlsX + 11, OffsetY + 40 + (i * 16), Color(180, 180, 180), g::menu_font, false, itemname.at(i).c_str());
		}

		/*TRIANGLE*/
		g_pSurface->OutlinedRect(ControlsX + 92, OffsetY + 24, 5, 1, Color(85, 85, 85));
		g_pSurface->OutlinedRect(ControlsX + 93, OffsetY + 23, 3, 1, Color(85, 85, 85));
		g_pSurface->OutlinedRect(ControlsX + 94, OffsetY + 22, 1, 1, Color(85, 85, 85));
	}
	else {
		/*TRIANGLE*/
		g_pSurface->OutlinedRect(ControlsX + 92, OffsetY + 22, 5, 1, Color(85, 85, 85));
		g_pSurface->OutlinedRect(ControlsX + 93, OffsetY + 23, 3, 1, Color(85, 85, 85));
		g_pSurface->OutlinedRect(ControlsX + 94, OffsetY + 24, 1, 1, Color(85, 85, 85));
	}

	g_pSurface->DrawT(ControlsX + 11, OffsetY + 16, Color(180, 180, 180), g::menu_font, false, itemname.at(*item).c_str());

	OldOffsetY = OffsetY;

	if (open)
		OffsetY += 48 + (itemname.size() * 16);
	else
		OffsetY += 38;

	PreviousControl = combo_box;
}

void Menu::MultiComboBox(std::string name, std::vector< std::string > itemname, bool* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	static bool multiPressed = false;
	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;
	std::string itemsSelected = "";
	int lastItem = 0;

	OffsetY -= 3;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX, OffsetY + 15, 100, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX, OffsetY + 15, 100, 16))
		clickRest = false;

	if (pressed)
	{
		if (!rest)
			selectedOpened = !selectedOpened;

		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;

	g_pSurface->DrawT(ControlsX + 7, OffsetY, Color(180, 180, 180), g::menu_font, false, name.c_str());
	g_pSurface->draw_gradient_rect(ControlsX + 6, OffsetY + 15, 100, 16, control_color, gradient_type_t::horizontal);
	g_pSurface->OutlinedRect(ControlsX + 6, OffsetY + 15, 100, 16, Color(25, 25, 25));

	if (open)
	{

		for (int i = 0; i < itemname.size(); i++)
		{
			if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX, OffsetY + 37 + (i * 16), 100, 16))
			{
				if (multiPressed)
					item[i] = !item[i];
				multiPressed = false;
			}

			if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX, OffsetY + 37 + (i * 16), 100, 16) && !multiPressed)
				multiPressed = true;

			g_pSurface->draw_gradient_rect(ControlsX + 6, OffsetY + 37 + (i * 16), 98, 16, control_color, gradient_type_t::horizontal);

			if (item[i])
				g_pSurface->DrawT(ControlsX + 10, OffsetY + 37 + (i * 16), menurgba, g::menu_font, false, itemname.at(i).c_str());
			else
				g_pSurface->DrawT(ControlsX + 10, OffsetY + 37 + (i * 16), Color(180, 180, 180), g::menu_font, false, itemname.at(i).c_str());

		}
		g_pSurface->OutlinedRect(ControlsX + 6, OffsetY + 37, 99, (itemname.size() * 16), Color(25, 25, 25));

		/*TRIANGLE*/
		g_pSurface->OutlinedRect(ControlsX + 92, OffsetY + 24, 5, 1, Color(85, 85, 85));
		g_pSurface->OutlinedRect(ControlsX + 93, OffsetY + 23, 3, 1, Color(85, 85, 85));
		g_pSurface->OutlinedRect(ControlsX + 94, OffsetY + 22, 1, 1, Color(85, 85, 85));
	}
	else {
		/*TRIANGLE*/
		g_pSurface->OutlinedRect(ControlsX + 92, OffsetY + 22, 5, 1, Color(85, 85, 85));
		g_pSurface->OutlinedRect(ControlsX + 93, OffsetY + 23, 3, 1, Color(85, 85, 85));
		g_pSurface->OutlinedRect(ControlsX + 94, OffsetY + 24, 1, 1, Color(85, 85, 85));
	}

	bool items = false;

	// man look at all these for loops i sure am retarded

	for (int i = 0; i < itemname.size(); i++)
	{
		if (item[i])
		{
			if (lastItem < i)
				lastItem = i;
		}
	}

	for (int i = 0; i < itemname.size(); i++)
	{
		if (item[i])
		{
			items = true;
			RECT TextSize = g_pSurface->GetTextSizeRect(g::menu_font, itemsSelected.c_str());
			RECT TextSizeGonaAdd = g_pSurface->GetTextSizeRect(g::menu_font, itemname.at(i).c_str());
			if (TextSize.right + TextSizeGonaAdd.right < 98)
				itemsSelected += itemname.at(i) + ((lastItem == i) ? "" : ", ");
		}
	}

	if (!items)
		itemsSelected = "Off";

	g_pSurface->DrawT(ControlsX + 11, OffsetY + 16, Color(180, 180, 180), g::menu_font, false, itemsSelected.c_str());

	OldOffsetY = OffsetY;

	if (open)
		OffsetY += 42 + (itemname.size() * 16);
	else
		OffsetY += 38;

	PreviousControl = multi_box;
}

void Menu::ColorPicker(std::string name, ColorV2 & item, int y) // best coder in the universe
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	if (PreviousControl == slider || PreviousControl == -1)
		return;

	int CtrXoffset = 132;

	int yoffset = OffsetY + 10;
	int xoffset = ControlsX + 330;

	Color rainbow;

	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string name_selected;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 120, OffsetY - 15 + y, 16, 10) && !clickRest) //checking if pressed preivew box
	{
		name_selected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 120, OffsetY - 15 + y, 16, 10)) //checking if pressed preivew box
		clickRest = false;

	if (pressed)
	{
		if (!rest)
			selectedOpened = !selectedOpened;

		rest = true;
	}
	else
		rest = false;

	if (name_selected == name)
		open = selectedOpened;

	if (open)
	{
		g_pSurface->OutlinedRect(xoffset - 174, yoffset - 14 + y, 108, 128, Color(0, 0, 0)); //outline around color box
		g_pSurface->FilledRect(xoffset - 173, yoffset - 13 + y, 106, 126, Color(90, 90, 90)); //background


		g_pSurface->FilledRect(xoffset - 170, OffsetY + y, 100, 20, Color(0, 0, 0, 255)); //black at the top
		g_pSurface->FilledRect(xoffset - 170, OffsetY + 100 + y, 100, 20, Color(255, 255, 255, 255)); //white at the bottom

		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(xoffset - 170, OffsetY + y, 100, 10)) //this is for checking if its in black
		{
			item.red = 0;
			item.green = 0;
			item.blue = 0;
			item.alpha = 255;
		}

		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(xoffset - 170, OffsetY + 110 + y, 100, 10)) //checking if its in the white bar
		{
			item.red = 255;
			item.green = 255;
			item.blue = 255;
			item.alpha = 255;
		}

		for (int i = 0; i < 100; i++)
		{
			if (xoffset >= ControlsX + 430)
			{
				xoffset -= 100; //puts the colors on different lines and shid
				yoffset += 10;
			}

			float hue = (i * .01f);

			rainbow.FromHSV(hue, 1.f, 1.f); //fuck if i know, changes color (dynam)

			rainbow.alpha = 255;
			g_pSurface->FilledRect(xoffset - 170, yoffset + y, 10, 10, rainbow);

			if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(xoffset - 170, yoffset + y, 10, 10)) //checks if color is being changed
			{
				item.red = rainbow.red;
				item.green = rainbow.green;
				item.blue = rainbow.blue;
				item.alpha = 255.f;
			}

			xoffset += 10;
		}
	}

	rainbow.red = item.red; //just restores old values
	rainbow.green = item.green; //g
	rainbow.blue = item.blue; //b
	rainbow.alpha = 255;

	g_pSurface->FilledRect(ControlsX + 120, OffsetY - 15 + y, 16, 10, rainbow); //the preivew box
	g_pSurface->OutlinedRect(ControlsX + 120, OffsetY - 15 + y, 16, 10, Color(0, 0, 0)); //the preivew box

	//char buffer[24];
	//sprintf_s(buffer, "R: %.f G: %.f B: %.f", item.red, item.green, item.blue);
	//if (open)
	//	render::get().draw_text(xoffset - 240, yoffset + 25, render::get().menu_font, buffer, color(180, 180, 180));
}