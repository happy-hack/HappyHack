#include "ESP.h"
#include "..\Aimbot\Aimbot.h"
#include "..\Aimbot\LagComp.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\SDK\ISurface.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\Utils\Math.h"
#include "..\..\Menu\Menu.h"
#include "../../Hooks.h"

ESP g_ESP;
static int add = 0;
#define degrees_2_radians( x ) ( ( float )( x ) * ( float )( ( float )( 3.14159265358979323846f ) / 180.0f ) )
constexpr double pi = 3.14159265358979323846;
#define deg_to_rad( x )  ( (float)(x) * (float)( pi / 180.f ) )
std::string clean_name(std::string name) {
	std::string wep = name;
	if (wep[0] == 'W') wep.erase(wep.begin());

	auto end_of_weapon = wep.find("EAPON_");
	if (end_of_weapon != std::string::npos)
		wep.erase(wep.begin() + end_of_weapon, wep.begin() + end_of_weapon + 6);

	return wep;
}
void ESP::RenderBox(int alpha, int outline_alpha)
{
	int width = Box.right - 5;
	int xaxis = Box.left + 5;
	g_pSurface->OutlinedRect(xaxis, Box.top, width, Box.bottom, Color(255, 255, 255, outline_alpha));
	g_pSurface->OutlinedRect(xaxis + 1, Box.top + 1, width - 2, Box.bottom - 2, Color(0, 0, 0, outline_alpha));
	g_pSurface->OutlinedRect(xaxis - 1, Box.top - 1, width + 2, Box.bottom + 2, Color(0, 0, 0, outline_alpha));
}
void rotate_arrow(std::array< Vector2D, 3 > & points, float rotation)
{
	const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
	for (auto& point : points) {
		point -= points_center;

		const auto temp_x = point.x;
		const auto temp_y = point.y;

		const auto theta = deg_to_rad(rotation);
		const auto c = cos(theta);
		const auto s = sin(theta);

		point.x = temp_x * c - temp_y * s;
		point.y = temp_x * s + temp_y * c;

		point += points_center;
	}
}
void draw_filled_triangle(std::array< Vector2D, 3 > points, Color col) {
	std::array< Vertex_t, 3 > vertices
	{
		Vertex_t(points.at(0)),
		Vertex_t(points.at(1)),
		Vertex_t(points.at(2))
	};
	g_pSurface->DrawSetColor(col.red, col.green, col.blue, col.alpha);
	g_pSurface->DrawTexturedPolygon(3, vertices.data(), false);
}
void ESP::fov_arrow(C_BaseEntity * ent, int poopie) {
	Vector viewangles;
	g_pEngine->GetViewAngles(viewangles);
	int w, h;
	g_pEngine->GetScreenSize(w, h);

	const auto screen_center = Vector2D(w * .5f, h * .5f);
	const auto in = (1.f / 7.f) * g_pGlobalVars->frametime;
	const auto out = (1.f / 7.f) * g_pGlobalVars->frametime;
	if (esp_fov_fade[ent->EntIndex()] < 1.f) {
		esp_fov_fade[ent->EntIndex()] += in;
	}

	if (esp_fov_fade[ent->EntIndex()] > 0.f) {
		esp_fov_fade[ent->EntIndex()] -= out;
	}
	const int alpha = esp_alpha_fade[ent->EntIndex()] * 255;
	const int outline_alpha = alpha * 0.9f;
	const auto angle_yaw_rad = deg_to_rad(viewangles.y - g_Math.CalcAngle(g::local->GetOrigin(), ent->GetOrigin()).y - 90);

	int radius = max(10, g_menu.config.visuals.fov_arrows_dia);
	int size = max(5, g_menu.config.visuals.fov_arrows_size);

	const auto new_point_x = screen_center.x + ((((w - (size * 3)) * .5f) * (radius / 100.0f)) * cos(angle_yaw_rad)) + (int)(6.0f * (((float)size - 4.f) / 16.0f));
	const auto new_point_y = screen_center.y + ((((h - (size * 3)) * .5f) * (radius / 100.0f)) * sin(angle_yaw_rad));

	Color clr(225, 225, 225);

	std::array< Vector2D, 3 >points
	{
		Vector2D(new_point_x - size, new_point_y - size),
		Vector2D(new_point_x + size, new_point_y),
		Vector2D(new_point_x - size, new_point_y + size)
	};

	rotate_arrow(points, viewangles.y - g_Math.CalcAngle(g::local->GetAbsOrigin(), ent->GetAbsOrigin()).y - 90);
	if (ent->IsDormant()) {
		draw_filled_triangle(points, Color(75, 75, 75, poopie));
	}
	else {
		draw_filled_triangle(points, Color(g_menu.config.visuals.fov_arrows_color.red, g_menu.config.visuals.fov_arrows_color.green, g_menu.config.visuals.fov_arrows_color.blue, outline_alpha));
	}
}
void ESP::RenderName(C_BaseEntity * pEnt, int iterator, int alpha, int outline_alpha)
{
	PlayerInfo_t pInfo;
	g_pEngine->GetPlayerInfo(iterator, &pInfo);

	if (!g_menu.config.visuals.Name)
		return;

	g_pSurface->DrawT(Box.left + (Box.right / 2), Box.top - 12, Color(255, 255, 255, outline_alpha), font, true, pInfo.szName);
}

