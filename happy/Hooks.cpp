#include <thread>
#include "Hooks.h"
#include "Utils\Utils.h"
#include "Features\Features.h"
#include "Menu\Menu.h"
#include "SDK\Hitboxes.h"
#include "libloaderapi.h"
#include "Features/Logs/Event Logs.h"
#include "Features/Visuals/chams.hpp"
#include "Features/event_man.hpp"
#include "Features/Aimbot/Legitbot.hpp"
#include "Features/Misc/skinchanger.hpp"
#include "Features/Misc/dots.hpp"
#include "SDK\xostr.hpp"
#include "Features/Aimbot/engine_prediction.h"
Misc     g_Misc;
Hooks    g_Hooks;
Event    g_Event;
store_matrix str_matrix;
skins skinchanger;
std::vector<trace_info> trace_logs;
std::string hitgroup_name(int hitgroup)
{
	switch (hitgroup) {
	case 1: return "head"; break;
	case 2: return "chest"; break;
	case 3: return "stomach"; break;
	case 4: return "left arm"; break;
	case 5: return "right arm"; break;
	case 6: return "left leg"; break;
	case 7: return "right leg"; break;
	default: return "body";
	}
}

DamageEventListener* DamageListener;
DamageEventListener* BuyListener;
DamageEventListener* bulletimpact;
matrix3x4_t local_last_bone_matrix[128];
Vector fake_angle, real_angle;
bool hit_event = false;
int hit_alpha = 100;
float hit_time = 0;

void SayInChat(const char* text)
{
	char buffer[250];
	sprintf_s(buffer, "say \"%s\"", text);
	g_pEngine->ClientCmd_Unrestricted(buffer);
}
PlayerInfo_t GetInfo(int Index)
{
	PlayerInfo_t Info;
	g_pEngine->GetPlayerInfo(Index, &Info);
	return Info;
}
void killsays(I_Game_Event* pEvent)
{
	int attackerid = pEvent->GetInt("attacker");
	int entityid = g_pEngine->GetPlayerForUserID(attackerid);

	if (entityid == g_pEngine->GetLocalPlayer()) {
		int nUserID = pEvent->GetInt("attacker");
		int nDead = pEvent->GetInt("userid");
		if (nUserID || nDead) {
			PlayerInfo_t killed_info = GetInfo(g_pEngine->GetPlayerForUserID(nDead));
			if (g_menu.config.misc.killsay)
				SayInChat("NN DOWN.");

			g::balance += 100;
		}
	}
}
std::string clean_weapon(std::string name) {
	std::string wep = name;
	if (wep[0] == 'w') wep.erase(wep.begin());
	auto end_of_wep = wep.find("eapon_");
	if (end_of_wep != std::string::npos)
		wep.erase(wep.begin() + end_of_wep, wep.begin() + end_of_wep + 6);
	return wep;
}
void purchase(I_Game_Event* _event) {
	int uid = _event->GetInt("userid");
	if (uid && g_menu.config.misc.EventLog[1] && g_menu.config.misc.EventLogs) {
		PlayerInfo_t killed_info = GetInfo(g_pEngine->GetPlayerForUserID(uid));
		std::string one = killed_info.szName;
		std::string two = (" bought: ");
		std::string three = clean_weapon(_event->GetString("weapon"));
		std::string six = "\n";
		std::string out = one + two + three + six;
		if (clean_weapon(_event->GetString("weapon")) != "unknown")
			events::log(out);
	}
}
void health(IGameEvent* _event)
{
	int attackerid = _event->GetInt("attacker");
	int entityid = g_pEngine->GetPlayerForUserID(attackerid);
	if (g_pEngine->IsConnected() && g_pEngine->IsInGame() && g_menu.config.misc.EventLog[0] && g_menu.config.misc.EventLogs)
	{
		if (entityid == g_pEngine->GetLocalPlayer())
		{
			int nUserID = _event->GetInt("attacker");
			int nDead = _event->GetInt("userid");
			if (nUserID || nDead) {
				PlayerInfo_t killed_info = GetInfo(g_pEngine->GetPlayerForUserID(nDead));
				std::string hit = ("Hurt ");
				std::string name = killed_info.szName;
				std::string forz = " for ";
				std::string dmghealth = _event->GetString("dmg_health");
				std::string inthe = " in the ";
				std::string hitbo = hitgroup_name(_event->GetInt("hitgroup"));
				std::string paren = " (";
				std::string health = _event->GetString("health");
				std::string hlthremain = " health remaining)";
				std::string newline = "\n";
				std::string out = hit + name + forz + dmghealth + inthe + hitbo + paren + health + hlthremain + newline;
				events::log(out);
			}
		}
	}
}
void bullet_impact(I_Game_Event* _event) {
	if (g_pEngine->IsConnected() && g_pEngine->IsInGame() && g_menu.config.misc.bullet_tracer)
	{
		C_BaseEntity* shooter = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(_event->GetInt("userid")));
		if (shooter == nullptr || shooter != g::local) return;

		trace_logs.push_back(trace_info(
				shooter->GetEyePosition(), 
				Vector(_event->GetFloat("x"), _event->GetFloat("y"), _event->GetFloat("z")), 
				g_pGlobalVars->curtime));
	}
}
void Hooks::Init()
{
    // Get window handle
    while (!(g_Hooks.hCSGOWindow = FindWindowA("Valve001", nullptr)))
    {
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(50ms);
    }

    interfaces::Init();                         // Get interfaces
    g_pNetvars = std::make_unique<NetvarTree>();// Get netvars after getting interfaces as we use them

    if (g_Hooks.hCSGOWindow)        // Hook WNDProc to capture mouse / keyboard input FUCK THIS SHIT ITS ALL CHINESE RAWRRRRRRRRR
        g_Hooks.pOriginalWNDProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_Hooks.WndProc)));

    // VMTHooks
	g_Hooks.pClientHook     = std::make_unique<VMTHook>(g_pClientDll);
    g_Hooks.pClientModeHook = std::make_unique<VMTHook>(g_pClientMode);
    g_Hooks.pSurfaceHook	= std::make_unique<VMTHook>(g_pSurface);
	g_Hooks.pPanelHook		= std::make_unique<VMTHook>(g_pPanel);
	g_Hooks.pModelHook = std::make_unique<VMTHook>(g_pModelRender);
	g_Hooks.pRenderViewHook = std::make_unique<VMTHook>(g_pRenderView);

    // Hook the table functions
	g_Hooks.pClientHook    ->Hook(vtable_indexes::frameStage, Hooks::FrameStageNotify);
    g_Hooks.pClientModeHook->Hook(vtable_indexes::createMove, Hooks::CreateMove);
	g_Hooks.pClientModeHook->Hook(vtable_indexes::view, Hooks::OverrideView);
    g_Hooks.pSurfaceHook   ->Hook(vtable_indexes::lockCursor, Hooks::LockCursor);
	g_Hooks.pModelHook     ->Hook(vtable_indexes::dme, Hooks::DrawModelExecute);
	g_Hooks.pPanelHook	   ->Hook(vtable_indexes::paint, Hooks::PaintTraverse);
	g_Hooks.pRenderViewHook->Hook(vtable_indexes::sceneEnd, Hooks::SceneEnd);

	g_Event.Init();


	DamageListener = new DamageEventListener(killsays);
	g_pEventManager2->AddListener(DamageListener, "player_death", false);

	BuyListener = new DamageEventListener(purchase);
	g_pEventManager2->AddListener(BuyListener, "item_purchase", false);

	bulletimpact = new DamageEventListener(bullet_impact);
	g_pEventManager2->AddListener(bulletimpact, "bullet_impact", false);

	g::CourierNew = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::CourierNew, "Courier New", 14, 300, 0, 0, FONTFLAG_ANTIALIAS);

	g::Verdana = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::Verdana, "VerdanaPro-CondLight", 10, 300, 0, 0, FONTFLAG_DROPSHADOW);

	g::Tahoma = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::Tahoma, "Tahoma", 12, 400, 0, 0, FONTFLAG_DROPSHADOW);

	g::menu_font = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::menu_font, "Roboto", 14, 400, 0, 0, FONTFLAG_ANTIALIAS);

	g::menu_font2 = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::menu_font2, "Roboto", 14, 400, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);

	g::tab_font = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::tab_font, "AvenirLTStd-Roman", 20, 500, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);

	g::esp_font = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::esp_font, "Small Fonts", 10, 400, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);

	g::esp_font2 = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::esp_font2, "Small Fonts", 8, 400, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);

	g::title_font = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::title_font, "Roboto", 30, 400, 0, 0, FONTFLAG_ANTIALIAS);

	g::money_font = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::money_font, "Roboto", 60, 400, 0, 0, FONTFLAG_ANTIALIAS);

	g::subtab_font = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::subtab_font, "AvenirLTStd-Roman", 17, 400, 0, 0, FONTFLAG_ANTIALIAS);

	g::misc_font = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::misc_font, "Roboto", 15.5, 400, 0, 0, FONTFLAG_ANTIALIAS);

	g::icon_font = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::icon_font, "Counter-Strike", 33, 400, 0, 0, FONTFLAG_ANTIALIAS);

	g_pCvar->FindVar("viewmodel_fov")->callbacks.SetSize(0);
	g_pCvar->FindVar("viewmodel_offset_x")->callbacks.SetSize(0);
	g_pCvar->FindVar("viewmodel_offset_z")->callbacks.SetSize(0);

    Utils::Log("Hooking completed!");
}

