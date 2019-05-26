#pragma once
#include "Definitions.h"
#include "IClientUnknown.h"
#include "IClientEntityList.h"
#include "CInput.h"
#include "..\Utils\Utils.h"
#include "..\Utils\NetvarManager.h"

// class predefinition
class C_BaseCombatWeapon;

enum c_weapon_types {
	type_knife = 0,
	type_pistol = 1,
	type_smg = 2,
	type_rifle = 3,
	type_shotgun = 4,
	type_sniper = 5,
	type_machinegun = 6,
};
class C_AnimState
{
public:
	char pad[3];
	char bUnknown; //0x4
	char pad2[91];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];
	float* feetyaw() //rofl
	{
		return reinterpret_cast<float*>((DWORD)this + 0x70);
	}
};

class AnimationLayer {
public:
	char  pad_0000[20];
	// These should also be present in the padding, don't see the use for it though
	//float	m_flLayerAnimtime;
	//float	m_flLayerFadeOuttime;
	unsigned int m_nOrder; //0x0014
	unsigned int m_nSequence; //0x0018
	float m_flPrevCycle; //0x001C
	float m_flWeight; //0x0020
	float m_flWeightDeltaRate; //0x0024
	float m_flPlaybackRate; //0x0028
	float m_flCycle; //0x002C
	void *m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
}; //Size: 0x0038