void ESP::RenderWeaponName(C_BaseEntity * pEnt, int alpha, int outline_alpha)
{
	auto weapon = pEnt->GetActiveWeapon();

	if (!weapon || !g_menu.config.visuals.Weapon)
		return;

	std::string wep_str = weapon->GetName();

	std::transform(wep_str.begin(), wep_str.end(), wep_str.begin(), ::toupper);

	char ammo[519];
	sprintf_s(ammo, " - [%d / %d]", weapon->GetAmmo(), weapon->m_iPrimaryReserveAmmoCount());

	g_pSurface->DrawT(Box.left + (Box.right / 2), Box.top + Box.bottom + add, Color(255, 255, 255, outline_alpha), font, true, (clean_name(wep_str) + (g_menu.config.visuals.ammo[1] ? ammo : "")).c_str());
}

void ESP::RenderHealth(C_BaseEntity * pEnt, int alpha, int outline_alpha)
{
	auto health = pEnt->GetHealth();
	if (health > 100)
		health = 100;

	int pixelValue = health * Box.bottom / 100;
	if (g_menu.config.visuals.HealthVal[1])
	{
		g_pSurface->FilledRect(Box.left, Box.top, 2, Box.bottom, Color(20, 20, 20, outline_alpha));
		g_pSurface->FilledRect(Box.left, Box.top + Box.bottom - pixelValue, 2, pixelValue, Color(0, 255, 0, outline_alpha));
		g_pSurface->OutlinedRect(Box.left - 1, Box.top - 1, 4, Box.bottom + 2, Color(35, 35, 35, outline_alpha));
	}
	if (g_menu.config.visuals.HealthVal[0] && health < 100)
	{
		std::string Health = std::to_string(health);

		auto size = g_pSurface->get_text_size(g::esp_font2, Health.c_str());
		g_pSurface->DrawT(Box.left - (size.left * 0.6), (Box.top + Box.bottom - pixelValue) - 8, Color(255, 255, 255, outline_alpha), g::esp_font2, false, Health.c_str());
	}
}
void ESP::ammo(C_BaseEntity * ent, int alpha, int outline_alpha) {
	if (!ent->GetActiveWeapon()->GetCSWpnData_2()) return;
	int
		max_in_clip = ent->GetActiveWeapon()->GetCSWpnData_2()->iMaxClip1,
		ammo_in_clip = ent->GetActiveWeapon()->GetAmmo(),
		scaled_ammo = ammo_in_clip * Box.right / max_in_clip,
		shit = std::clamp(scaled_ammo, 0, Box.right);

	g_pSurface->FilledRect(Box.left + 5, Box.top + Box.bottom + 3, Box.right - 5, 2, Color(20, 20, 20, outline_alpha));
	g_pSurface->OutlinedRect(Box.left + 4, Box.top + Box.bottom + 2, Box.right - 3, 4, Color(35, 35, 35, outline_alpha));
	g_pSurface->FilledRect(Box.left + 5, Box.top + Box.bottom + 3, shit - 5, 2, Color(0, 191, 191, outline_alpha));
}