void Hooks::Restore()
{
	Utils::Log("Unhooking in progress...");
	{
		g_Hooks.pClientHook->Unhook(vtable_indexes::frameStage);
		g_Hooks.pClientModeHook->Unhook(vtable_indexes::createMove);
		g_Hooks.pClientModeHook->Unhook(vtable_indexes::view);
		g_Hooks.pSurfaceHook->Unhook(vtable_indexes::lockCursor);
		g_Hooks.pPanelHook->Unhook(vtable_indexes::paint);
		g_Hooks.pModelHook->Unhook(vtable_indexes::dme);
		g_Hooks.pRenderViewHook->Unhook(vtable_indexes::sceneEnd);

		SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_Hooks.pOriginalWNDProc));

		g_pNetvars.reset();   /* Need to reset by-hand, global pointer so doesnt go out-of-scope */
	}
	Utils::Log("Unhooking succeded!");
}
DWORD WaitOnModuleHandle(std::string moduleName) {
	DWORD ModuleHandle = NULL;
	while (!ModuleHandle) {
		ModuleHandle = (DWORD)GetModuleHandle(moduleName.c_str());
		if (!ModuleHandle)
			Sleep(50);
	}
	return ModuleHandle;
}
bool bCompare(const BYTE * Data, const BYTE * Mask, const char* szMask) {
	for (; *szMask; ++szMask, ++Mask, ++Data) {
		if (*szMask == 'x' && *Mask != *Data)
			return false;
	}
	return (*szMask) == 0;
}
DWORD pattern_scan(std::string moduleName, BYTE * Mask, const char* szMask) {
	DWORD Address = WaitOnModuleHandle(moduleName.c_str());
	MODULEINFO ModInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)Address, &ModInfo, sizeof(MODULEINFO));
	DWORD Length = ModInfo.SizeOfImage;
	for (DWORD c = 0; c < Length; c += 1) {
		if (bCompare((BYTE*)(Address + c), Mask, szMask))
			return (DWORD)(Address + c);
	}
	return 0;
}
void clan_tag(const char* tag) {
	static auto set_clan_tag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)(pattern_scan("engine.dll", (PBYTE)"\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15\x00\x00\x00\x00\x6A\x24\x8B\xC8\x8B\x30", "xxxxxxxxx????xxxxxx"))));
	if (g_menu.config.misc.clantag)
		set_clan_tag(tag, tag);
	else
		set_clan_tag("", "");
}
void animated_tag() {
	static std::string cur_clantag = "happyhack";
	static float old_curtime;

	if (g_pEngine->IsInGame()) {
		if (g_pGlobalVars->curtime - old_curtime >= 0.2f) {
			cur_clantag += cur_clantag.at(0);
			cur_clantag.erase(0, 1);

			clan_tag(cur_clantag.c_str());
			old_curtime = g_pGlobalVars->curtime;
		}
	}
}
void rotate(std::string& urtext)
{
	std::string temp_string = urtext;
	urtext.erase(0, 1);
	urtext += temp_string[0];
}
void BulletImpacts()
{
	static ConVar* impacts = g_pCvar->FindVar("sv_showimpacts");
	static ConVar* impacttime = g_pCvar->FindVar("sv_showimpacts_time"); 

	if (g_menu.config.misc.bullet_impacts) {
		impacts->SetValue(1);
		impacttime->SetValue(1);
	}
	else impacts->SetValue(0);
}
template<class T, class U>
T fine(T in, U low, U high) {
	if (in <= low) return low;
	if (in >= high) return high;
	return in;
}
bool __fastcall Hooks::CreateMove(IClientMode* thisptr, void* edx, float sample_frametime, CUserCmd* cmd)
{
	// Call original createmove before we start screwing with it
	static auto oCreateMove = g_Hooks.pClientModeHook->GetOriginal<CreateMove_t>(vtable_indexes::createMove);
	oCreateMove(thisptr, edx, sample_frametime, cmd);

    if (!cmd || !cmd->command_number)
		return oCreateMove;

    // Get globals
    g::cmd         = cmd;
    g::local = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	g::bSendPacket  = true;
    if (!g::local)
		return oCreateMove;

	/*if (g_menu.config.visuals.Viewmodel > 3) {
		g_pCvar->FindVar("viewmodel_fov_offset_x")->SetValue(-(g_menu.config.visuals.nViewmodelX));
		g_pCvar->FindVar("viewmodel_fov")->SetValue(-g_menu.config.visuals.nViewmodelY);
		g_pCvar->FindVar("viewmodel_offset_z")->SetValue(-(g_menu.config.visuals.nViewmodelZ - 10));
	}
	else {
		g_pCvar->FindVar("viewmodel_fov_offset_x")->SetValue(g_menu.config.visuals.ViewmodelX);
		g_pCvar->FindVar("viewmodel_fov")->SetValue(g_menu.config.visuals.ViewmodelY);
		g_pCvar->FindVar("viewmodel_offset_z")->SetValue(g_menu.config.visuals.ViewmodelZ - 10);
	}*/

	g_pCvar->FindVar("crosshair")->SetValue(1);
	g_pCvar->FindVar("cl_foot_contact_shadows")->SetValue(0);
	ConVar* staticdrop = g_pCvar->FindVar("r_DrawSpecificStaticProp");
	staticdrop->SetValue(0);

	ConVar* crosshar = g_pCvar->FindVar("weapon_debug_spread_show");
	if (g_menu.config.visuals.AutowallCross)
	{
		crosshar->SetValue(3);
		if (g::local->IsScoped())
		{
			crosshar->SetValue(0);
		}
	}
	else {
		crosshar->SetValue(0);
	}
	BulletImpacts();
	if (g_menu.config.visuals.updateworld && g_pMaterialSys != nullptr) {
		for (MaterialHandle_t i = g_pMaterialSys->FirstMaterial(); i != g_pMaterialSys->InvalidMaterial(); i = g_pMaterialSys->NextMaterial(i)) {
			IMaterial* material = g_pMaterialSys->GetMaterial(i);

			if (!material || g_pMaterialSys == nullptr) continue;
			if (!material || material->IsErrorMaterial()) continue;
			if (strstr(material->GetTextureGroupName(), "World")) {
				material->ColorModulate(g_menu.config.visuals.nightmode * 0.01, g_menu.config.visuals.nightmode * 0.01, g_menu.config.visuals.nightmode * 0.01);
				material->AlphaModulate(g_menu.config.visuals.worldalpha * 0.01);
			}
			if (strstr(material->GetTextureGroupName(), "StaticProp")) {
				material->ColorModulate(g_menu.config.visuals.nightmode * 0.01, g_menu.config.visuals.nightmode * 0.01, g_menu.config.visuals.nightmode * 0.01);
				material->AlphaModulate(g_menu.config.visuals.propalpha * 0.01);
			}
		}
		g_menu.config.visuals.updateworld = false;
	}

	/*for (int i = 1; i < g_pEntityList->GetMaxEntities(); ++i)
	{
		auto entity = g_pEntityList->GetClientEntity(i);
		if (entity)
		{
			if (entity->GetClientClass()->ClassID == EClassIds::CEnvTonemapController)
			{
				entity->set_m_bUseCustomAutoExposureMin(1);
				entity->set_m_bUseCustomAutoExposureMax(1);
				entity->set_m_flCustomAutoExposureMin(g_menu.config.visuals.nightmode * 0.01f);
				entity->set_m_flCustomAutoExposureMax(g_menu.config.visuals.nightmode * 0.01f);
			}
		}
	}*/

	float ratio = (g_menu.config.misc.aspect_ratio * 0.1) / 2;
	if (ratio > 0.001)
		g_pCvar->FindVar("r_aspectratio")->SetValue(ratio);
	else
		g_pCvar->FindVar("r_aspectratio")->SetValue((35 * 0.1f) / 2);

	if (g_menu.config.skins.forceupdate) {
		g_pEngine->ClientCmd_Unrestricted("cl_fullupdate");
		g_menu.config.skins.forceupdate = false;
	}
	uintptr_t *framePtr;
	__asm mov framePtr, ebp;

	animated_tag();

	g::OriginalView = g::cmd->viewangles;

	if (g_menu.config.aimbot.RCS) {
		g::cmd->viewangles.x -= g::local->punchangle().x * 2.f;
		g::cmd->viewangles.y -= g::local->punchangle().y * 2.f;
	}

	for (int i = 0; i <= 64; ++i) {
		C_BaseEntity* e = g_pEntityList->GetClientEntity(i);

		if (!e || !e->IsAlive() || e->IsDormant() || e == g::local)
			continue;

		e->SetupBones(str_matrix.matrix[i], 128, 0x0100, 0.f);
		g::local->SetupBones(str_matrix.local_matrix, 128, 0x0100, 0.f);
		float* PosParams = (float*)((uintptr_t)e + 0x2774);
		g::pose_param_11[e->EntIndex()] = PosParams[11];
	}
	if (g_pEngine->IsConnected() && g_pEngine->IsInGame()) {
		g_Misc.OnCreateMove();

		g_Misc.start_move_fix();
		g_AntiAim.OnCreateMove();
		g_Misc.end_move_fix();

		if (g::local->IsAlive()) {
			engine_prediction::RunEnginePred();
			g_Aimbot.OnCreateMove();
			engine_prediction::EndEnginePred();
		}


		if (g_menu.config.aimbot.PosBacktrack)
			g_LagComp.handle();

	}

	//if (GetAsyncKeyState(VK_MENU))
	//	g::flip_bool = !g::flip_bool;

	g_Math.Clamp(g::cmd->viewangles);

	if (g::bSendPacket) {
		g::local->SetupBones(local_last_bone_matrix, 128, 0x0100, 0.f);
		g::FakeAngle = cmd->viewangles;
	}
	else {
		real_angle = g::cmd->viewangles;
		g::RealAngle = cmd->viewangles;
		g::stored_angles = g::cmd->viewangles;
	}
	*(bool*)(*framePtr - 0x1C) = g::bSendPacket;

	g::cmd->buttons |= IN_BULLRUSH; // hehe

    return false;
}
void angleMatrix(const Vector angles, matrix3x4_t& matrix) {
	float sr, sp, sy, cr, cp, cy;
	sy = sin(DEG2RAD(angles[1])); cy = cos(DEG2RAD(angles[1])); sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0])); sr = sin(DEG2RAD(angles[2])); cr = cos(DEG2RAD(angles[2]));
	matrix[0][0] = cp * cy; matrix[1][0] = cp * sy; matrix[2][0] = -sp;
	float crcy = cr * cy; float crsy = cr * sy; float srcy = sr * cy; float srsy = sr * sy;
	matrix[0][1] = sp * srcy - crsy; matrix[1][1] = sp * srsy + crcy;
	matrix[2][1] = sr * cp; matrix[0][2] = (sp * crcy + srsy); matrix[1][2] = (sp * crsy - srcy);
	matrix[2][2] = cr * cp; matrix[0][3] = 0.0f;  matrix[1][3] = 0.0f; matrix[2][3] = 0.0f;
}
void matrixCopy(const matrix3x4_t& source, matrix3x4_t& target) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++)
			target[i][j] = source[i][j];
	}
}
void matrixMultiply(matrix3x4_t& in1, const matrix3x4_t& in2) {
	matrix3x4_t out;
	if (&in1 == &out) {
		matrix3x4_t in1b;
		matrixCopy(in1, in1b);
		matrixMultiply(in1b, in2);
		return;
	}
	if (&in2 == &out) {
		matrix3x4_t in2b;
		matrixCopy(in2, in2b);
		matrixMultiply(in1, in2b);
		return;
	}
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
	in1 = out;
}
FORCEINLINE float DotProduct(const float* v1, const float* v2) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}
void vectorRotate(const float* in1, const matrix3x4_t& in2, float* out) {
	out[0] = DotProduct(in1, in2[0]);
	out[1] = DotProduct(in1, in2[1]);
	out[2] = DotProduct(in1, in2[2]);
}
void vectorRotate(const Vector& in1, const matrix3x4_t& in2, Vector& out) {
	vectorRotate(&in1.x, in2, &out.x);
}
void vectorRotate(const Vector& in1, const Vector& in2, Vector& out) {
	matrix3x4_t matRotate;
	angleMatrix(in2, matRotate);
	vectorRotate(in1, matRotate, out);
}
static bool scene_end = false;
void __fastcall Hooks::DrawModelExecute(void* ecx, void* edx, IMatRenderContext* context, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix)
{
	static auto oDrawModelExecute = g_Hooks.pModelHook->GetOriginal<DrawModelExecute_t>(vtable_indexes::dme);
	const char* ModelName = g_pModelInfo->GetModelName((model_t*)info.pModel);

	float color[3] = { 0.f };
	color[0] = g_menu.config.visuals.localchamscolor.red / 255.f;
	color[1] = g_menu.config.visuals.localchamscolor.green / 255.f;
	color[2] = g_menu.config.visuals.localchamscolor.blue / 255.f;

	float colorfl[3] = { 0.f };
	colorfl[0] = g_menu.config.visuals.fakelagcolor.red / 255.f;
	colorfl[1] = g_menu.config.visuals.fakelagcolor.green / 255.f;
	colorfl[2] = g_menu.config.visuals.fakelagcolor.blue / 255.f;


	float colorde[3] = { 0.f };
	colorde[0] = g_menu.config.visuals.desynccolor.red / 255.f;
	colorde[1] = g_menu.config.visuals.desynccolor.green / 255.f;
	colorde[2] = g_menu.config.visuals.desynccolor.blue / 255.f;

	static IMaterial* texture = chams::get().create_material(false, true, false);
	static IMaterial* flat = chams::get().create_material(false, false, false);
	static IMaterial* mat;
	static IMaterial* pulse = g_pMaterialSys->FindMaterial("models/inventory_items/dogtags/dogtags_outline", "Model textures");
	float old_color[3] = { 0.f };

	if (info.index == g_pEngine->GetLocalPlayer() && strstr(ModelName, "models/player")) {
		if (g::local->IsScoped())
			g_pRenderView->SetBlend(0.3f);

		g_pRenderView->GetColorModulation(old_color);
		if (g_menu.config.visuals.fakelagchams) {
			g_pModelRender->ForcedMaterialOverride(pulse);
			g_pRenderView->SetColorModulation(colorfl);
			oDrawModelExecute(ecx, context, state, info, local_last_bone_matrix);
			g_pRenderView->SetColorModulation(old_color);
			g_pModelRender->ForcedMaterialOverride(NULL);
		}

		if (g_menu.config.visuals.desync_chams) {
			Vector bonePos;
			Vector outPos;
			matrix3x4_t BoneMatrix[128];
			g_pRenderView->SetColorModulation(colorde);
			for (int i = 0; i < 128; i++)
			{
				angleMatrix(Vector(0, g::FakeAngle.y, 0), BoneMatrix[i]);
				matrixMultiply(BoneMatrix[i], matrix[i]);
				bonePos = Vector(matrix[i][0][3], matrix[i][1][3], matrix[i][2][3]) - info.origin;
				vectorRotate(bonePos, Vector(0, g::FakeAngle.y, 0), outPos);
				outPos += info.origin;
				BoneMatrix[i][0][3] = outPos.x;
				BoneMatrix[i][1][3] = outPos.y;
				BoneMatrix[i][2][3] = outPos.z;
			}
			g_pModelRender->ForcedMaterialOverride(pulse);
			oDrawModelExecute(ecx, context, state, info, BoneMatrix);
			g_pRenderView->SetColorModulation(old_color);
			g_pModelRender->ForcedMaterialOverride(NULL);
		}
		if (g_menu.config.visuals.local_chams) {
			switch (g_menu.config.visuals.localchammat) {
				case 0: mat = texture; break;
				case 1: mat = flat; break;
				case 2: mat = g_pMaterialSys->FindMaterial("models/inventory_items/dogtags/dogtags_outline", "Model textures"); break;
			}
			g_pModelRender->ForcedMaterialOverride(mat);
			g_pRenderView->SetColorModulation(color);
			g_pRenderView->SetBlend(g_menu.config.visuals.local_chams_opacity / 100.f);
			oDrawModelExecute(ecx, context, state, info, matrix);
		}
	}

	oDrawModelExecute(ecx, context, state, info, matrix);
	if (!scene_end && !strstr(ModelName, "models/arms"))
		g_pModelRender->ForcedMaterialOverride(NULL);
}
void __fastcall Hooks::SceneEnd(void* ecx, void* edx)
{
	static auto oSceneEnd = g_Hooks.pRenderViewHook->GetOriginal<SceneEnd_t>(vtable_indexes::sceneEnd);
	oSceneEnd(ecx, edx);

	static IMaterial* texture = chams::get().create_material(false, true, false);
	static IMaterial* flat = chams::get().create_material(false, false, false);

	static IMaterial* texture_xqz = chams::get().create_material(true, true, false);
	static IMaterial* flat_xqz = chams::get().create_material(true, false, false);
	
	float colors[3] = { 0.f };
	colors[0] = g_menu.config.visuals.ChamsColor.red / 255.f;
	colors[1] = g_menu.config.visuals.ChamsColor.green / 255.f;
	colors[2] = g_menu.config.visuals.ChamsColor.blue / 255.f;

	float colors_xqz[3] = { 0.f };
	colors_xqz[0] = g_menu.config.visuals.ChamsColorXQZ.red / 255.f;
	colors_xqz[1] = g_menu.config.visuals.ChamsColorXQZ.green / 255.f;
	colors_xqz[2] = g_menu.config.visuals.ChamsColorXQZ.blue / 255.f;

	float colorsbt[3] = { 0.f };
	colorsbt[0] = g_menu.config.visuals.btchamcolor.red / 255.f;
	colorsbt[1] = g_menu.config.visuals.btchamcolor.green / 255.f;
	colorsbt[2] = g_menu.config.visuals.btchamcolor.blue / 255.f;

	for (int i = 0; i <= 64; ++i)
	{
		C_BaseEntity* e = g_pEntityList->GetClientEntity(i);

		if (!e || !e->IsAlive() || e->IsDormant() || e == g::local)
			continue;

		/*if (g_menu.config.visuals.Esp) {
			Vector origAng;
			origAng = g::local->GetEyeAngles();
			g::local->set_angles(Vector(0, fake_angle.y, 0));
			g_pModelRender->ForcedMaterialOverride(flat_xqz);
			g_pRenderView->SetBlend(0.5f);
			g_pRenderView->SetColorModulation(colors);
			g::local->DrawModel(0x1, 255);
			g_pModelRender->ForcedMaterialOverride(nullptr);
			g::local->set_angles(origAng);
		}*/
		static IMaterial* mat;
		switch (g_menu.config.visuals.chammat) {
		case 0: mat = texture; break;
		case 1: mat = flat; break;
		}
		if (g_menu.config.visuals.ChamsXQZ) {
			scene_end = true;
			g_pRenderView->SetBlend(g_menu.config.visuals.chams_opacity / 100.f);
			if (g_menu.config.visuals.chammat == 1)
				g_pModelRender->ForcedMaterialOverride(flat_xqz);
			else if (g_menu.config.visuals.chammat == 0)
				g_pModelRender->ForcedMaterialOverride(texture_xqz);

			g_pRenderView->SetColorModulation(colors_xqz);
			e->DrawModel(0x1, 255);
			//g_pModelRender->ForcedMaterialOverride(nullptr);

			g_pModelRender->ForcedMaterialOverride(mat);
			g_pRenderView->SetColorModulation(colors);
			e->DrawModel(0x1, 255);
			g_pModelRender->ForcedMaterialOverride(nullptr);
		}
		else if (g_menu.config.visuals.Chams) {
			scene_end = true;
			g_pRenderView->SetBlend(g_menu.config.visuals.chams_opacity / 100.f);
			g_pModelRender->ForcedMaterialOverride(mat);
			g_pRenderView->SetColorModulation(colors);
			e->DrawModel(0x1, 255);
			g_pModelRender->ForcedMaterialOverride(nullptr);
		}
		for (auto i = 0; i < g_GlowObject->GetSize(); i++) {
			auto& glow = g_GlowObject->m_GlowObjectDefinitions[i];
			auto e = reinterpret_cast<C_BaseEntity*>(glow.get_entity());

			if (glow.IsUnused())
				continue;

			if (e->GetClientClass()->ClassID == EClassIds::CCSPlayer) {
				if (e->GetTeam() != g::local->GetTeam() && g_menu.config.visuals.Esp && g_menu.config.visuals.glow) {
					glow.red = g_menu.config.visuals.glow_color.red / 255.f;
					glow.green = g_menu.config.visuals.glow_color.green / 255.f;
					glow.blue = g_menu.config.visuals.glow_color.blue / 255.f;
					glow.alpha = g_menu.config.visuals.glow_alpha / 100.f;
					glow.renderWhenOccluded = true;
					glow.renderWhenUnoccluded = false;
				}
			}
		}
		scene_end = false;
	}
}
void __stdcall Hooks::FrameStageNotify(ClientFrameStage_t curStage)
{
	static auto oFrameStage = g_Hooks.pClientHook->GetOriginal<FrameStageNotify_t>(vtable_indexes::frameStage);

	g_Misc.ThirdPerson(curStage);

	g_Resolver.Resolve(curStage);

	if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		//skinchanger.apply();
	}

	oFrameStage(curStage);
}
void __fastcall Hooks::PaintTraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	static auto oPaintTraverse = g_Hooks.pPanelHook->GetOriginal<PaintTraverse_t>(vtable_indexes::paint);

	if (!strcmp("HudZoom", g_pPanel->GetName(vguiPanel)) && g_menu.config.misc.NoScope)
		return;

	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);

	static unsigned int _panel = 0;
	static bool once = false;

	if (!once) {
		PCHAR panel_char = (PCHAR)g_pPanel->GetName(vguiPanel);
		if (strstr(panel_char, "MatSystemTopPanel")) {
			_panel = vguiPanel;
			g_Hooks.bInitializedDrawManager = true;
			once = true;
		}
	}
	else if (_panel == vguiPanel) {
		int width, height;
		g_pEngine->GetScreenSize(width, height);

		auto x = width - 255;
		auto y = 10;
		if (g_menu.config.misc.watermark) {
			g_pSurface->FilledRect(x - 5, y - 5, 247, 32, Color(45, 45, 45));
			g_pSurface->OutlinedRect(x - 5, y - 5, 247, 32, Color(0, 0, 0));

			g_pSurface->FilledRect(x, y, 237, 22, Color(35, 35, 35));
			g_pSurface->OutlinedRect(x, y, 237, 22, Color(0, 0, 0));

			g_pSurface->draw_text(x + 5, y + 3, g::misc_font, "happy", false, Color(215, 215, 215));
			g_pSurface->draw_text(x + 33, y + 3, g::misc_font, "hack", false, menurgba);
			g_pSurface->draw_text(x + 68, y, g::tab_font, "|", false, Color(215, 215, 215));

			time_t now = time(0);
			char* dt = ctime(&now);
			g_pSurface->draw_text(x + 77, y + 3, g::misc_font, dt, false, Color(215, 215, 215));
		}
		int nigga, nigga2;
		g_pEngine->GetScreenSize(nigga, nigga2);
		nigga /= 2, nigga2 /= 2;
		if (g::local && g::local->IsAlive() && g_pEngine->IsInGame() && g_menu.config.visuals.pene_dot)
		{
			float damage = 0.f;
			g_pSurface->RoundedFilledRect(nigga - 2, nigga2 - 2, 5, 5, 2,  g_Autowall.CanWallbang(damage) ? Color(0, 255, 0) : Color(255, 0, 0));
			g_pSurface->RoundedFilledRect(nigga - 3, nigga2 - 3, 7, 7, 3.5, g_Autowall.CanWallbang(damage) ? Color(0, 200, 0, 150) : Color(200, 0, 0, 150));
			//g_pSurface->OutlinedRect(nigga - 3, nigga2 - 3, 6, 6, Color(0, 0, 0));
		}
		if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
		{
			if (g::local != nullptr) {
				if (g_menu.config.misc.NoScope && g::local->IsScoped() && g::local->IsAlive() && g::local->GetActiveWeapon())
				{
					int width, height;
					g_pEngine->GetScreenSize(width, height);
					auto spread = g::local->GetActiveWeapon()->GetInaccuracy() * 200.f;
					int heights = std::clamp(spread, 1.f, 25.f);

					if (heights > 2) {
						g_pSurface->FilledRect(0, height / 2 - (heights / 2), width, heights, Color(0, 0, 0, 170));
						g_pSurface->FilledRect(width / 2 - (heights / 2), 0, heights, height, Color(0, 0, 0, 170));
					}
					else {
						g_pSurface->Line(-1, height / 2, width, height / 2, Color(0, 0, 0));
						g_pSurface->Line(width / 2, -1, width / 2, height, Color(0, 0, 0));
					}
				}
			}


			g_ESP.Render();
			events::print("[HH]");
			int draw_y = 70;
			if (g_menu.config.aimbot.yaw_stand == 6) {
				int width, height;
				g_pEngine->GetScreenSize(width, height);

				float* PosParams = (float*)((uintptr_t)g::local + 0x2774);
				if (PosParams[11] * 58 >= 35)
					g_pSurface->draw_text(10, height - draw_y, g::title_font, "FAKE", false, Color(0, 255, 60));
				else
					g_pSurface->draw_text(10, height - draw_y, g::title_font, "FAKE", false, Color(255, 10, 10));

				draw_y += 32;
			}
			if (GetAsyncKeyState(VK_MENU) && !(g::local->GetFlags() & FL_DUCKING) && g_menu.config.misc.fakeduck) {
				g_pSurface->draw_text(10, height - draw_y, g::title_font, "FAKEDUCK", false, Color(225, 255, 255));
				draw_y += 32;
			}
			static auto linegoesthrusmoke = Utils::FindPattern("client_panorama.dll", (PBYTE)"\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0", "xxxxxxxx????xxx");
			static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);
			static std::vector< const char* > smoke_materials = {
				"particle/vistasmokev1/vistasmokev1_smokegrenade",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
				"particle/vistasmokev1/vistasmokev1_fire",
			};
			if (g_pMaterialSys != nullptr) {
				if (g_menu.config.misc.nosmoke) {
					for (auto matName : smoke_materials) {
						IMaterial* mat = g_pMaterialSys->FindMaterial(matName, "Other textures");
						switch (g_menu.config.misc.nosmokeset) {
						case 0: mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true); mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);  break;
						case 1: mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);  mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false); break;
						}
						if (g_menu.config.misc.nosmoke)
							* (int*)(smokecout) = 0;
					}
				}
				else {
					for (auto matName : smoke_materials) {
						IMaterial* mat = g_pMaterialSys->FindMaterial(matName, "Other textures");
						mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
						mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
					}
				}
			}

			if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
			{
				Vector w2s, w2s2;
				for (int i = 0; i < trace_logs.size(); i++) {
					auto tracer = trace_logs.at(i);

					auto color = Color(g_menu.config.misc.bullet_tracecol.red, g_menu.config.misc.bullet_tracecol.green, g_menu.config.misc.bullet_tracecol.blue);
					if (g_pSurface->world_to_screen(tracer.start, w2s) && g_pSurface->world_to_screen(tracer.end, w2s2)) {
						g_pSurface->Line(w2s.x, w2s.y, w2s2.x, w2s2.y, color);
						g_pSurface->RoundedFilledRect(w2s2.x - 2.5, w2s2.y - 2.5, 5, 5, 2.5, Color(180, 180, 180));
					}

					if ((g_pGlobalVars->curtime - tracer.time) >= 4)
						trace_logs.erase(trace_logs.begin() + i);
				}
			}
			int x; int y;
			g_pEngine->GetScreenSize(x, y);
			x /= 2; y /= 2;
			if (hit_time > 0) {
				//left upper
				g_pSurface->Line(x - 8, y - 6, x - 3, y - 2, Color(255, 255, 255, hit_time));
				g_pSurface->Line(x - 8, y - 7, x - 3, y - 3, Color(255, 255, 255, hit_time));

				//left bottom
				g_pSurface->Line(x - 8, y + 7, x - 3, y + 3, Color(255, 255, 255, hit_time));
				g_pSurface->Line(x - 8, y + 6, x - 3, y + 2, Color(255, 255, 255, hit_time));

				//right upper
				g_pSurface->Line(x + 8, y - 6, x + 3, y - 2, Color(255, 255, 255, hit_time));
				g_pSurface->Line(x + 8, y - 7, x + 3, y - 3, Color(255, 255, 255, hit_time));

				//right bottom
				g_pSurface->Line(x + 8, y + 7, x + 3, y + 3, Color(255, 255, 255, hit_time));
				g_pSurface->Line(x + 8, y + 6, x + 3, y + 2, Color(255, 255, 255, hit_time));

				hit_time -= 3;
			}
			bool defuse = false, ent_has_defuser = false;
			for (int i = 0; i <= 64; i++) {
				C_BaseEntity* e = g_pEntityList->GetClientEntity(i);
				if (!e || !e->IsAlive()) continue;

				if (e->defusing())
					defuse = true;

				if (e->has_defuser())
					ent_has_defuser = true;
			}
			for (int i = 0; i <= 2048; i++) {
				C_BaseEntity* e = g_pEntityList->GetClientEntity(i);
				if (!e) continue;

				if (e->GetClientClass()->ClassID == EClassIds::CPlantedC4) {
					ccsbomb* bomb = (ccsbomb*)e;
					if (bomb) {

						if (!g_menu.config.misc.bomb_timer || bomb->bomb_defused()) continue;

						float c4_clock = bomb->c4_blow_time() - g_pGlobalVars->curtime;
						char buf[24];
						sprintf_s(buf, sizeof(buf) - 1, "Bomb Time: %.1f", c4_clock);
						g_pSurface->draw_text(890 + 20, 90, g::menu_font2, c4_clock >= 0 ? buf : "Bomb Time: 0.0", false, Color(220, 220, 220));
						g_pSurface->FilledRect(619 + 20, 109, 600, 12, Color(0, 0, 0, 180));
						g_pSurface->FilledRect(620 + 20, 110, c4_clock * 15, 10, menurgba);


						float c4_defuse = bomb->c4_defuse_count_down() - g_pGlobalVars->curtime;
						char buff[24];
						sprintf_s(buff, sizeof(buff) - 1, "Defuse Time: %.1f", c4_defuse);
						if (g::local->defusing() || defuse) {
							auto multi = 0;
							if (g::local->has_defuser() || ent_has_defuser)
								multi = 100;
							else
								multi = 60;

							g_pSurface->draw_text(890 + 20, 135, g::menu_font2, c4_defuse >= 0 ? buff : "Defuse Time: 0.0", false, Color(220, 220, 220));
							g_pSurface->FilledRect(619 + 20, 154, 600, 12, Color(0, 0, 0, 180));
							g_pSurface->FilledRect(620 + 20, 155, c4_defuse * multi, 10, menurgba);
						}
					}
				}
			}

		}
		if (g_menu.menuOpened)
			Drop::DrawBackDrop();

		g_menu.Render();
	}
}