class C_BaseEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable
{
private:
    template<class T>
    T GetPointer(const int offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
    // To get value from the pointer itself
    template<class T>
    T GetValue(const int offset)
    {
        return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

	template <typename T>
	T& SetValue(uintptr_t offset)
	{
		return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset);
	}

public:

	C_AnimState *AnimState()
	{
		return *(C_AnimState * *)((DWORD)this + 0x3900);
	}

	float* poseparameterptr()
	{
		DWORD m_flPoseParameter = g_pNetvars->GetOffset("DT_CSPlayer", "m_flPoseParameter");
		return (float*)((uintptr_t)this + m_flPoseParameter);
	}

	AnimationLayer *AnimOverlays()
	{
		return *reinterpret_cast<AnimationLayer**>(uintptr_t(this) + 0x2980);
	}
	AnimationLayer& get_anim_overlay(int i)
	{
		return (*(AnimationLayer * *)((DWORD)this + 0x2970))[i];
	}
	int NumOverlays()
	{
		return 15;
	}

	void UpdateClientAnimation()
	{
		Utils::GetVFunc<void(__thiscall*)(void*)>(this, 219)(this);
	}
	int sequence_activity(C_BaseEntity* pEntity, int sequence);

	void ClientAnimations(bool value)
	{
		static int m_bClientSideAnimation = g_pNetvars->GetOffset("DT_BaseAnimating", "m_bClientSideAnimation");
		*reinterpret_cast<bool*>(uintptr_t(this) + m_bClientSideAnimation) = value;
	}
	int GetSequence()
	{
		static int m_nSequence = g_pNetvars->GetOffset("DT_BaseAnimating", "m_nSequence");
		return GetValue<int>(m_nSequence);
	}
	int duck_speed()
	{
		static int m_flDuckSpeed = g_pNetvars->GetOffset("DT_BasePlayer", "m_flDuckSpeed");
		return GetValue<int>(m_flDuckSpeed);
	}
	int duck_amount()
	{
		static int m_flDuckAmount = g_pNetvars->GetOffset("DT_BasePlayer", "m_flDuckAmount");
		return GetValue<int>(m_flDuckAmount);
	}
	void set_m_bUseCustomBloomScale(byte value)
	{
		static int m_bUseCustomBloomScale = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_bUseCustomBloomScale");
		*reinterpret_cast<byte*>(uintptr_t(this) + m_bUseCustomBloomScale) = value;
	}

	void set_m_bUseCustomAutoExposureMin(byte value)
	{
		static int m_bUseCustomAutoExposureMin = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_bUseCustomAutoExposureMin");
		*reinterpret_cast<byte*>(uintptr_t(this) + m_bUseCustomAutoExposureMin) = value;
	}

	void set_m_bUseCustomAutoExposureMax(byte value)
	{
		static int m_bUseCustomAutoExposureMax = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_bUseCustomAutoExposureMax");
		*reinterpret_cast<byte*>(uintptr_t(this) + m_bUseCustomAutoExposureMax) = value;
	}

	void set_m_flCustomBloomScale(float value)
	{
		static int m_flCustomBloomScale = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_flCustomBloomScale");
		*reinterpret_cast<float*>(uintptr_t(this) + m_flCustomBloomScale) = value;
	}

	void set_m_flCustomAutoExposureMin(float value)
	{
		static int m_flCustomAutoExposureMin = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_flCustomAutoExposureMin");
		*reinterpret_cast<float*>(uintptr_t(this) + m_flCustomAutoExposureMin) = value;
	}

	void set_m_flCustomAutoExposureMax(float value)
	{
		static int m_flCustomAutoExposureMax = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_flCustomAutoExposureMax");
		*reinterpret_cast<float*>(uintptr_t(this) + m_flCustomAutoExposureMax) = value;
	}
	Vector& abs_angles() {
		using original_fn = Vector & (__thiscall*)(void*);
		return (*(original_fn * *)this)[11](this);
	}
	void set_angles(Vector angles) {
		using original_fn = void(__thiscall*)(void*, const Vector&);
		static original_fn set_angles_fn = (original_fn)((DWORD)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1"));
		set_angles_fn(this, angles);
	}
	void set_yaw(Vector angles) {
		using original_fn = void(__thiscall*)(void*, const Vector&);
		static original_fn set_angles_fn = (original_fn)((DWORD)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1"));
		set_angles_fn(this, angles);
	}
	void set_duck_amount(int amount)
	{
		static int m_flDuckAmount = g_pNetvars->GetOffset("DT_BasePlayer", "m_flDuckAmount");
		*reinterpret_cast<int*>(uintptr_t(this) + m_flDuckAmount) = amount;
	}
	Vector punchangle() {
		return *reinterpret_cast<Vector*>(uintptr_t(this) + 0x302C);
	}
	void set_position(Vector position) {
		using original_fn = void(__thiscall*)(void*, const Vector&);
		static original_fn set_position_fn = (original_fn)((DWORD)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));
		set_position_fn(this, position);
	}
	void flash_alpha(float value)
	{
		static int m_flFlashMaxAlpha = g_pNetvars->GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");
		*reinterpret_cast<float*>(uintptr_t(this) + m_flFlashMaxAlpha) = value;
	}
	void SetSequence(int Sequence)
	{
		static int m_nSequence = g_pNetvars->GetOffset("DT_BaseAnimating", "m_nSequence");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nSequence) = Sequence;
	}
	int m_hViewModel()
	{
		static int m_hViewModel = g_pNetvars->GetOffset("DT_BasePlayer", "m_hViewModel[0]");
		return GetValue<int>(m_hViewModel);
	}
	int GetNumAnimOverlays() {
		return *(int*)((DWORD)this + 0x298C);
	}
	void SimulatedEveryTick(bool value)
	{
		static int m_bSimulatedEveryTick = g_pNetvars->GetOffset("DT_BaseEntity", "m_bSimulatedEveryTick");
		*reinterpret_cast<bool*>(uintptr_t(this) + m_bSimulatedEveryTick) = value;
	}

	void SetAbsAngles(Vector angles);
	void SetAbsOrigin(Vector origin);

	Vector GetAbsOrigin()
	{
		if (this != nullptr) {
			return Utils::GetVFunc<Vector& (__thiscall*)(void*)>(this, 10)(this);
		}
	}

	void SetAbsVelocity(Vector velocity);

    C_BaseCombatWeapon* GetActiveWeapon()
    {
		if (this) {
			static int m_hActiveWeapon = g_pNetvars->GetOffset("DT_BaseCombatCharacter", "m_hActiveWeapon");
			const auto weaponData = GetValue<CBaseHandle>(m_hActiveWeapon);
			return reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntityFromHandle(weaponData));
		}
    }
	UINT* m_hMyWeapons()
	{
		if (!this || nullptr) return 0;
		auto m_hMyWeapons = g_pNetvars->GetOffset("DT_BasePlayer", "m_hMyWeapons");
		return (UINT*)((DWORD)this + m_hMyWeapons);
	}
	float get_max_desync_delta() {
		auto animstate = uintptr_t(this->AnimState());
		float rate = 180;
		float duckammount = *(float*)(animstate + 0xA4);
		float speedfraction = max(0, min(*reinterpret_cast<float*>(animstate + 0xF8), 1));
		float speedfactor = max(0, min(1, *reinterpret_cast<float*> (animstate + 0xFC)));
		float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
		float unk2 = unk1 + 1.f;
		float unk3;
		if (duckammount > 0) 
			unk2 += ((duckammount * speedfactor) * (0.5f - unk2));

		unk3 = *(float*)(animstate + 0x334) * unk2;
		return unk3;
	}
	float get_fixed_feet_yaw() {
		float current_feet_yaw = *(float*)(this + 0x80);
		if (current_feet_yaw >= -360)
			current_feet_yaw = min(current_feet_yaw, 360.f);
		return current_feet_yaw;
	}

