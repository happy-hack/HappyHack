#include "CEntity.h"
#include "IVModelInfo.h"
#include "CGlobalVarsBase.h"
#include "..\Utils\GlobalVars.h"
#include "..\Utils\Math.h"

Vector C_BaseEntity::GetHitboxPosition(int Hitbox, matrix3x4_t *Matrix, float *Radius)
{
	studiohdr_t* hdr = g_pModelInfo->GetStudiomodel(this->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	mstudiobbox_t* hitbox = set->GetHitbox(Hitbox);

	if (hitbox)
	{
		Vector vMin, vMax, vCenter, sCenter;
		g_Math.VectorTransform(hitbox->min, Matrix[hitbox->bone], vMin);
		g_Math.VectorTransform(hitbox->max, Matrix[hitbox->bone], vMax);
		vCenter = (vMin + vMax) * 0.5;

		*Radius = hitbox->radius;

		return vCenter;
	}
	
	return Vector(0, 0, 0);
}

Vector C_BaseEntity::GetHitboxPosition(int Hitbox, matrix3x4_t *Matrix) // any public source
{
	studiohdr_t* hdr = g_pModelInfo->GetStudiomodel(this->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	mstudiobbox_t* hitbox = set->GetHitbox(Hitbox);

	if (hitbox)
	{
		Vector vMin, vMax, vCenter, sCenter;
		g_Math.VectorTransform(hitbox->min, Matrix[hitbox->bone], vMin);
		g_Math.VectorTransform(hitbox->max, Matrix[hitbox->bone], vMax);
		vCenter = (vMin + vMax) * 0.5;

		return vCenter;
	}

	return Vector(0, 0, 0);
}
Vector C_BaseEntity::get_hitbox_pos(C_BaseEntity* m_entity, int Hitbox) {
	matrix3x4_t matrix[128];
	if (m_entity->SetupBones(matrix, 128, 0x100, 0.f)) {
		studiohdr_t* hdr = g_pModelInfo->GetStudiomodel(this->GetModel());
		if (hdr != nullptr) {
			mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
			mstudiobbox_t* hitbox = set->GetHitbox(Hitbox);

			if (hitbox) {
				Vector vMin, vMax, vCenter, sCenter;
				g_Math.VectorTransform(hitbox->min, matrix[hitbox->bone], vMin);
				g_Math.VectorTransform(hitbox->max, matrix[hitbox->bone], vMax);

				vCenter = (vMin + vMax) * 0.5f;
				return vCenter;
			}
		}
	}
	return Vector(0, 0, 0);
}
void C_BaseEntity::FixSetupBones(matrix3x4_t *Matrix)
{
	int Backup = *(int*)((uintptr_t)this + 0x274);
	*(int*)((uintptr_t)this + 0x274) = 0;
	Vector absOriginBackupLocal = this->GetAbsOrigin();
	this->SetAbsOrigin(this->GetOrigin());
	this->SetupBones(Matrix, 128, 0x00000100, g_pGlobalVars->curtime);
	this->SetAbsOrigin(absOriginBackupLocal);
	*(int*)((uintptr_t)this + 0x274) = Backup;
}

//SanekGame https://www.unknowncheats.me/forum/1798568-post2.html

void C_BaseEntity::SetAbsAngles(Vector angles)
{
	using Fn = void(__thiscall*)(C_BaseEntity*, const Vector& angles);
	static Fn AbsAngles = (Fn)(Utils::FindPattern("client_panorama.dll", (BYTE*)"\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x64\x53\x56\x57\x8B\xF1\xE8", "xxxxxxxxxxxxxxx"));

	AbsAngles(this, angles);
}

void C_BaseEntity::SetAbsOrigin(Vector origin)
{
	using Fn = void(__thiscall*)(void*, const Vector &origin);
	static Fn AbsOrigin = (Fn)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");

	AbsOrigin(this, origin);
}

void C_BaseEntity::SetAbsVelocity(Vector velocity) // i dont remember
{
	using Fn = void(__thiscall*)(void*, const Vector &velocity);
	static Fn AbsVelocity = (Fn)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3");

	AbsVelocity(this, velocity);
}

bool C_BaseEntity::IsNade()
{
	if (!this)
		return false;
	if (!this->IsAlive())
		return false;

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)this->GetActiveWeapon();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	if (WeaponName == "weapon_incgrenade")
		return true;
	else if (WeaponName == "weapon_decoy")
		return true;
	else if (WeaponName == "weapon_flashbang")
		return true;
	else if (WeaponName == "weapon_hegrenade")
		return true;
	else if (WeaponName == "weapon_smokegrenade")
		return true;
	else if (WeaponName == "weapon_molotov")
		return true;

	return false;
}

bool C_BaseEntity::IsKnife()
{
	if (!this)
		return false;
	if (!this->IsAlive())
		return false;

	C_BaseCombatWeapon * pWeapon = (C_BaseCombatWeapon*)this->GetActiveWeapon();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	if (WeaponName == "weapon_knife")
		return true;

	return false;
}

float C_BaseEntity::FireRate()
{
	if (!this)
		return 0.f;
	if (!this->IsAlive())
		return 0.f;
	if (this->IsNade() || this->IsKnife())
		return 0.f;

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)this->GetActiveWeapon();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	if (WeaponName == "weapon_glock")
		return 0.15f;
	else if (WeaponName == "weapon_hkp2000")
		return 0.169f;
	else if (WeaponName == "weapon_p250")//the cz and p250 have the same name idky same with other guns
		return 0.15f;
	else if (WeaponName == "weapon_tec9")
		return 0.12f;
	else if (WeaponName == "weapon_elite")
		return 0.12f;
	else if (WeaponName == "weapon_fiveseven")
		return 0.15f;
	else if (WeaponName == "weapon_deagle")
		return 0.224f;
	else if (WeaponName == "weapon_nova")
		return 0.882f;
	else if (WeaponName == "weapon_sawedoff")
		return 0.845f;
	else if (WeaponName == "weapon_mag7")
		return 0.845f;
	else if (WeaponName == "weapon_xm1014")
		return 0.35f;
	else if (WeaponName == "weapon_mac10")
		return 0.075f;
	else if (WeaponName == "weapon_ump45")
		return 0.089f;
	else if (WeaponName == "weapon_mp9")
		return 0.070f;
	else if (WeaponName == "weapon_bizon")
		return 0.08f;
	else if (WeaponName == "weapon_mp7")
		return 0.08f;
	else if (WeaponName == "weapon_p90")
		return 0.070f;
	else if (WeaponName == "weapon_galilar")
		return 0.089f;
	else if (WeaponName == "weapon_ak47")
		return 0.1f;
	else if (WeaponName == "weapon_sg556")
		return 0.089f;
	else if (WeaponName == "weapon_m4a1")
		return 0.089f;
	else if (WeaponName == "weapon_aug")
		return 0.089f;
	else if (WeaponName == "weapon_m249")
		return 0.08f;
	else if (WeaponName == "weapon_negev")
		return 0.0008f;
	else if (WeaponName == "weapon_ssg08")
		return 1.25f;
	else if (WeaponName == "weapon_awp")
		return 1.463f;
	else if (WeaponName == "weapon_g3sg1")
		return 0.25f;
	else if (WeaponName == "weapon_scar20")
		return 0.25f;
	else if (WeaponName == "weapon_mp5sd")
		return 0.08f;
	else
		return .0f;
	
}
bool C_BaseEntity::scoped_wep() {
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)this->GetActiveWeapon();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	if (WeaponName == "weapon_ssg08")
		return true;
	else if (WeaponName == "weapon_awp")
		return true;
	else if (WeaponName == "weapon_g3sg1")
		return true;
	else if (WeaponName == "weapon_scar20")
		return true;
}
bool C_BaseEntity::IsEnemy()
{
	return this->GetTeam() != g::local->GetTeam();
}