void Event::FireGameEvent(IGameEvent* event)
{
	if (!event)
		return;

	if (!g::local)
		return;

	auto attacker = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("attacker")));
	if (!attacker)
		return;

	if (attacker != g::local)
		return;

	int index = g_pEngine->GetPlayerForUserID(event->GetInt("userid"));

	PlayerInfo_t pInfo;
	g_pEngine->GetPlayerInfo(index, &pInfo);

	g::Hit[index] = true;

	if (g_menu.config.misc.hitmarker) {
		hit_time = 255;
		switch (g_menu.config.misc.Hitmarkers) {
		case 1: g_pEngine->ExecuteClientCmd("play physics\\metal\\paintcan_impact_hard3.wav"); break;
		case 2: g_pEngine->ExecuteClientCmd("play buttons\\arena_switch_press_02.wav"); break;
		}
	}
	health(event);
};

void __fastcall Hooks::OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	static auto oOverrideView = g_Hooks.pClientModeHook->GetOriginal<OverrideView_t>(vtable_indexes::view);

	if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
	{
		if (!g::local)
			return;

		if (g_menu.config.misc.NoRecoil)
		{
			Vector viewPunch = g::local->GetViewPunchAngle();
			Vector aimPunch = g::local->GetAimPunchAngle();

			pSetup->angles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
			pSetup->angles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
			pSetup->angles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		}

		if (g_menu.config.visuals.Fov != 0 && !g::local->IsScoped())
			pSetup->fov = g_menu.config.visuals.Fov;

		if (g_menu.config.misc.NoZoom&& g::local->IsScoped())
			pSetup->fov = (g_menu.config.visuals.Fov == 0) ? 90 : g_menu.config.visuals.Fov;

		static float ViewModel_X_Backup = 0.f;
		static float ViewModel_Y_Backup = 0.f;
		static float ViewModel_Z_Backup = 0.f;

		static auto ViewModel_X_Var = g_pCvar->FindVar("viewmodel_offset_x");
		static auto ViewModel_Y_Var = g_pCvar->FindVar("viewmodel_offset_y");
		static auto ViewModel_Z_Var = g_pCvar->FindVar("viewmodel_offset_z");


		if (ViewModel_X_Backup != g_menu.config.visuals.nViewmodelX || ViewModel_Y_Backup != g_menu.config.visuals.nViewmodelY || ViewModel_Z_Backup != g_menu.config.visuals.nViewmodelZ ||
			ViewModel_X_Backup != g_menu.config.visuals.ViewmodelX || ViewModel_Y_Backup != g_menu.config.visuals.ViewmodelY || ViewModel_Z_Backup != g_menu.config.visuals.ViewmodelZ)
		{

			if (g_menu.config.visuals.nViewmodelX == 0) {
				*(float*)((DWORD)& ViewModel_X_Var->callbacks + 0xC) = 0;
				ViewModel_X_Var->SetValue(g_menu.config.visuals.ViewmodelX);
				ViewModel_X_Backup = g_menu.config.visuals.ViewmodelX;
			}
			else {
				*(float*)((DWORD)& ViewModel_X_Var->callbacks + 0xC) = 0;
				ViewModel_X_Var->SetValue(-g_menu.config.visuals.nViewmodelX);
				ViewModel_X_Backup = -g_menu.config.visuals.nViewmodelX;
			}

			if (g_menu.config.visuals.nViewmodelY == 0) {
				*(float*)((DWORD)& ViewModel_Y_Var->callbacks + 0xC) = 0;
				ViewModel_Y_Var->SetValue(g_menu.config.visuals.ViewmodelY);
				ViewModel_Y_Backup = g_menu.config.visuals.ViewmodelY;
			}
			else {
				*(float*)((DWORD)& ViewModel_Y_Var->callbacks + 0xC) = 0;
				ViewModel_Y_Var->SetValue(-g_menu.config.visuals.nViewmodelY);
				ViewModel_Y_Backup = -g_menu.config.visuals.nViewmodelY;
			}

			*(float*)((DWORD)& ViewModel_Z_Var->callbacks + 0xC) = 0;
			ViewModel_Z_Var->SetValue(g_menu.config.visuals.ViewmodelZ - 10);
			ViewModel_Z_Backup = g_menu.config.visuals.ViewmodelZ - 10;
		}
		if (GetAsyncKeyState(VK_MENU)) {
			if (g_menu.config.misc.fakeduck&& g::local&& g::local->IsAlive())
				pSetup->origin.z = g::local->GetAbsOrigin().z + 64.f;
		}
		auto ViewModel = reinterpret_cast<C_BaseEntity*>(g_pEntityList->GetClientEntityFromHandle(g::local->m_hViewModel()));
		if (ViewModel)
		{
			auto eyeAng = pSetup->angles;
			eyeAng.z -= g_menu.config.visuals.roll;
			eyeAng.y += g_menu.config.visuals.swing;
			eyeAng.x += g_menu.config.visuals.tilt;
			ViewModel->SetAbsAngles(eyeAng);
		}
	}

	oOverrideView(ecx, edx, pSetup);
}

void __fastcall Hooks::LockCursor(ISurface* thisptr, void* edx)
{
    static auto oLockCursor = g_Hooks.pSurfaceHook->GetOriginal<LockCursor_t>(vtable_indexes::lockCursor);

    if (!g_menu.menuOpened)
        return oLockCursor(thisptr, edx);

    g_pSurface->UnLockCursor();
}

LRESULT Hooks::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    const auto getButtonHeld = [uMsg, wParam](bool& bButton, int vKey) {
		if (wParam != vKey) return;
        if (uMsg == WM_KEYDOWN)
            bButton = true;
        else if (uMsg == WM_KEYUP)
            bButton = false;
    };
	const auto getButtonToggle = [uMsg, wParam](bool& bButton, int vKey) {
		if (wParam != vKey) return;

		if (uMsg == WM_KEYUP)
			bButton = !bButton;
	};
    if (g_Hooks.bInitializedDrawManager) {
        if (g_menu.menuOpened)
            return true;
    }
    return CallWindowProcA(g_Hooks.pOriginalWNDProc, hWnd, uMsg, wParam, lParam);
}