    int GetTeam()
    {
		if (this) {
			static int m_iTeamNum = g_pNetvars->GetOffset("DT_BaseEntity", "m_iTeamNum");
			return GetValue<int>(m_iTeamNum);
		}
    }

    EntityFlags GetFlags()
    {
        static int m_fFlags = g_pNetvars->GetOffset("DT_BasePlayer", "m_fFlags");
        return GetValue<EntityFlags>(m_fFlags);
    }

	void SetFlags(int offset)
	{
		static int m_fFlags = g_pNetvars->GetOffset("DT_BasePlayer", "m_fFlags");
		*reinterpret_cast<int*>(uintptr_t(this) + m_fFlags) = offset;
	}

	uintptr_t observer_target()
	{
		static int m_hObserverTarget = g_pNetvars->GetOffset("DT_BasePlayer", "m_hObserverTarget");
		return GetValue<uintptr_t>(m_hObserverTarget);
	}

    MoveType_t GetMoveType()
    {
        static int m_Movetype = g_pNetvars->GetOffset("DT_BaseEntity", "m_nRenderMode") + 1;
        return GetValue<MoveType_t>(m_Movetype);
    }

	float GetSimulationTime()
	{
		static int m_flSimulationTime = g_pNetvars->GetOffset("DT_BaseEntity", "m_flSimulationTime");
		return GetValue<float>(m_flSimulationTime);
	}

	float GetOldSimulationTime()
	{
		static int m_flOldSimulationTime = g_pNetvars->GetOffset("DT_BaseEntity", "m_flSimulationTime") + 4;
		return GetValue<float>(m_flOldSimulationTime);
	}

	float GetLowerBodyYaw()
	{
		static int m_flLowerBodyYawTarget = g_pNetvars->GetOffset("DT_CSPlayer", "m_flLowerBodyYawTarget");
		return GetValue<float>(m_flLowerBodyYawTarget);
	}

	void SetLowerBodyYaw(float value)
	{
		static int m_flLowerBodyYawTarget = g_pNetvars->GetOffset("DT_CSPlayer", "m_flLowerBodyYawTarget");
		*reinterpret_cast<float*>(uintptr_t(this) + m_flLowerBodyYawTarget) = value;
	}

	bool defusing()
	{
		static int m_bIsDefusing = g_pNetvars->GetOffset("DT_CSPlayer", "m_bIsDefusing");
		return GetValue<bool>(m_bIsDefusing);
	}
	bool has_defuser()
	{
		static int m_bHasDefuser = g_pNetvars->GetOffset("DT_CSPlayer", "m_bHasDefuser");
		return GetValue<bool>(m_bHasDefuser);
	}
	