void ESP::player_flags(C_BaseEntity * ent, int alpha, int outline_alpha) {
	std::vector<std::string> info;
	if (ent->IsDormant()) return;
	if (ent->IsScoped() && g_menu.config.visuals.flags[0])
		info.push_back("SCOPED");

	if ((ent->GetFlags()& FL_DUCKING) && g_menu.config.visuals.flags[1])
		info.push_back("DUCK");

	if (ent->duck_amount() > 1060000000 && ent->duck_amount() < 1062000000) {
		if (g_menu.config.visuals.flags[1])
			info.push_back("FD");

		g::fake_duck = true;
	}
	if (g_menu.config.visuals.flags[2]) {
		if (ent->ArmorValue() > 0) {
			if (!ent->HasHelmet())
				info.push_back("K");
			else
				info.push_back("HK");
		}
	}

	static RECT size = g_pSurface->get_text_size(g::esp_font, "A");
	int loop = 0;
	for (auto text : info) {
		g_pSurface->draw_text(Box.left + Box.right + 3, Box.top + (size.bottom * loop) / 1.2, g::esp_font, text.c_str(), false, Color(225, 225, 255, 225));
		loop++;
	}
}
void ESP::BoundBox(C_BaseEntity * pEnt, int alpha, int outline_alpha)
{
	Box.bottom = 0;
	Box.top = 0;
	Box.left = 0;
	Box.right = 0;

	Vector2D w2sBottom, w2sTop;

	Utils::WorldToScreen(pEnt->GetOrigin() - Vector(0, 0, 8), w2sBottom);
	Utils::WorldToScreen(pEnt->GetHitboxPosition(0, str_matrix.matrix[pEnt->EntIndex()]) + Vector(0, 0, 10), w2sTop);

	int Middle = w2sBottom.y - w2sTop.y;
	int Width = Middle / 3.f;

	Box.bottom = Middle;
	Box.top = w2sTop.y;
	Box.left = w2sBottom.x - Width;
	Box.right = Width * 2;
}
void ESP::RenderSkeleton(C_BaseEntity* pEnt) // the best
{
	Vector Hitbox[19];
	Vector2D Hitboxw2s[19];

	for (int hitbox = 0; hitbox < 19; hitbox++)
	{
		Hitbox[hitbox] = pEnt->GetHitboxPosition(hitbox, str_matrix.matrix[pEnt->EntIndex()]);
		Utils::WorldToScreen(Hitbox[hitbox], Hitboxw2s[hitbox]);
	}

	//spine
	g_pSurface->Line(Hitboxw2s[HITBOX_HEAD].x, Hitboxw2s[HITBOX_HEAD].y, Hitboxw2s[HITBOX_NECK].x, Hitboxw2s[HITBOX_NECK].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_NECK].x, Hitboxw2s[HITBOX_NECK].y, Hitboxw2s[HITBOX_UPPER_CHEST].x, Hitboxw2s[HITBOX_UPPER_CHEST].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_UPPER_CHEST].x, Hitboxw2s[HITBOX_UPPER_CHEST].y, Hitboxw2s[HITBOX_LOWER_CHEST].x, Hitboxw2s[HITBOX_LOWER_CHEST].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_LOWER_CHEST].x, Hitboxw2s[HITBOX_LOWER_CHEST].y, Hitboxw2s[HITBOX_THORAX].x, Hitboxw2s[HITBOX_THORAX].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_THORAX].x, Hitboxw2s[HITBOX_THORAX].y, Hitboxw2s[HITBOX_BELLY].x, Hitboxw2s[HITBOX_BELLY].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_BELLY].x, Hitboxw2s[HITBOX_BELLY].y, Hitboxw2s[HITBOX_PELVIS].x, Hitboxw2s[HITBOX_PELVIS].y, skelecolor);

	//right leg
	g_pSurface->Line(Hitboxw2s[HITBOX_PELVIS].x, Hitboxw2s[HITBOX_PELVIS].y, Hitboxw2s[HITBOX_RIGHT_THIGH].x, Hitboxw2s[HITBOX_RIGHT_THIGH].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_RIGHT_THIGH].x, Hitboxw2s[HITBOX_RIGHT_THIGH].y, Hitboxw2s[HITBOX_RIGHT_CALF].x, Hitboxw2s[HITBOX_RIGHT_CALF].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_RIGHT_CALF].x, Hitboxw2s[HITBOX_RIGHT_CALF].y, Hitboxw2s[HITBOX_RIGHT_FOOT].x, Hitboxw2s[HITBOX_RIGHT_FOOT].y, skelecolor);

	//right arm
	g_pSurface->Line(Hitboxw2s[HITBOX_NECK].x, Hitboxw2s[HITBOX_NECK].y, Hitboxw2s[HITBOX_RIGHT_UPPER_ARM].x, Hitboxw2s[HITBOX_RIGHT_UPPER_ARM].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_RIGHT_UPPER_ARM].x, Hitboxw2s[HITBOX_RIGHT_UPPER_ARM].y, Hitboxw2s[HITBOX_RIGHT_FOREARM].x, Hitboxw2s[HITBOX_RIGHT_FOREARM].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_RIGHT_FOREARM].x, Hitboxw2s[HITBOX_RIGHT_FOREARM].y, Hitboxw2s[HITBOX_RIGHT_HAND].x, Hitboxw2s[HITBOX_RIGHT_HAND].y, skelecolor);

	//left leg
	g_pSurface->Line(Hitboxw2s[HITBOX_PELVIS].x, Hitboxw2s[HITBOX_PELVIS].y, Hitboxw2s[HITBOX_LEFT_THIGH].x, Hitboxw2s[HITBOX_LEFT_THIGH].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_LEFT_THIGH].x, Hitboxw2s[HITBOX_LEFT_THIGH].y, Hitboxw2s[HITBOX_LEFT_CALF].x, Hitboxw2s[HITBOX_LEFT_CALF].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_LEFT_CALF].x, Hitboxw2s[HITBOX_LEFT_CALF].y, Hitboxw2s[HITBOX_LEFT_FOOT].x, Hitboxw2s[HITBOX_LEFT_FOOT].y, skelecolor);

	//left arm
	g_pSurface->Line(Hitboxw2s[HITBOX_NECK].x, Hitboxw2s[HITBOX_NECK].y, Hitboxw2s[HITBOX_LEFT_UPPER_ARM].x, Hitboxw2s[HITBOX_LEFT_UPPER_ARM].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_LEFT_UPPER_ARM].x, Hitboxw2s[HITBOX_LEFT_UPPER_ARM].y, Hitboxw2s[HITBOX_LEFT_FOREARM].x, Hitboxw2s[HITBOX_LEFT_FOREARM].y, skelecolor);
	g_pSurface->Line(Hitboxw2s[HITBOX_LEFT_FOREARM].x, Hitboxw2s[HITBOX_LEFT_FOREARM].y, Hitboxw2s[HITBOX_LEFT_HAND].x, Hitboxw2s[HITBOX_LEFT_HAND].y, skelecolor);
}
void ESP::Render()
{
	if (!g::local || !g_pEngine->IsInGame() || !g_menu.config.visuals.Esp)
		return;
	color = Color(225, 225, 225);
	textcolor = Color(225, 225, 225, 255);
	font = g::esp_font;

	for (int i = 1; i < g_pEngine->GetMaxClients(); ++i)
	{
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);
		//esp_alpha_fade[pPlayerEntity->EntIndex()] = 255;
		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| pPlayerEntity == g::local
			|| pPlayerEntity->GetTeam() == g::local->GetTeam())
			continue;
		const auto in = (1.f / 0.5f) * g_pGlobalVars->frametime;
		const auto out = (1.f / 7.f) * g_pGlobalVars->frametime;
		if (!pPlayerEntity->IsDormant()) {
			if (esp_alpha_fade[pPlayerEntity->EntIndex()] < 1.f) {
				esp_alpha_fade[pPlayerEntity->EntIndex()] += in;
			}
		}
		else {
			if (esp_alpha_fade[pPlayerEntity->EntIndex()] > 0.f) {
				esp_alpha_fade[pPlayerEntity->EntIndex()] -= out;
			}
		}
		const int alpha = esp_alpha_fade[pPlayerEntity->EntIndex()] * 255;
		const int outline_alpha = alpha * 0.9f;
		offsetY = 0;
		BoundBox(pPlayerEntity, alpha, outline_alpha);
		if (Box.bottom == 0)
			continue;

		if (g_menu.config.visuals.Box)
			RenderBox(alpha, outline_alpha);

		if (g_menu.config.visuals.Name > 0)
			RenderName(pPlayerEntity, i, alpha, outline_alpha);

		if (g_menu.config.visuals.Weapon)
			RenderWeaponName(pPlayerEntity, alpha, outline_alpha);

		if (g_menu.config.visuals.player_flags)
			player_flags(pPlayerEntity, alpha, outline_alpha);

		if (g_menu.config.visuals.fovarrow && g_pEngine->IsInGame())
			fov_arrow(pPlayerEntity, outline_alpha);

		if (g_menu.config.visuals.ammo[0]) {
			ammo(pPlayerEntity, alpha, outline_alpha);
			add = 6;
		}
		else add = 0;

		if (g_menu.config.visuals.HealthBar || g_menu.config.visuals.HealthVal > 0)
			RenderHealth(pPlayerEntity, alpha, outline_alpha);

		skelecolor = Color(g_menu.config.visuals.SkeletonColor.red, g_menu.config.visuals.SkeletonColor.green, g_menu.config.visuals.SkeletonColor.blue, outline_alpha);
		if (g_menu.config.visuals.Skeleton)
			RenderSkeleton(pPlayerEntity);
	}
}