	bool GetHeavyArmor()
	{
		static int m_bHasHeavyArmor = g_pNetvars->GetOffset("DT_CSPlayer", "m_bHasHeavyArmor");
		return GetValue<bool>(m_bHasHeavyArmor);
	}

	int ArmorValue()
	{
		static int m_ArmorValue = g_pNetvars->GetOffset("DT_CSPlayer", "m_ArmorValue");
		return GetValue<int>(m_ArmorValue);
	}

	bool HasHelmet()
	{
		static int m_bHasHelmet = g_pNetvars->GetOffset("DT_CSPlayer", "m_bHasHelmet");
		return GetValue<bool>(m_bHasHelmet);
	}

    bool GetLifeState()
    {
        static int m_lifeState = g_pNetvars->GetOffset("DT_BasePlayer", "m_lifeState");
        return GetValue<bool>(m_lifeState);
    }

	bool IsScoped()
	{
		static int m_bIsScoped = g_pNetvars->GetOffset("DT_CSPlayer", "m_bIsScoped");
		return GetValue<bool>(m_bIsScoped);
	}

    int GetHealth()
    {
        static int m_iHealth = g_pNetvars->GetOffset("DT_BasePlayer", "m_iHealth");
        return GetValue<int>(m_iHealth);
    }

	bool IsNade();
	bool IsKnife();

    bool IsAlive() { return this->GetHealth() > 0 && this->GetLifeState() == 0; }

	bool IsEnemy();

    bool IsImmune()
    {
        static int m_bGunGameImmunity = g_pNetvars->GetOffset("DT_CSPlayer", "m_bGunGameImmunity");
        return GetValue<bool>(m_bGunGameImmunity);
    }

    int GetTickBase()
    {
        static int m_nTickBase = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_nTickBase");
        return GetValue<int>(m_nTickBase);
    }

	int GetShotsFired()
	{
		static int m_iShotsFired = g_pNetvars->GetOffset("DT_CSPlayer", "cslocaldata", "m_iShotsFired");
		return GetValue<int>(m_iShotsFired);
	}

	void SetTickBase(int TickBase)
	{
		static int m_nTickBase = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_nTickBase");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nTickBase) = TickBase;
	}

	Vector GetEyeAngles()
	{
		static int m_angEyeAngles = g_pNetvars->GetOffset("DT_CSPlayer", "m_angEyeAngles");
		return GetValue<Vector>(m_angEyeAngles);
	}

	void SetEyeAngles(Vector Angle)
	{
		static int m_angEyeAngles = g_pNetvars->GetOffset("DT_CSPlayer", "m_angEyeAngles");
		*reinterpret_cast<Vector*>(uintptr_t(this) + m_angEyeAngles) = Angle;
	}

    Vector GetOrigin()
    {
        static int m_vecOrigin = g_pNetvars->GetOffset("DT_BaseEntity", "m_vecOrigin");
        return GetValue<Vector>(m_vecOrigin);
    }

	Vector GetOldOrigin()
	{
		static int m_vecOldOrigin = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_flFriction") + 12;
		return GetValue<Vector>(m_vecOldOrigin);
	}

	Vector GetNetworkOrigin()
	{
		static int m_vecNetworkOrigin = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_flFriction") + 40;
		return GetValue<Vector>(m_vecNetworkOrigin);
	}

	void SetOrigin(Vector Origin)
	{
		static int m_vecOrigin = g_pNetvars->GetOffset("DT_BaseEntity", "m_vecOrigin");
		*reinterpret_cast<Vector*>(uintptr_t(this) + m_vecOrigin) = Origin;
	}

	Vector GetVelocity()
	{
		static int m_vecVelocity = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_vecVelocity[0]");
		return GetValue<Vector>(m_vecVelocity);
	}

	void SetVelocity(Vector velocity)
	{
		static int m_vecVelocity = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_vecVelocity[0]");
		*reinterpret_cast<Vector*>(uintptr_t(this) + m_vecVelocity) = velocity;
	}
	Vector GetAimPunchAngle()
	{
		return *reinterpret_cast<Vector*>(uintptr_t(this) + uintptr_t(0x302C));
	}

	Vector GetViewPunchAngle()
	{
		return *reinterpret_cast<Vector*>(uintptr_t(this) + uintptr_t(0x3020));
	}

    Vector GetEyePosition() 
	{
		Vector ret;
		typedef void(__thiscall *OrigFn)(void *, Vector&);
		Utils::GetVFunc<OrigFn>(this, 279)(this, ret);
		return ret;
	}

	ICollideable* GetCollideable()
	{
		return (ICollideable*)((DWORD)this + 0x318);
	}

	void SetCurrentCommand(CUserCmd *cmd)
	{
		static int m_hConstraintEntity = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_hConstraintEntity");
		*reinterpret_cast<CUserCmd**>(uintptr_t(this) + m_hConstraintEntity - 0xC) = cmd;
	}
	float FireRate();
	bool scoped_wep();
	void FixSetupBones(matrix3x4_t *Matrix);
	Vector GetHitboxPosition(int Hitbox, matrix3x4_t *Matrix, float *Radius);
	Vector GetHitboxPosition(int Hitbox, matrix3x4_t *Matrix);
	Vector get_hitbox_pos(C_BaseEntity* m_entity, int Hitbox);
};

class c_env_tonemap_controller : public C_BaseEntity {
public:
	/*	
	NETVAR(unsigned char, use_custom_exposure_min, "DT_EnvTonemapController", "m_bUseCustomAutoExposureMin");
	NETVAR(unsigned char, use_custom_exposure_max, "DT_EnvTonemapController", "m_bUseCustomAutoExposureMax");
	NETVAR(float, custom_exposure_min, "DT_EnvTonemapController", "m_flCustomAutoExposureMin");
	NETVAR(float, custom_exposure_max, "DT_EnvTonemapController", "m_flCustomAutoExposureMax");
	*/

	void use_custom_exposure_min(unsigned char value)
	{
		static int m_bUseCustomAutoExposureMin = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_bUseCustomAutoExposureMin");
		*reinterpret_cast<unsigned char*>(uintptr_t(this) + m_bUseCustomAutoExposureMin) = value;
	}
	void use_custom_exposure_max(unsigned char value)
	{
		static int m_bUseCustomAutoExposureMax = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_bUseCustomAutoExposureMax");
		*reinterpret_cast<unsigned char*>(uintptr_t(this) + m_bUseCustomAutoExposureMax) = value;
	}

	void custom_exposure_min(float value)
	{
		static int m_flCustomAutoExposureMin = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_flCustomAutoExposureMin");
		*reinterpret_cast<float*>(uintptr_t(this) + m_flCustomAutoExposureMin) = value;
	}
	void custom_exposure_max(float value)
	{
		static int m_bUseCustomAutoExposureMax = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_flCustomAutoExposureMax");
		*reinterpret_cast<float*>(uintptr_t(this) + m_bUseCustomAutoExposureMax) = value;
	}
};

class ccsbomb : public C_BaseEntity {
private:
	// To get value from the pointer itself
	template<class T>
	T GetValue(const int offset)
	{
		return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
	}
public:
	float c4_defuse_count_down()
	{
		static int m_flDefuseCountDown = g_pNetvars->GetOffset("DT_PlantedC4", "m_flDefuseCountDown");
		return GetValue<float>(m_flDefuseCountDown);
	}
	
	float defuse_length()
	{
		static int m_flDefuseLength = g_pNetvars->GetOffset("DT_PlantedC4", "m_flDefuseLength");
		return GetValue<float>(m_flDefuseLength);
	}	
	float c4_blow_time()
	{
		static int m_flC4Blow = g_pNetvars->GetOffset("DT_PlantedC4", "m_flC4Blow");
		return GetValue<float>(m_flC4Blow);
	}
	float bomb_defused()
	{
		static int m_bBombDefused = g_pNetvars->GetOffset("DT_PlantedC4", "m_bBombDefused");
		return GetValue<float>(m_bBombDefused);
	}
};
class C_BaseCombatWeapon : public C_BaseEntity
{
private:
    template<class T>
    T GetPointer(const int offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
    // To get value from the pointer itself
    template<class T>
    T GetValue(const int offset)
    {
        return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

public:

    ItemDefinitionIndex GetItemDefinitionIndex()
    {
        static int m_iItemDefinitionIndex = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_AttributeManager", "m_Item", "m_iItemDefinitionIndex");
        return GetValue<ItemDefinitionIndex>(m_iItemDefinitionIndex);
    }
	void SetItemDefinitionIndex(ItemDefinitionIndex value)
	{
		static int m_iItemDefinitionIndex = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_AttributeManager", "m_Item", "m_iItemDefinitionIndex");
		*reinterpret_cast<ItemDefinitionIndex*>(uintptr_t(this) + m_iItemDefinitionIndex) = value;
	}
	void view_model_index(int value)
	{
		static int m_iViewModelIndex = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iViewModelIndex");
		*reinterpret_cast<int*>(uintptr_t(this) + m_iViewModelIndex) = value;
	}


	void model_index(int value)
	{
		static int m_nModelIndex = g_pNetvars->GetOffset("DT_BaseViewModel", "m_nModelIndex");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nModelIndex) = value;
	}


	void fallback_paint_kit(int value)
	{
		static int m_nFallbackPaintKit = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_nFallbackPaintKit");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nFallbackPaintKit) = value;
	}


	void original_owner_xuid_low(int value)
	{
		static int m_OriginalOwnerXuidLow = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
		*reinterpret_cast<int*>(uintptr_t(this) + m_OriginalOwnerXuidLow) = value;
	}


	void original_owner_xuid_high(int value)
	{
		static int m_OriginalOwnerXuidHigh = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh");
		*reinterpret_cast<int*>(uintptr_t(this) + m_OriginalOwnerXuidHigh) = value;
	}


	void fallback_seed(int value)
	{
		static int m_nFallbackSeed = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_nFallbackSeed");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nFallbackSeed) = value;
	}

	void baseview_model_index(int value)
	{
		static int m_nModelIndex = g_pNetvars->GetOffset("DT_BaseViewModel", "m_nModelIndex");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nModelIndex) = value;
	}

	void item_id_high(int value)
	{
		static int m_iItemIDHigh = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_AttributeManager", "m_Item", "m_iItemIDHigh");
		*reinterpret_cast<int*>(uintptr_t(this) + m_iItemIDHigh) = value;
	}

	float GetInaccuracy()
	{
		typedef float(__thiscall* oInaccuracy)(PVOID);
		return Utils::GetVFunc< oInaccuracy >(this, 476)(this);
	}

	float GetSpread()
	{
		typedef float(__thiscall* oWeaponSpread)(PVOID);
		return Utils::GetVFunc< oWeaponSpread >(this, 446)(this);
	}

	float GetAccuracyPenalty()
	{
		static int m_fAccuracyPenalty = g_pNetvars->GetOffset("DT_WeaponCSBase", "m_fAccuracyPenalty");
		return GetValue<float>(m_fAccuracyPenalty);
	}

	float GetLastShotTime()
	{
		static int m_fLastShotTime = g_pNetvars->GetOffset("DT_WeaponCSBase", "m_fLastShotTime");
		return GetValue<float>(m_fLastShotTime);
	}

	void AccuracyPenalty()
	{
		typedef void(__thiscall *OrigFn)(void *);
		return Utils::GetVFunc<OrigFn>(this, 472)(this);
	}

	short* ItemDefinitionIndexz() {
		auto m_iItemDefinitionIndex = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_iItemDefinitionIndex");
		return (short*)((DWORD)this + m_iItemDefinitionIndex);
	}

	void ViewModelIndex(int value) {
		auto m_iViewModelIndex = g_pNetvars->GetOffset("DT_BaseViewModel", "m_nViewModelIndex");
		*reinterpret_cast<int*>(uintptr_t(this) + m_iViewModelIndex) = value;
	}
	void WorldModelIndex(int value) {
		auto m_iWorldModelIndex = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iWorldModelIndex");
		*reinterpret_cast<int*>(uintptr_t(this) + m_iWorldModelIndex) = value;
	}

	void ModelIndex(int value) {
		auto m_nModelIndex = g_pNetvars->GetOffset("DT_BaseViewModel", "m_nModelIndex");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nModelIndex) = value;
	}
	int	GetModelIndex() {
		auto m_nModelIndex = g_pNetvars->GetOffset("DT_BaseViewModel", "m_nModelIndex");
		return *(int*)((uintptr_t)this + m_nModelIndex);
	}
#define VirtualFn( cast ) typedef cast( __thiscall* OriginalFn )
	void SetModelIndex(int nModelIndex) {
		VirtualFn(void)(PVOID, int);
		Utils::GetVFunc< OriginalFn >(this, 75)(this, nModelIndex);
	}
	int* GetEntityQuality() {
		auto m_iEntityQuality = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_iEntityQuality");
		return (int*)((uintptr_t)this + m_iEntityQuality);
	}
	int* OwnerXuidLow() {
		auto m_OriginalOwnerXuidLow = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
		return (int*)((uintptr_t)this +m_OriginalOwnerXuidLow);
	}
	int* OwnerXuidHigh() {
		auto m_OriginalOwnerXuidHigh = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh");
		return (int*)((uintptr_t)this + m_OriginalOwnerXuidHigh);
	}
	void ItemIDHigh(int value) {
		auto m_iItemIDHigh = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_iItemIDHigh");
		*reinterpret_cast<int*>(uintptr_t(this) + m_iItemIDHigh) = value;
	}
	int* FallbackPaintKit() {
		auto m_nFallbackPaintKit = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_nFallbackPaintKit");
		return (int*)((uintptr_t)this + m_nFallbackPaintKit);
	}
	int* FallbackSeed() {
		auto m_nFallbackSeed = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_nFallbackSeed");
		return (int*)((uintptr_t)this + m_nFallbackSeed);
	}
	void FallbackWear(float value) {
		auto m_flFallbackWear = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_flFallbackWear");
		*reinterpret_cast<int*>(uintptr_t(this) + m_flFallbackWear) = value;
	}


    float GetNextPrimaryAttack()
    {
        static int m_flNextPrimaryAttack = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "LocalActiveWeaponData", "m_flNextPrimaryAttack");
        return GetValue<float>(m_flNextPrimaryAttack);
    }

    int GetAmmo()
    {
        static int m_iClip1 = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iClip1");
        return GetValue<int>(m_iClip1);
    }
	int m_iPrimaryReserveAmmoCount()
	{
		static int primary_reserve = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iPrimaryReserveAmmoCount");
		return GetValue<int>(primary_reserve);
	}

    WeaponInfo_t* GetCSWpnData()
    {
		if (!this) return nullptr;
        return Utils::CallVFunc<454, WeaponInfo_t*>(this);
    }
	weapon_info_t* GetCSWpnData_2()
	{
		if (!this) return nullptr;
		return Utils::CallVFunc<454, weapon_info_t*>(this);
	}
	
    std::string GetName()
    {
        ///TODO: Test if szWeaponName returns proper value for m4a4 / m4a1-s or it doesnt recognize them.
        return std::string(this->GetCSWpnData()->weapon_name);
    }
};