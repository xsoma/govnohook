#include <props/player.hpp>
#include <props/displacement.hpp>
#include "source.hpp"
#include <props/prop_manager.hpp>
#include <props/weapon.hpp>
#include <ragebot/resolver.hpp>
#include <ragebot/prediction.hpp>
#include <sdk/trace/ray_tracer.hpp>

unsigned int C_BasePlayer::FindInDataMap(datamap_t* pMap, const char* name)
{
	//FUNCTION_GUARD;

	while (pMap)
	{
		for (int i = 0; i < pMap->dataNumFields; i++)
		{
			if (pMap->dataDesc[i].fieldName == nullptr)
				continue;

			if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
				return pMap->dataDesc[i].fieldOffset;

			if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED)
			{
				if (pMap->dataDesc[i].td)
				{
					unsigned int offset;

					if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
						return offset;
				}
			}
		}
		pMap = pMap->baseMap;
	}

	return 0;
}

int& C_BasePlayer::m_iCrosshairID()
{
	/*"name": "m_iCrosshairId",
		"prop" : "m_bHasDefuser",
		"offset" : 92,
		"table" : "DT_CSPlayer"*/

	static int offset = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bHasDefuser");

	return *(int*)(uintptr_t(this) + offset + 92);
}

typedescription_t* C_BasePlayer::get_datamap_entry(datamap_t* pMap, const char* name)
{
	//FUNCTION_GUARD;

	while (pMap)
	{
		for (int i = 0; i < pMap->dataNumFields; i++)
		{
			if (pMap->dataDesc[i].fieldName == nullptr)
				continue;

			if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
				return &pMap->dataDesc[i];
		}
		pMap = pMap->baseMap;
	}

	return 0;
}

datamap_t* C_BasePlayer::GetPredDescMap()
{
	typedef datamap_t* (__thiscall * o_GetPredDescMap)(void*);
	return Memory::VCall<o_GetPredDescMap>(this, 17)(this);
}

QAngle& C_BasePlayer::m_aimPunchAngle()
{
	return *( QAngle* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_aimPunchAngle );
}

QAngle C_BasePlayer::m_aimPunchAngleScaled()
{
	static auto weapon_recoil_scale = csgo.m_engine_cvars()->FindVar(sxor("weapon_recoil_scale"));
	const auto m_aim_punch_angle = m_aimPunchAngle();

	return m_aim_punch_angle * weapon_recoil_scale->GetFloat();
}

QAngle& C_BasePlayer::m_viewPunchAngle()
{
	return *( QAngle* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_viewPunchAngle );
}

Vector& C_BasePlayer::m_vecViewOffset()
{
	return *( Vector* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_vecViewOffset );
}

Vector& C_BasePlayer::m_vecVelocity()
{
	return *( Vector* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_vecVelocity );
}

Vector& C_BasePlayer::m_vecBaseVelocity()
{
	return *( Vector* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_vecBaseVelocity );
}

float& C_BasePlayer::m_flFallVelocity()
{
	return *( float* )(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_flFallVelocity );
}

char& C_BasePlayer::m_lifeState()
{
	return *(char*)(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_lifeState);
}

int& C_BasePlayer::m_nTickBase()
{
	return *(int*)(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_nTickBase);
}

int& C_BasePlayer::m_nFinalPredictedTick()
{
	return *(int*)(uintptr_t(this) + (Engine::Displacement::DT_BasePlayer::m_nTickBase + 0x4));
}

int& C_BasePlayer::m_vphysicsCollisionState() {
	static auto m_vphysicsCollisionState = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_vphysicsCollisionState"));
	return *(int*)(uintptr_t(this) + m_vphysicsCollisionState);
}

int& C_BasePlayer::m_iHealth()
{
	return *(int*)(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_iHealth);
}

int& C_BasePlayer::m_fFlags()
{
	return *(int*)(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_fFlags);
}

int& C_BasePlayer::m_iPlayerState()
{
	static auto m_iPlayerState = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_iPlayerState"));
	return *(int*)(uintptr_t(this) + m_iPlayerState);
}

bool& C_BasePlayer::m_bIsLocalPlayer()
{
	static auto m_bIsLocalPlayer = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_bIsLocalPlayer"));
	return *(bool*)(uintptr_t(this) + m_bIsLocalPlayer);
}

bool& C_BasePlayer::m_bSpotted()
{
	static auto m_bSpotted = Engine::PropManager::Instance()->GetOffset(sxor("DT_BaseEntity"), sxor("m_bSpotted"));
	return *(bool*)(uintptr_t(this) + m_bSpotted);
}

float& C_BasePlayer::m_flThirdpersonRecoil()
{
	static auto m_flThirdpersonRecoil = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_flThirdpersonRecoil"));
	return *(float *)(uintptr_t(this) + m_flThirdpersonRecoil);
}

bool& C_BasePlayer::m_bIsScoped()
{
	return *(bool*)(uintptr_t(this) + Engine::Displacement::DT_CSPlayer::m_bIsScoped);
}

bool& C_BasePlayer::m_bIsRagdoll()
{
	return *(bool*)(uintptr_t(this) + 0x9F4);
}

bool C_BasePlayer::IsDead()
{
	if (this == nullptr || *(void**)this == nullptr)
		return true;

	return (m_lifeState());
}

matrix3x4_t& C_BasePlayer::GetCollisionBoundTrans()
{
	return m_rgflCoordinateFrame();
}

float& C_BasePlayer::m_flLowerBodyYawTarget()
{
	return *(float*)(uintptr_t(this) + Engine::Displacement::DT_CSPlayer::m_flLowerBodyYawTarget);
}

int C_BasePlayer::GetvphysicsCollisionState()
{
	return *(int*)((DWORD)this + Engine::Displacement::DT_CSPlayer::m_vphysicsCollisionState);
}

void C_BasePlayer::SetvphysicsCollisionState(int state)
{
	*(int*)((DWORD)this + Engine::Displacement::DT_CSPlayer::m_vphysicsCollisionState) = state;
}
CCSGOPlayerAnimState* C_BasePlayer::get_animation_state()
{
	if (this == nullptr) return nullptr;

	return *reinterpret_cast<CCSGOPlayerAnimState**>(uintptr_t(this) + (Engine::Displacement::DT_CSPlayer::m_bIsScoped - 0x14));
}
CCSGOPlayerAnimStateReadble* C_BasePlayer::GetAnimState()
{
	if (this == nullptr) return nullptr;

	return *reinterpret_cast<CCSGOPlayerAnimStateReadble**>(uintptr_t(this) + (Engine::Displacement::DT_CSPlayer::m_bIsScoped - 0x14));
}
void C_BasePlayer::attachment_helper()
{
	static auto DoAttachments = reinterpret_cast <void(__thiscall*)(C_BasePlayer*, CStudioHdr*)> (Memory::Scan("client.dll", "55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4"));
	DoAttachments(this, this->GetModelPtr());
}
void C_BasePlayer::SetCurrentCommand(CUserCmd* cmd)
{
	*(CUserCmd** )(uintptr_t(this) + Engine::Displacement::C_BasePlayer::m_pCurrentCommand ) = cmd;
	//*( CUserCmd* )(uintptr_t(this) + (Engine::Displacement::C_BasePlayer::m_pCurrentCommand - 0xB0	)) = (cmd2 != nullptr ? *cmd2 : *cmd);
}

Vector& C_BasePlayer::get_abs_origin()
{
	return Memory::VCall<Vector & (__thiscall*)(void*)>(this, GET_ABS_ORIGIN)(this);
}

bool C_BasePlayer::is_local_player()
{
	return Memory::VCall<bool(__thiscall*)(void*)>(this, IS_LOCAL_PLAYER)(this);
}

Vector& C_BasePlayer::m_vecAbsOrigin()
{
	return *(Vector*)(uintptr_t(this) + 0xA0);
}

QAngle& C_BasePlayer::get_abs_angles()
{
	return Memory::VCall<QAngle & (__thiscall*)(void*)>(this, GET_ABS_ANGLES)(this);
}

float &C_BasePlayer::get_player_max_speed()
{
	return Memory::VCall<float& (__thiscall*)(void*)>(this, GET_MAX_SPEED)(this);//Memory::VCall<float& (__thiscall*)(void*)>(this, GET_ABS_ANGLES)(this);
}

float& C_BasePlayer::m_flSimulationTime()
{
	return *(float*)(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_flSimulationTime);
}

float& C_BasePlayer::m_flAnimTime()
{
	return *(float*)(uintptr_t(this) + Engine::Displacement::DT_BasePlayer::m_flSimulationTime - 8);
}

void C_BasePlayer::select_item(const char* name, int sub_type)
{
	static auto select_item_fn = reinterpret_cast <void(__thiscall*)(void*, const char*, int)> (Memory::Scan(sxor("client.dll"), sxor("55 8B EC 56 8B F1 ? ? ? 85 C9 74 71 8B 06")));
	select_item_fn(this, name, sub_type);
}

void C_BasePlayer::interpolate(float time)
{
	static auto interpolate_fn = reinterpret_cast <void(__thiscall*)(void*, float)> (Memory::Scan(sxor("client.dll"), sxor("55 8B EC 83 EC 20 56 8B F1 83 ")));
	interpolate_fn(this, time);
}

CStudioHdr* C_BasePlayer::m_pStudioHdr()
{
	static auto studio_hdr = Memory::Scan(sxor("client.dll"), sxor("8B B7 ?? ?? ?? ?? 89 74 24 20"));
	return *(CStudioHdr**)((uintptr_t)this + *(uintptr_t*)(studio_hdr + 0x2) + 0x4);
}

QAngle& C_BasePlayer::get_render_angles()
{
	static auto deadflag = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("deadflag"));
	return *(QAngle*)(uintptr_t(this) + (deadflag + 0x4));
}

QAngle& C_BasePlayer::get_local_angles()
{
	//static auto deadflag = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("deadflag"));
	return *(QAngle*)(uintptr_t(this) + 0x34);
}

int& C_BasePlayer::m_iObserverMode()
{
	static auto m_iObserverMode = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_iObserverMode"));
	return *(int*)(uintptr_t(this) + m_iObserverMode);
}

int& C_BasePlayer::m_nIsAutoMounting()
{
	static auto m_iObserverMode = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_nIsAutoMounting"));
	return *(int*)(uintptr_t(this) + m_iObserverMode);
}

Vector& C_BasePlayer::GetAutoMoveOrigin()
{
	return *(Vector*)(uintptr_t(this) + 0x3A9C);
}

Vector& C_BasePlayer::GetAutomoveTargetEnd()
{
	static auto m_vecAutomoveTargetEnd = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_vecAutomoveTargetEnd"));
	return *(Vector*)(uintptr_t(this) + m_vecAutomoveTargetEnd);
}

float& C_BasePlayer::GetAutomoveTargetTime()
{
	static auto m_flAutoMoveTargetTime = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_flAutoMoveTargetTime"));
	return *(float*)(uintptr_t(this) + m_flAutoMoveTargetTime);
}

float &C_BasePlayer::GetAutomoveStartTime()
{
	static auto m_flAutoMoveStartTime = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_flAutoMoveStartTime"));
	return *(float*)((uintptr_t)this + m_flAutoMoveStartTime);
}

void* C_BasePlayer::animation_layers_ptr()
{
	return *(void**)(uintptr_t(this) + 0x2980);
}

C_AnimationLayer& C_BasePlayer::animation_layer(int i)
{
	return *(C_AnimationLayer*)(uintptr_t(this) + (0x2980 + int(56 * i)));
}

CUtlVector<C_AnimationLayer>& C_BasePlayer::m_anim_overlay() {
	auto layers_ptr = *(int*)(Engine::Displacement::Signatures[c_signatures::PTR_ANIMATION_LAYER] + 2);
	return *(CUtlVector<C_AnimationLayer>*)(uintptr_t(this) + layers_ptr);
}
C_AnimationLayer* C_BasePlayer::get_animlayers()
{
	return *(C_AnimationLayer**)((DWORD)this + 0x2980);
}
C_AnimationLayer& C_BasePlayer::get_animation_layer(int index)
{
	// ref: CCBaseEntityAnimState::ComputePoseParam_MoveYaw
	// move_x move_y move_yaw
	typedef C_AnimationLayer& (__thiscall * Fn)(void*, int, bool);
	/*static Fn fn = NULL;

	if (!fn)*/
	auto fn = (Fn)Engine::Displacement::Signatures[c_signatures::GET_ANIMATION_LAYER];

	index = Math::clamp(index, 0, get_animation_layers_count());

	return fn(this, index, 1);
}

int C_BasePlayer::get_animation_layers_count()
{
	return *(int*)(uintptr_t(this) + 0x298C);
}

Vector C_BasePlayer::Weapon_ShootPosition()
{
	Vector eyePos;
	eyePos[2] = 0;
	eyePos[1] = 0;
	eyePos[0] = 0;

	/*if (this == ctx.m_local()) {
		memcpy(mx, m_CachedBoneData().Base(), GetBoneCount() * sizeof(matrix3x4_t));
		memcpy(m_CachedBoneData().Base(), ctx.zero_matrix, GetBoneCount() * sizeof(matrix3x4_t));
	}*/

	Memory::VCall<void(__thiscall*)(void*, Vector&)>(this, 284)(this, eyePos); //ayyyyy

	return eyePos;
}

Vector C_BasePlayer::EyePosition()
{
	Vector eyePos;
	eyePos[2] = 0;
	eyePos[1] = 0;
	eyePos[0] = 0;

	/*if (this == ctx.m_local()) {
		memcpy(mx, m_CachedBoneData().Base(), GetBoneCount() * sizeof(matrix3x4_t));
		memcpy(m_CachedBoneData().Base(), ctx.zero_matrix, GetBoneCount() * sizeof(matrix3x4_t));
	}*/

	Memory::VCall<void(__thiscall*)(void*, Vector&)>(this, 168)(this, eyePos); //ayyyyy

	return eyePos;
}

Vector C_BasePlayer::GetEyePosition()
{
	if (this == nullptr)
		return Vector(0, 0, 0);

	Vector eyePos = get_abs_origin() + m_vecViewOffset();//EyePosition();

	if (get_animation_state() && should_fix_modify_eye_pos())
	{
		int bone = LookupBone(sxor("head_0"));

		if (bone != -1)
		{
			Vector vecHeadPos = ctx.m_local()->get_bone_pos(bone);

			auto flHeadHeight = vecHeadPos.z + 1.7f;
			if (eyePos.z > flHeadHeight)
			{
				auto tmp = 0.0f;
				tmp = (fabsf(eyePos.z - flHeadHeight) - 4.0f) * 0.16666667f;
				if (tmp >= 0.0f)
					tmp = fminf(tmp, 1.0f);
				eyePos.z = ((flHeadHeight - eyePos.z)
					* (((tmp * tmp) * 3.0f) - (((tmp * tmp) * 2.0f) * tmp)))
					+ eyePos.z;
			}
		}
	}

	return eyePos;
}

float& C_BasePlayer::m_flCycle()
{
	return *(float*)(uintptr_t(this) + Engine::Displacement::DT_BaseAnimating::m_flCycle);
}

int& C_BasePlayer::m_iEFlags()
{
	static auto m_iEFlags = FindInDataMap(this->GetPredDescMap(), sxor("m_iEFlags"));
	return *(int*)(uintptr_t(this) + m_iEFlags);
}

Vector& C_BasePlayer::m_vecAbsVelocity()
{
	//return *(Vector*)(uintptr_t(this)  + 0x94);
	static auto m_vecAbsVelocity = FindInDataMap(this->GetPredDescMap(), sxor("m_vecAbsVelocity"));
	return *(Vector*)(uintptr_t(this) + m_vecAbsVelocity);
}

float& C_BasePlayer::m_flOldSimulationTime()
{
	return *(float*)(uintptr_t(this)  + Engine::Displacement::DT_BasePlayer::m_flSimulationTime + 4);
}

float C_BasePlayer::m_flSpawnTime()
{
	static auto m_iAddonBits = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_iAddonBits"));
	return *(float*)(uintptr_t(this)  + m_iAddonBits - 4);
}

float& C_BasePlayer::m_flNextAttack()
{
	static auto m_flNextAttack = Engine::PropManager::Instance()->GetOffset(sxor("DT_BaseCombatCharacter"), sxor("m_flNextAttack"));
	return *(float*)(uintptr_t(this)  + m_flNextAttack);
}

float& C_BasePlayer::m_flTimeOfLastInjury()
{
	static auto m_flTimeOfLastInjury = Engine::PropManager::Instance()->GetOffset(sxor("DT_BaseCombatCharacter"), sxor("m_flTimeOfLastInjury"));
	return *(float*)(uintptr_t(this) + m_flTimeOfLastInjury);
}

float& C_BasePlayer::m_flGroundAccelLinearFracLastTime()
{
	static auto m_flGroundAccelLinearFracLastTime = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_flGroundAccelLinearFracLastTime"));
	return *(float*)(uintptr_t(this) + m_flGroundAccelLinearFracLastTime);
}

float& C_BasePlayer::m_flDuckAmount()
{
	return *(float*)(uintptr_t(this) + Engine::Displacement::DT_CSPlayer::m_flDuckAmount);
}

CUtlVector< matrix3x4_t >& C_BasePlayer::m_CachedBoneData() {
	return *(CUtlVector< matrix3x4_t >*)(uintptr_t(this)  + 0x2910);
}//DT_BasePlayer::m_CachedBoneData = reinterpret_cast<std::uintptr_t>(core::find_signature(charenc("client.dll"), charenc("8B BF ? ? ? ? 8D 14 49")) + 2); for using base ent

int& C_BasePlayer::m_bone_count()
{
	return *(int*)(uintptr_t(this) + 0x291C);
}

void C_BasePlayer::force_bone_cache()
{
	static auto bone_counter = *(int**)(uintptr_t(Memory::Scan(sxor("client.dll"), sxor("FF 05 ? ? ? ? FF 05 ? ? ? ? ")) + 2));
	*(int*)(uintptr_t(this) + 0x2690) = *bone_counter; //55 8B EC 53 56 57 8B F9 8B 87 ? ? ? ? 3B 
}

bool& C_BasePlayer::m_bGunGameImmunity()
{
	static auto _m_bGunGameImmunity = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_bGunGameImmunity"));
	return *(bool*)(uintptr_t(this)  + _m_bGunGameImmunity);
}

QAngle& C_BasePlayer::m_aimPunchAngleVel()
{
	static auto m_aimPunchAngleVel = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_aimPunchAngleVel"));
	return *(QAngle*)(uintptr_t(this) + m_aimPunchAngleVel);
}

int& C_BasePlayer::m_hGroundEntity()
{
	static auto m_hGroundEntity = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_hGroundEntity"));
	return *(int*)(uintptr_t(this) + m_hGroundEntity);
}

float& C_BasePlayer::m_flDuckSpeed()
{
	static auto m_flDuckSpeed = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_flDuckSpeed"));
	return *(float*)(uintptr_t(this) + m_flDuckSpeed);
}

Vector C_BasePlayer::get_bone_pos(int iBone)
{
	if (iBone < 0 || iBone > 128 || !m_CachedBoneData().Base())
		return Vector::Zero;

	return Vector(m_CachedBoneData().Base()[iBone][0][3], m_CachedBoneData().Base()[iBone][1][3], m_CachedBoneData().Base()[iBone][2][3]);
}

//////////////////=
//////////////////=
void C_BasePlayer::GetBonePosition(int iBone, Vector& origin, QAngle& angles)
{
	matrix3x4_t bonetoworld = m_CachedBoneData().Base()[iBone];
	//GetBoneTransform(iBone, bonetoworld);

	Math::MatrixAngles(bonetoworld, angles, origin);
}

Vector C_BasePlayer::get_bone_pos(int iBone, matrix3x4_t mx[])
{
	if (iBone < 0 || iBone > 128 || !mx)
		return Vector::Zero;

	return Vector(mx[iBone][0][3], mx[iBone][1][3], mx[iBone][2][3]);
}

void C_BasePlayer::set_model_index(int index)
{
	typedef void(__thiscall * OriginalFn)(PVOID, int);
	return Memory::VCall<OriginalFn>(this, 75)(this, index);
}

int& C_BasePlayer::m_iShotsFired()
{
	static auto m_iShotsFired = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_iShotsFired"));
	return *(int*)(uintptr_t(this)  + m_iShotsFired);
}

bool& C_BasePlayer::m_bDucking()
{
	static auto m_bDucking = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_bDucking"));
	return *(bool*)(uintptr_t(this)  + m_bDucking);
}

bool& C_BasePlayer::m_bDucked()
{
	static auto m_bDucked = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_bDucked"));
	return *(bool*)(uintptr_t(this)  + m_bDucked);
}

float& C_BasePlayer::m_flLastDuckTime()
{
	static auto m_flLastDuckTime = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_flLastDuckTime"));
	return *(float*)(uintptr_t(this)  + m_flLastDuckTime);
}

int& C_BasePlayer::LastBoneSetupFrame() {
	return *(int*)(uintptr_t(this) + 0xA68);
}

void C_BasePlayer::UpdateVisibilityAllEntities()
{
	static uintptr_t* update_visibility_all_entities = nullptr;
	if (update_visibility_all_entities == nullptr) {
		//static auto relative_call = (std::uintptr_t*)(Memory::Scan("client.dll", "E8 ? ? ? ? 83 7D D8 00 7C 0F"));

		//static auto offset = *(uintptr_t*)(relative_call + 0x1);
		//auto update_visibility_all_entities = (uintptr_t*)(relative_call + 5 + offset);

		static DWORD callInstruction = Engine::Displacement::Signatures[c_signatures::UPDATEVISIBILITY_ALLENTS]; // get the instruction address
		static DWORD relativeAddress = *(DWORD*)(callInstruction + 1); // read the rel32
		static DWORD nextInstruction = callInstruction + 5; // get the address of next instruction
		update_visibility_all_entities = (uintptr_t*)(nextInstruction + relativeAddress); // our function address will be nextInstruction + relativeAddress

	}
	else
		reinterpret_cast<void(__thiscall*)(void*)>(update_visibility_all_entities)(this);
}

void C_BasePlayer::force_bone_rebuild()
{
	m_BoneAccessor().m_WritableBones = m_BoneAccessor().m_ReadableBones = 0;
	*(int*)(uintptr_t(this) + 0x2924) = 0xFF7FFFFF;
	*(int*)(uintptr_t(this) + 0x2690) = 0;
}

bool& C_WeaponCSBaseGun::m_bPinPulled()
{
	static auto m_bPinPulled = Engine::PropManager::Instance()->GetOffset("DT_BaseCSGrenade", sxor("m_bPinPulled"));
	return *(bool*)(uintptr_t(this) + m_bPinPulled);
}

float& C_WeaponCSBaseGun::m_fThrowTime()
{
	static auto m_fThrowTime = Engine::PropManager::Instance()->GetOffset("DT_BaseCSGrenade", sxor("m_fThrowTime"));
	return *(float*)(uintptr_t(this) + m_fThrowTime);
}

bool C_WeaponCSBaseGun::IsBeingThrowed()
{
	//static auto last_throwtime = 0.f;

	if (!m_bPinPulled()) {
		float throwTime = m_fThrowTime();

		//last_throwtime = throwTime;

		if (throwTime > 0.f)
			return true;

		return false;
	}
	else
		return true;

	/*if ((cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2)) {
		if (m_fThrowTime() > 0.f)
			return true;
	}*/

	//return false;
}

float& C_BasePlayer::m_flVelocityModifier()
{
	static auto m_flVelocityModifier = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_flVelocityModifier"));
	return *(float*)(uintptr_t(this)  + m_flVelocityModifier);
}

int& C_BasePlayer::m_hViewModel()
{
	static auto m_hViewModel = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_hViewModel[0]");

	return *(int*)(uintptr_t(this) + m_hViewModel);
}

CBaseHandle C_BaseViewModel::get_viewmodel_weapon()
{
	static int offset = Engine::PropManager::Instance()->GetOffset("DT_BaseViewModel", "m_hWeapon");
	return *(CBaseHandle*)(uintptr_t(this) + offset);
}

int C_BaseViewModel::m_nViewModelIndex()
{
	static int offset = Engine::PropManager::Instance()->GetOffset("DT_BaseViewModel", "m_nViewModelIndex");
	return *(int*)(uintptr_t(this) + offset);
}

float& C_BaseViewModel::m_flCycle()
{
	static unsigned int m_flCycle = FindInDataMap(GetPredDescMap(), "m_flCycle");
	return *(float*)(uintptr_t(this)  + m_flCycle);
}

float& C_BaseViewModel::m_flModelAnimTime()
{
	static unsigned int m_flAnimTime = FindInDataMap(GetPredDescMap(), "m_flAnimTime");
	return *(float*)(uintptr_t(this)  + m_flAnimTime);
}

int& C_BaseViewModel::m_nAnimationParity()
{
	static int m_nAnimationParity = Engine::PropManager::Instance()->GetOffset("DT_BaseViewModel", "m_nAnimationParity");
	//static unsigned int m_flAnimTime = FindInDataMap(GetPredDescMap(), "m_flAnimTime");
	return *(int*)(uintptr_t(this) + m_nAnimationParity);
}

CBaseHandle& C_BasePlayer::m_hNetworkMoveParent()
{
	static auto m_hOwnerEntity = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_hOwnerEntity");

	//static unsigned int m_flAnimTime = FindInDataMap(GetPredDescMap(), "m_flAnimTime");
	return *(CBaseHandle*)(uintptr_t(this) + m_hOwnerEntity - sizeof(CBaseHandle));
}

CBaseHandle& C_BasePlayer::m_pMoveParent()
{
	static int moveparent = Engine::PropManager::Instance()->GetOffset("DT_BaseEntity", "moveparent");
	//static unsigned int m_flAnimTime = FindInDataMap(GetPredDescMap(), "m_flAnimTime");
	return *(CBaseHandle*)(uintptr_t(this) + moveparent);
}

int& C_BasePlayer::m_fEffects()
{
	static auto m_fEffects = FindInDataMap(GetPredDescMap(), sxor("m_fEffects"));
	return *(int*)(uintptr_t(this) + m_fEffects);
}

const char* C_BasePlayer::m_szLastPlaceName()
{
	static auto m_szLastPlaceName = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_szLastPlaceName");
	return (const char*)(uintptr_t(this) + m_szLastPlaceName);
}

int& C_BasePlayer::m_iAccount()
{
	static auto m_iAccount = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_iAccount");

	return *(int*)(uintptr_t(this) + m_iAccount);
}

int* C_BasePlayer::m_hMyWeapons()
{
	return (int*)(uintptr_t(this) + Engine::Displacement::DT_BaseCombatCharacter::m_hActiveWeapon - 256);
}

CBaseHandle* C_BasePlayer::m_hMyWearables()
{
	static auto m_hMyWearables = Engine::PropManager::Instance()->GetOffset("DT_BaseCombatCharacter", "m_hMyWearables");

	return (CBaseHandle*)((uintptr_t)this + m_hMyWearables);
}

int& C_BaseViewModel::m_nSequence()
{
	static unsigned int m_nSequence = FindInDataMap(GetPredDescMap(), "m_nSequence");
	return *(int*)(uintptr_t(this)  + m_nSequence);
}

void C_BaseViewModel::send_matching_seq(int seq)
{
	typedef void(__thiscall * fn)(C_BaseViewModel*, int);
	return Memory::VCall<fn>(this, 245)(this, seq);
}

CBaseHandle* C_BasePlayer::m_hObserverTarget()
{
	static auto m_hObserverTarget = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_hObserverTarget");

	return (CBaseHandle*)(uintptr_t(this) + m_hObserverTarget);
}

bool C_BasePlayer::SetupBonesEx(int matrix, matrix3x4a_t* pBoneToWorld) {
	if (IsDormant())
		return false;

	// BuildTransformations Fix (credits: polak)
	static auto r_jiggle_bones = csgo.m_engine_cvars()->FindVar("r_jiggle_bones");

	const auto penis = r_jiggle_bones->GetInt();
	const auto old_origin = get_abs_origin();
	const auto clientsideanim = client_side_animation();

	r_jiggle_bones->SetValue(0);

	float bk = FLT_MAX;

	if (this != ctx.m_local()) {
		set_abs_origin(m_vecOrigin());
	}

	const auto v20 = *(int*)(uintptr_t(this) + 0xA28);
	const auto v19 = *(int*)(uintptr_t(this) + 0xA30);
	const auto v22 = *(uint8_t*)(uintptr_t(this) + 0x68);

	const auto effects = m_fEffects();
	m_fEffects() |= 8u;
	*(int*)(uintptr_t(this) + 0xA68) = 0;
	*(int*)(uintptr_t(this) + 0xA28) &= ~10u;
	*(int*)(uintptr_t(this) + 0xA30) = 0;
	*(unsigned short*)(uintptr_t(this) + 0x68) |= 2; // m_ClientEntEffects // entindex() + 0x4

	auto realtime_backup = csgo.m_globals()->realtime;
	auto curtime = csgo.m_globals()->curtime;
	auto frametime = csgo.m_globals()->frametime;
	auto absoluteframetime = csgo.m_globals()->absoluteframetime;
	auto framecount = csgo.m_globals()->framecount;
	auto tickcount = csgo.m_globals()->tickcount;
	auto interpolation_amount = csgo.m_globals()->interpolation_amount;

	float time = (ctx.m_local() != this ? m_flSimulationTime() : TICKS_TO_TIME(csgo.m_client_state()->m_clockdrift_manager.m_nServerTick));
	int ticks = TIME_TO_TICKS(time);

	csgo.m_globals()->curtime = time;
	csgo.m_globals()->realtime = time;
	csgo.m_globals()->frametime = csgo.m_globals()->interval_per_tick;
	csgo.m_globals()->absoluteframetime = csgo.m_globals()->interval_per_tick;
	csgo.m_globals()->framecount = ticks;
	csgo.m_globals()->tickcount = ticks;
	csgo.m_globals()->interpolation_amount = 0.f;

	const float weight12 = get_animation_layers_count() < 12 ? -999 : animation_layer(12).m_flWeight;
	if (ctx.m_local() != this)
		animation_layer(12).m_flWeight = 0.f;

	//csgo.m_globals()->framecount = -999;

	const auto v26 = *(int*)(uintptr_t(this) + 0x2670);
	*(int*)(uintptr_t(this) + 0x2670) = 0;
	ctx.setup_bones = true;
	client_side_animation() = false;
	const bool result = SetupBones(pBoneToWorld, -1, matrix, time);
	client_side_animation() = clientsideanim;
	ctx.setup_bones = false;
	*(int*)(uintptr_t(this) + 0x2670) = v26;
	*(int*)(uintptr_t(this) + 0xA28) = v20;
	*(int*)(uintptr_t(this) + 0xA30) = v19;
	*(unsigned short*)(uintptr_t(this) + 0x68) = v22;
	m_fEffects() = effects;
	r_jiggle_bones->SetValue(penis);

	if (weight12 > -1)
	animation_layer(12).m_flWeight = weight12;

	csgo.m_globals()->realtime = realtime_backup;
	csgo.m_globals()->curtime = curtime;
	csgo.m_globals()->frametime = frametime;
	csgo.m_globals()->absoluteframetime = absoluteframetime;
	csgo.m_globals()->framecount = framecount;
	csgo.m_globals()->tickcount = tickcount;
	csgo.m_globals()->interpolation_amount = interpolation_amount;

	if (this != ctx.m_local())
		set_abs_origin(old_origin);

	return result;
}

void C_BasePlayer::update_animstate(CCSGOPlayerAnimState* state, QAngle angle)
{
	if (!state || IsDormant() || this == nullptr || *(void**)this == nullptr)
		return;

	if (GetModelPtr() == nullptr /*|| get_weapon() == nullptr*/)
		return;

	angle.y = Math::normalize_angle(angle.y);

	static auto UpdateAnimState = Engine::Displacement::Signatures[c_signatures::UPDATEANIMSTATE];

	if (!UpdateAnimState || state->m_player != this || *(void**)state->m_player == nullptr || csgo.m_client_state()->m_iDeltaTick < 0)
		return;

	const auto frametime = csgo.m_globals()->frametime;
	for (int i = 0; i < m_anim_overlay().Count(); ++i) {
		m_anim_overlay().Element(i).m_pOwner = this;
	}

	if (state->m_last_update_frame >= csgo.m_globals()->framecount)
		state->m_last_update_frame = csgo.m_globals()->framecount - 1;

	if (this == ctx.m_local())
		m_flThirdpersonRecoil() = m_aimPunchAngleScaled().x;

	static auto ret = ((void(__vectorcall*)(void*, void*, float, float, float, void*))UpdateAnimState);

	if (!ret)
		return;

	ctx.updating_anims = true;
	this->client_side_animation() = true;

	ret(state, NULL, NULL, angle.y, angle.x, NULL);

	ctx.updating_anims = false;
	csgo.m_globals()->frametime = frametime;
}

void C_PlayerResource::update()
{
	if (!ctx.init_finished || !csgo.m_engine()->IsInGame() || !csgo.m_game_rules())
		return;

	for (ClientClass* pClass = csgo.m_client()->GetAllClasses(); pClass; pClass = pClass->m_pNext)
	{
		if (!strcmp(pClass->m_pNetworkName, sxor("CPlayerResource")))
		{
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++)
			{
				RecvProp* pProp = &pClassTable->m_pProps[nIndex];

				if (!pProp || strcmp(pProp->m_pVarName, sxor("m_iTeam")) != 0)
					continue;

				this_ptr = *reinterpret_cast<C_PlayerResource***>(DWORD(pProp->m_pDataTable->m_pProps->m_ProxyFn) + 0x10);

				break;
			}
			break;
		}
	}
}

int C_PlayerResource::get_ping(C_BasePlayer* m_player)
{
	if (!this_ptr)
		return -1;

	static auto m_iPing = Engine::PropManager::Instance()->GetOffset("DT_CSPlayerResource", "m_iPing");
	return *(bool*)(uintptr_t(*this_ptr) + m_iPing + m_player->entindex() * 4);
}

int C_PlayerResource::get_c4_carrier()
{
	if (!this_ptr)
		return -1;

	static auto m_iPlayerC4 = Engine::PropManager::Instance()->GetOffset("DT_CSPlayerResource", "m_iPlayerC4");
	return *(bool*)(uintptr_t(*this_ptr) + m_iPlayerC4);
}

int C_PlayerResource::get_hostage_carrier()
{
	if (!this_ptr)
		return -1;

	static auto m_iPlayerVIP = Engine::PropManager::Instance()->GetOffset("DT_CSPlayerResource", "m_iPlayerVIP");
	return *(bool*)(uintptr_t(*this_ptr) + m_iPlayerVIP);
}
void  C_BasePlayer::set_m_bUseCustomBloomScale(byte value)
{
	*reinterpret_cast<byte*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(sxor("CEnvTonemapController"), sxor("m_bUseCustomBloomScale"))) = value;
}

void  C_BasePlayer::set_m_flCustomBloomScale(float value)
{
	*reinterpret_cast<float*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(sxor("CEnvTonemapController"), sxor("m_flCustomBloomScale"))) = value;
}

void  C_BasePlayer::set_m_bUseCustomAutoExposureMin(byte value)
{
	*reinterpret_cast<byte*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(sxor("CEnvTonemapController"), sxor("m_bUseCustomAutoExposureMin"))) = value;
}

void  C_BasePlayer::set_m_flCustomAutoExposureMin(float value)
{
	*reinterpret_cast<float*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(sxor("CEnvTonemapController"), sxor("m_flCustomAutoExposureMin"))) = value;
}

void  C_BasePlayer::set_m_bUseCustomAutoExposureMax(byte value)
{
	*reinterpret_cast<byte*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(sxor("CEnvTonemapController"), sxor("m_bUseCustomAutoExposureMax"))) = value;
}

void  C_BasePlayer::set_m_flCustomAutoExposureMax(float value)
{
	*reinterpret_cast<float*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(sxor("CEnvTonemapController"), sxor("m_flCustomAutoExposureMax"))) = value;
}
void C_BasePlayer::set_abs_angles(QAngle origin)
{
	if (!IsDormant())
		reinterpret_cast<void(__thiscall*)(C_BaseEntity*, const QAngle&)>(Engine::Displacement::Signatures[c_signatures::SETABSANGLES])(this, origin);
}

bool C_BasePlayer::delay_unscope(float& fov)
{
	if (!IsDormant())
		return reinterpret_cast<bool(__thiscall*)(C_BaseEntity*, float&)>(Engine::Displacement::Signatures[c_signatures::DELAYUNSCOPE])(this, fov);

	return false;
}

bool& C_BasePlayer::m_bWaitForNoAttack()
{
	static auto m_bWaitForNoAttack = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_bWaitForNoAttack"));
	return *(bool*)(uintptr_t(this) + m_bWaitForNoAttack);
}

bool& C_BasePlayer::m_bStrafing()
{
	static auto m_bStrafing = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_bStrafing"));
	return *(bool*)(uintptr_t(this)  + m_bStrafing);
}

float& C_BasePlayer::m_flTimeLastTouchedGround()
{
	static auto m_bIsPlayerGhost = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_bIsPlayerGhost"));
	return *(float*)(uintptr_t(this)  + (m_bIsPlayerGhost - 0x2A5));
}

bool& C_BasePlayer::client_side_animation()
{
	return *(bool*)(uintptr_t(this) + Engine::Displacement::DT_BaseAnimating::m_bClientSideAnimation);
}

void C_BasePlayer::pre_data_update(int updateType)
{
	if (!this) return;

	PVOID pNetworkable = (PVOID)(uintptr_t(this) + 0x8);

	if (!pNetworkable)
		return;

	typedef void(__thiscall * OriginalFn)(PVOID, int);
	return Memory::VCall<OriginalFn>(pNetworkable, 6)(pNetworkable, updateType);
}

void C_BasePlayer::on_pre_data_change(int updateType)
{
	if (!this) return;

	PVOID pNetworkable = (PVOID)(uintptr_t(this) + 0x8);

	if (!pNetworkable)
		return;

	typedef void(__thiscall * OriginalFn)(PVOID, int);
	return Memory::VCall<OriginalFn>(pNetworkable, 4)(pNetworkable, updateType);
}

void C_BasePlayer::on_data_changed(int updateType)
{
	if (!this) return;

	PVOID pNetworkable = (PVOID)(uintptr_t(this) + 0x8);

	if (!pNetworkable)
		return;

	typedef void(__thiscall * OriginalFn)(PVOID, int);
	return Memory::VCall<OriginalFn>(pNetworkable, 5)(pNetworkable, updateType);
}

void C_BasePlayer::post_data_update(int updateType)
{
	if (!this) return;

	PVOID pNetworkable = (PVOID)(uintptr_t(this) + 0x8);

	if (!pNetworkable)
		return;

	typedef void(__thiscall * OriginalFn)(PVOID, int);
	return Memory::VCall<OriginalFn>(pNetworkable, 7)(pNetworkable, updateType);
}

void C_BasePlayer::set_abs_origin(Vector origin)
{
	/*Ignoring unreasonable position (%f,%f,%f) from vphysics! (entity %s)\n*/
	reinterpret_cast<void(__thiscall*)(void*, const Vector&)>(Engine::Displacement::Signatures[c_signatures::SETABSORIGIN])(this, origin);
}

bool C_BasePlayer::physics_run_think(int think_type)
{
	return reinterpret_cast<bool(__thiscall*)(void*, int)>(Engine::Displacement::Signatures[c_signatures::PHYSICS_RUN_THINK])(this, think_type);
}

void C_BasePlayer::prediction_unk_func(int unk1)
{
	reinterpret_cast<void(__thiscall*)(void*, int)>(Engine::Displacement::Signatures[c_signatures::SETNEXTTHINK])(this, unk1);
}

void C_BasePlayer::set_local_viewangles(QAngle angles)
{
	typedef void(__thiscall* origfn)(void*, const QAngle&);
	Memory::VCall<origfn>(this, SET_LOCAL_VIEWANGLES)(this, angles);
}

void C_BasePlayer::think()
{
	typedef void(__thiscall* origfn)(void*);
	Memory::VCall<origfn>(this, THINK)(this);
}

void C_BasePlayer::pre_think()
{
	typedef void(__thiscall* origfn)(void*);
	Memory::VCall<origfn>(this, PRE_THINK)(this);
}

int& C_BasePlayer::m_nHitboxSet()
{
	return *(int*)(uintptr_t(this) + Engine::Displacement::DT_BaseAnimating::m_nHitboxSet);
}

std::array<float, 24> & C_BasePlayer::m_flPoseParameter()
{
	return *(std::array<float, 24>*)(uintptr_t(this) + Engine::Displacement::DT_CSPlayer::m_flPoseParameter);
}

int C_BasePlayer::draw_model(int flags, uint8_t alpha)
{
	if (!this) return 0;
	auto renderable = this->GetClientRenderable();
	if (!renderable) return 0;

	typedef int(__thiscall * origfn)(void*, int, uint8_t);
	return Memory::VCall<origfn>(renderable, SET_MODEL)(renderable, flags, alpha);
}

int& C_BasePlayer::m_nSequence()
{
	return *(int*)(uintptr_t(this) + Engine::Displacement::DT_BaseAnimating::m_nSequence);
}

float C_BasePlayer::get_bomb_blow_timer()
{
	if (!this)
		return 0;

	static auto m_flC4Blow = Engine::PropManager::Instance()->GetOffset("DT_PlantedC4", "m_flC4Blow");

	float bombTime = *(float*)(uintptr_t(this) + m_flC4Blow);
	float returnValue = bombTime - csgo.m_globals()->curtime;
	return (returnValue < 0) ? 0.f : returnValue;
}

bool C_BasePlayer::m_bBombTicking()
{
	static int offset = Engine::PropManager::Instance()->GetOffset("DT_PlantedC4", "m_bBombTicking");
	return *(bool*)(uintptr_t(this) + offset);
}

bool C_BasePlayer::m_bHasDefuser()
{
	if (!this || this->m_iTeamNum() != 3) return false;

	static int offset = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bHasDefuser");

	return *(bool*)(uintptr_t(this) + offset);
}

int& C_BasePlayer::m_nExplodeEffectTickBegin()
{
	static int offset = Engine::PropManager::Instance()->GetOffset("DT_BaseCSGrenadeProjectile", "m_nExplodeEffectTickBegin");

	return *(int*)(uintptr_t(this) + offset);
}

void C_BasePlayer::invalidate_anims(int m)
{
	if (this != nullptr && *(void**)this != nullptr)
		((void(__thiscall*)(C_BasePlayer*, int))Engine::Displacement::Signatures[c_signatures::INVALIDATEPHYSICS])(this, m);
}

float& C_BasePlayer::m_flHealthShotBoostExpirationTime()
{
	static int m_flHealthShotBoostExpirationTime = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flHealthShotBoostExpirationTime");

	return *(float*)(uintptr_t(this)  + m_flHealthShotBoostExpirationTime);
}

float& C_BasePlayer::m_flMaxSpeed()
{
	static int m_flMaxSpeed = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_flMaxSpeed"));

	return *(float*)(uintptr_t(this) + m_flMaxSpeed);
}

bool& C_BasePlayer::m_bIsDefusing()
{
	static int m_bIsDefusing = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_bIsDefusing"));

	return *(bool*)(uintptr_t(this)  + m_bIsDefusing);
}

bool& C_BasePlayer::m_bIsWalking()
{
	static int m_bIsWalking = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSPlayer"), sxor("m_bIsWalking"));

	return *(bool*)(uintptr_t(this)  + m_bIsWalking);
}

int& C_BasePlayer::m_iMoveState()
{
	static int m_iMoveState = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_iMoveState");

	return *(int*)(uintptr_t(this)  + m_iMoveState);
}

C_BasePlayer* C_BasePlayer::m_hBombDefuser()
{
	if (!this) return nullptr;

	static int offset = Engine::PropManager::Instance()->GetOffset("DT_PlantedC4", "m_hBombDefuser");

	auto defuser = *(CBaseHandle*)(uintptr_t(this) + offset);

	if (defuser == -1)
		return nullptr;

	return csgo.m_entity_list()->GetClientEntityFromHandle(defuser);
}

CBoneAccessor& C_BasePlayer::GetBoneAccessor()
{
	return *(CBoneAccessor*)(uintptr_t(this) + Engine::Displacement::DT_BaseAnimating::m_nForceBone - 0x1C); //0x2694
}

void C_BasePlayer::set_collision_bounds(const Vector mins, const Vector maxs)
{
	auto _SetCollisionBounds = Engine::Displacement::Signatures[c_signatures::SETCOLLISIONBOUNDS];

	if (_SetCollisionBounds && GetCollideable())
		reinterpret_cast<void(__thiscall*)(ICollideable*, const Vector&, const Vector&)>(_SetCollisionBounds)(GetCollideable(), mins, maxs);
}

int C_BasePlayer::LookupSequence(const char* label)
{
	return reinterpret_cast<int(__thiscall*)(void*, const char*)>(Engine::Displacement::Signatures[c_signatures::LOOKUPSEQUENCE])(this, label);
}

int C_BasePlayer::LookupBone(const char* szName)
{
	return reinterpret_cast<int(__thiscall*)(void*, const char*)>(Engine::Displacement::Signatures[c_signatures::LOOKUPBONE])(this, szName);
}

float C_BasePlayer::get_bomb_defuse_timer()
{
	if (!this)
		return 0.f;

	float returnValue = m_flDefuseCountDown() - csgo.m_globals()->curtime;
	return (returnValue < 0) ? 0.f : returnValue;
}

float& C_BasePlayer::m_flDefuseCountDown()
{
	static int offset = Engine::PropManager::Instance()->GetOffset("DT_PlantedC4", "m_flDefuseCountDown");
	return *(float*)(uintptr_t(this) + offset);
}

int& C_BasePlayer::m_nSkin()
{
	static auto m_nSkin = Engine::PropManager::Instance()->GetOffset("DT_BaseAnimating", "m_nSkin");

	return *(int*)(uintptr_t(this) + m_nSkin);
}

int& C_BasePlayer::m_nBody()
{
	static auto m_nBody = Engine::PropManager::Instance()->GetOffset("DT_BaseAnimating", "m_nBody");

	return *(int*)(uintptr_t(this) + m_nBody);
}

int& C_BasePlayer::m_hOwnerEntity()
{
	static auto m_hOwnerEntity = Engine::PropManager::Instance()->GetOffset("DT_BasePlayer", "m_hOwnerEntity");

	return *(int*)(uintptr_t(this) + m_hOwnerEntity);
}
void IClientLeafSystem::CreateRenderableHandle(void* obj)
{
	typedef void(__thiscall* tOriginal)(void*, int, int, char, signed int, char);
	Memory::VCall<tOriginal>(this, 0)(this, reinterpret_cast<uintptr_t>(obj) + 0x4, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
}
int& C_BasePlayer::m_hOwner()
{
	static auto m_hOwnerEntity = Engine::PropManager::Instance()->GetOffset("DT_BaseViewModel", "m_hOwner");

	return *(int*)(uintptr_t(this) + m_hOwnerEntity);
}
bool& C_BasePlayer::m_bDidSmokeEffect()
{
	static auto m_nSmokeEffectTickBegin = Engine::PropManager::Instance()->GetOffset("DT_SmokeGrenadeProjectile", "m_bDidSmokeEffect");

	return *(bool*)(uintptr_t(this) + m_nSmokeEffectTickBegin);
}
int& C_BasePlayer::m_nSmokeEffectTickBegin()
{
	static auto m_nSmokeEffectTickBegin = Engine::PropManager::Instance()->GetOffset("DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin");

	return *(int*)(uintptr_t(this) + m_nSmokeEffectTickBegin);
}
int& C_BasePlayer::m_hThrower()
{
	static auto m_hThrower = Engine::PropManager::Instance()->GetOffset("DT_BaseGrenade", "m_hThrower");

	return *(int*)(uintptr_t(this) + m_hThrower);
}
bool& C_BasePlayer::m_bHasHelmet()
{
	return *(bool*)(uintptr_t(this) + Engine::Displacement::DT_CSPlayer::m_bHasHelmet);
}

bool& C_BasePlayer::m_bHasHeavyArmor()
{
	static auto m_bHasHeavyArmor = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bHasHeavyArmor");

	return *(bool*)(uintptr_t(this) + m_bHasHeavyArmor);
}

bool& C_BasePlayer::m_bResumeZoom()
{
	static auto m_bResumeZoom = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_bResumeZoom");

	return *(bool*)(uintptr_t(this) + m_bResumeZoom);
}

int C_BasePlayer::get_sec_activity(int sequence)
{
	if (!this || !this->GetClientRenderable()) return 0;

	if (!this->GetModel())
		return 0;

	auto hdr = csgo.m_model_info()->GetStudioModel(this->GetModel());

	if (!hdr)
		return -1;

	return reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Engine::Displacement::Signatures[c_signatures::GETSEQUENCEACTIVITY])(this, hdr, sequence);
}

int& C_BasePlayer::m_nModelIndex()
{
	static auto model_Index = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_nModelIndex"));

	return *(int*)(uintptr_t(this) + model_Index);
}

int& C_BasePlayer::TakeDamage()
{
	static auto model_Index = Engine::PropManager::Instance()->GetOffset(sxor("DT_BasePlayer"), sxor("m_nModelIndex"));

	return *(int*)(uintptr_t(this) + model_Index - 0xf);
}

int& C_BasePlayer::m_ArmorValue()
{
	static auto m_ArmorValue = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_ArmorValue");

	return *(int*)(uintptr_t(this) + m_ArmorValue);
}

float& C_BasePlayer::m_flFlashMaxAlpha()
{
	static auto m_flFlashMaxAlpha = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");

	return *(float*)(uintptr_t(this) + m_flFlashMaxAlpha);
}

float& C_BasePlayer::m_flFlashDuration()
{
	static auto m_flFlashDuration = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flFlashDuration");

	return *(float*)(uintptr_t(this) + m_flFlashDuration);
}

float& C_BasePlayer::m_flFlashTime()
{
	static auto m_flFlashDuration = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flFlashDuration");

	return *(float*)(uintptr_t(this) + m_flFlashDuration - 0x10);
}

CStudioHdr* C_BasePlayer::GetModelPtr()
{
	auto m_pStudioHdr = (*(int*)(Engine::Displacement::Signatures[c_signatures::STUDIOHDR] + 0x2)) + 0x4;
	return *(CStudioHdr**)(uintptr_t(this)  + m_pStudioHdr/*0x294C*/);
}

uint8_t* C_BasePlayer::GetServerEdict() {

	if (!GetModuleHandleA("server.dll"))
		return nullptr;

	uintptr_t pServerGlobals = **(uintptr_t * *)(Engine::Displacement::Signatures[c_signatures::SERVER_GLOBALS] + 0x2);
	int iMaxClients = *(int*)((uintptr_t)pServerGlobals + 0x18);
	if (entindex() > 0 && iMaxClients >= 1) {
		if (entindex() <= iMaxClients) {
			int v10 = entindex() * 16;
			uintptr_t v11 = *(uintptr_t*)(pServerGlobals + 96);
			if (v11) {
				if (!((*(uintptr_t*)(v11 + v10) >> 1) & 1)) {
					uintptr_t v12 = *(uintptr_t*)(v10 + v11 + 12);
					if (v12) {
						uint8_t* pReturn = nullptr;

						// abusing asm is not good
						__asm
						{
							pushad
							mov ecx, v12
							mov eax, dword ptr[ecx]
							call dword ptr[eax + 0x14]
							mov pReturn, eax
							popad
						}

						return pReturn;
					}
				}
			}
		}
	}
	return nullptr;
}

bool C_BasePlayer::ComputeHitboxSurroundingBox(matrix3x4_t* mx, Vector* pVecWorldMins, Vector* pVecWorldMaxs)
{
	// Note that this currently should not be called during Relink because of IK.
	// The code below recomputes bones so as to get at the hitboxes,
	// which causes IK to trigger, which causes raycasts against the other entities to occur,
	// which is illegal to do while in the Relink phase.

	if (!GetModelPtr() || !this)
		return false;

	studiohdr_t* pStudioHdr = GetModelPtr()->m_pStudioHdr;
	if (!pStudioHdr)
		return false;

	mstudiohitboxset_t* set = pStudioHdr->pHitboxSet(m_nHitboxSet());
	if (!set || !set->numhitboxes)
		return false;

	// Compute a box in world space that surrounds this entity
	pVecWorldMins->Set(FLT_MAX, FLT_MAX, FLT_MAX);
	pVecWorldMaxs->Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	Vector vecBoxAbsMins, vecBoxAbsMaxs;
	for (int i = 0; i < set->numhitboxes; i++)
	{
		mstudiobbox_t* pbox = set->pHitbox(i);

		if (mx)
		{
			Math::TransformAABB(mx[pbox->bone], pbox->bbmin, pbox->bbmax, vecBoxAbsMins, vecBoxAbsMaxs);
			Math::VectorMin(*pVecWorldMins, vecBoxAbsMins, *pVecWorldMins);
			Math::VectorMax(*pVecWorldMaxs, vecBoxAbsMaxs, *pVecWorldMaxs);
		}
	}
	return true;
}

float C_BasePlayer::GetSequenceCycleRate(CStudioHdr* pStudioHdr, int iSequence) {
	//55 8B EC 53 57 8B 7D 08 8B D9 85 FF 75 double __userpurge SequenceDuration@<st0>(int a1@<ecx>, float a2@<xmm0>, int *a3, int a4)
	using GetSequenceCycleRateFn = float(__thiscall*)(void*, CStudioHdr*, int);
	return Memory::VCall< GetSequenceCycleRateFn >(this, 221)(this, pStudioHdr, iSequence);
}

float C_BasePlayer::GetLayerSequenceCycleRate(C_AnimationLayer* layer, int iSequence) {
	//55 8B EC 51 53 8B 5D 0C 56 57 8B F9 89 7D FC 83 FB 01 float __fastcall SetLayerSequence(int pThis, int edx, int sequence)
	//called in middle of function and result is being stored in layer->playback_rate
	using GetLayerSequenceCycleRateFn = float(__thiscall*)(void*, C_AnimationLayer*, int);
	return Memory::VCall< GetLayerSequenceCycleRateFn >(this, 222)(this, layer, iSequence);
}

#pragma runtime_checks( "", off )
float C_BasePlayer::GetSequenceMoveDist(CStudioHdr* pStudioHdr, int iSequence) {
	Vector vecReturn;

	// int __usercall GetSequenceLinearMotion@<eax>(int a1@<edx>, _DWORD *a2@<ecx>, int a3, _DWORD *a4)
	// it fastcall, but edx and ecx swaped
	// xref: Bad pstudiohdr in GetSequenceLinearMotion()!\n | Bad sequence (%i out of %i max) in GetSequenceLinearMotion() for model '%s'!\n

	using GetSequenceLinearMotionFn = int(__fastcall*)(CStudioHdr*, int, float*, Vector*);
	((GetSequenceLinearMotionFn)Engine::Displacement::Signatures[c_signatures::GETSEQUENCEMOVEDIST])(pStudioHdr, iSequence, m_flPoseParameter().data(), &vecReturn);
	__asm {
		add esp, 8
	}
	return vecReturn.Length();
}
#pragma runtime_checks( "", restore )

void C_BasePlayer::DrawServerHitboxes(float time) {
	//auto duration = g_CVar->FindVar( "sv_showlagcompensation_duration" )->GetFloat();
	auto duration = max(time, csgo.m_globals()->interval_per_tick * 2.0f);

	auto serverPlayer = GetServerEdict();
	if (serverPlayer) {
		// CBaseAnimating::DrawServerHitboxes(CBaseAnimating *this, float a2, __int64 a3)
		// ref: mass %.1f, first call in found function - draw hitboxes

		auto pCall = reinterpret_cast<uintptr_t*>(Engine::Displacement::Signatures[c_signatures::DRAWSERVERHITBOXES]);
		float fDuration = duration;

		__asm
		{
			pushad
			movss xmm1, fDuration
			push 0 //bool monoColor
			mov ecx, serverPlayer
			call pCall
			popad
		}
	}
}

bool C_BasePlayer::IsBot()
{
	player_info pinfo;
	return (csgo.m_engine()->GetPlayerInfo(entindex(), &pinfo) && pinfo.fakeplayer);
}

float& C_BasePlayer::m_flLastBoneSetupTime() {
	return *(float*)(uintptr_t(this)  + Engine::Displacement::DT_BaseAnimating::m_flLastBoneSetupTime);
}

int& C_BasePlayer::m_iMostRecentModelBoneCounter() {
	return *(int*)(uintptr_t(this)  + Engine::Displacement::DT_BaseAnimating::m_iMostRecentModelBoneCounter);
}

CBoneAccessor& C_BasePlayer::m_BoneAccessor() {
	return *(CBoneAccessor*)(uintptr_t(this)  + Engine::Displacement::DT_BaseAnimating::m_BoneAccessor);
}

int& C_BasePlayer::m_nComputedLODframe()
{
	//m_nComputedLODframe
	static auto m_flFlashDuration = Engine::PropManager::Instance()->GetOffset("DT_CSPlayer", "m_flFlashDuration");
	return *(int*)(uintptr_t(this) + m_flFlashDuration - 0x10);
}

bool C_BasePlayer::valid(bool check_team, bool check_dormant)
{
	if (!this) //-V704
		return false;

	if (!ctx.m_local())
		return false;

	if (!this->IsPlayer())
		return false;

	if (this->IsDead())
		return false;

	if (this->IsDormant() && check_dormant)
		return false;

	if (check_team && ctx.m_local()->m_iTeamNum() == this->m_iTeamNum())
		return false;

	return true;
}

bool* C_BasePlayer::s_bEnableInvalidateBoneCache()
{
	static auto s_bEnableInvalidateBoneCache = *(bool**)(Memory::Scan(sxor("client.dll"), sxor("F3 0F 5C C1 C6 05 ? ? ? ? ?")) + 0x4);
	return s_bEnableInvalidateBoneCache;
}

void C_BasePlayer::update_clientside_animations()
{
	if (GetModelPtr() == nullptr || !get_animation_state() || csgo.m_client_state()->m_iDeltaTick == -1)
		return;

	if (get_animation_state()->m_last_update_frame >= csgo.m_globals()->framecount)
		get_animation_state()->m_last_update_frame = csgo.m_globals()->framecount - 1;

	if (this == ctx.m_local()) {
		m_flThirdpersonRecoil() = m_aimPunchAngleScaled().x;
	}
	else {
		m_iEFlags() &= ~(EFL_DIRTY_ABSVELOCITY | EFL_DIRTY_ABSTRANSFORM);
	}

	if (get_animation_state()->m_last_update_time == csgo.m_globals()->curtime)
		get_animation_state()->m_last_update_time = csgo.m_globals()->curtime + TICKS_TO_TIME(1);

	ctx.updating_anims = true;
	this->client_side_animation() = true;

	get_animation_state()->m_weapon_last_bone_setup = m_hActiveWeapon() == -1 ? 0 : get_weapon();

	Memory::VCall<void(__thiscall*)(void*)>(this, UPDATE_CLIENTSIDE_ANIMATIONS)(this);

	ctx.updating_anims = false;
}

int C_BasePlayer::DetermineSimulationTicks(void)
{
	auto simulation_ticks = csgo.m_client_state()->m_iChockedCommands + 13 + 1;
	simulation_ticks = min(simulation_ticks, 17);

	return simulation_ticks;
}

int C_BasePlayer::AdjustPlayerTimeBase(int simulation_ticks)
{
	//Assert(simulation_ticks >= 0);
	if (simulation_ticks < 0 /*|| net == nullptr */)
		return m_nTickBase();

	//// server tickcount when cmd will arrive
	auto serverTickcount = ctx.current_tickcount;

	if (ctx.fakeducking)
		serverTickcount += 14 - csgo.m_client_state()->m_iChockedCommands;
	else
		serverTickcount += ctx.m_settings.fake_lag_shooting ? 1 : 0;

	static auto sv_clockcorrection_msecs = csgo.m_engine_cvars()->FindVar(sxor("sv_clockcorrection_msecs"));
	const float flCorrectionSeconds = Math::clamp(sv_clockcorrection_msecs->GetFloat() / 1000.0f, 0.0f, 1.0f);
	const int nCorrectionTicks = TIME_TO_TICKS(flCorrectionSeconds);

	serverTickcount += TIME_TO_TICKS(ctx.latency[FLOW_OUTGOING]);

	int	nIdealFinalTick = serverTickcount + nCorrectionTicks;
	int nCorrectedTick = nIdealFinalTick - simulation_ticks + TIME_TO_TICKS(ctx.latency[FLOW_OUTGOING]) + 1/*csgo.m_globals()->simTicksThisFrame*/;

	return nCorrectedTick;
}

float& C_BasePlayer::m_surfaceFriction()
{
	static unsigned int _m_surfaceFriction = FindInDataMap(GetPredDescMap(), "m_surfaceFriction");
	return *(float*)(uintptr_t(this) + _m_surfaceFriction);
}

std::string C_BasePlayer::m_szNickName()
{
	player_info info;
	if (!csgo.m_engine()->GetPlayerInfo(entindex(), &info))
		return "";

	return std::string(info.name);
}

C_WeaponCSBaseGun* C_BasePlayer::get_weapon()
{
	if (this == ctx.m_local())
		return m_weapon();
	
	//if (m_hActiveWeapon() <= 0)
	//	return nullptr;

	return (C_WeaponCSBaseGun*)(csgo.m_entity_list()->GetClientEntityFromHandle(m_hActiveWeapon()));
}

QAngle& C_BasePlayer::m_angEyeAngles()
{
	return *( QAngle* )(uintptr_t(this) + Engine::Displacement::DT_CSPlayer::m_angEyeAngles );
}

Vector& C_BasePlayer::m_vecOldOrigin()
{
	return *( Vector* )(uintptr_t(this) + 0x3AC);
}

std::vector<Vector> C_BasePlayer::build_capsules(Vector Min, Vector Max, float Radius, float scale, matrix3x4_t& Transform)
{
	//1 р€д образует шапки над плоскост€ми цилиндра, содержит все нужные поинты
	//2 р€д образует цилиндр(бесполезно дл€ мультипоинтов)
	//3 р€д образует контакты верхней плоскости цилиндра с нижней | 0 и 2 верхние боковые грани, 4 задн€€ верхн€€ могут быть использованы как альтернатива 1 р€ду

	std::vector <Vector> points = {};

	Vector min, max;
	Math::VectorTransform(Min, Transform, min);
	Math::VectorTransform(Max, Transform, max);

	auto center = (min + max) * 0.5f;

	points.emplace_back(center);

	Vector Forward = (Max - Min);
	Forward.Normalize();
	Forward *= (Radius * scale);
	QAngle Ref;
	Vector Up;
	Vector Right;
	Vector Forward2;
	if (!Forward.LengthSquared()) Forward.x += 0.0001f;
	Math::VectorAngles(Forward, Ref);
	Math::AngleVectors(Ref, &Forward2, &Right, &Up);
	Up *= (Radius * scale);
	Right *= (Radius * scale);
	//float ang_step = (2.0f * M_PI) / 16.0f; //original step(rip fps)
	float ang_step = (2.0f * M_PI) / 6.0f;

	for (float Angle = 0; Angle < M_PI; Angle += ang_step)
	{
		Vector Temp;
		Vector Temp2;
		Temp = Max + Right * cosf(Angle) + Forward * sinf(Angle);

		Math::VectorTransform(Temp, Transform, Temp2);

		points.emplace_back(Temp2);
	}

	Vector Temp[6];
	Vector Temp2[6];
	Temp[0] = Max + Up;
	Temp[1] = Min + Up;
	Temp[2] = Max - Up;
	Temp[3] = Min - Up;
	Temp[4] = Max + Right;
	Temp[5] = Max - Right;

	for (int i = 0; i < 6; i++)
		Math::VectorTransform(Temp[i], Transform, Temp2[i]);

	points.emplace_back(Temp2[2]);
	points.emplace_back(Temp2[0]);
	points.emplace_back(Temp2[3]);
	points.emplace_back(Temp2[1]);
	points.emplace_back(Temp2[5]);
	points.emplace_back(Temp2[4]);

	

	//0 / 2 , 22, 20, 23, 21, 26, 24

	return points;
}

//float Studio_SetPoseParameter(const CStudioHdr* pStudioHdr, int iParameter, float flValue, float& ctlValue)
//{
//	if (iParameter < 0 || iParameter >= pStudioHdr->GetNumPoseParameters())
//	{
//		ctlValue = 0;
//		return 0;
//	}
//
//	const mstudioposeparamdesc_t& PoseParam = ((CStudioHdr*)pStudioHdr)->pPoseParameter(iParameter);
//
//	Assert(IsFinite(flValue));
//
//	if (PoseParam.loop)
//	{
//		float wrap = (PoseParam.start + PoseParam.end) / 2.0 + PoseParam.loop / 2.0;
//		float shift = PoseParam.loop - wrap;
//
//		flValue = flValue - PoseParam.loop * floor((flValue + shift) / PoseParam.loop);
//	}
//
//	ctlValue = (flValue - PoseParam.start) / (PoseParam.end - PoseParam.start);
//
//	if (ctlValue < 0) ctlValue = 0;
//	if (ctlValue > 1) ctlValue = 1;
//
//	Assert(IsFinite(ctlValue));
//
//	return ctlValue * (PoseParam.end - PoseParam.start) + PoseParam.start;
//}

float C_BasePlayer::Studio_SetPoseParameter(const CStudioHdr* pStudioHdr, int iParameter, float flValue, float& ctlValue)
{
	if (iParameter < 0 || iParameter >= pStudioHdr->GetNumPoseParameters())
	{
		return 0;
	}

	const mstudioposeparamdesc_t& PoseParam = ((CStudioHdr*)pStudioHdr)->pPoseParameter(iParameter);

	//Assert(IsFinite(flValue));

	if (PoseParam.loop)
	{
		float wrap = (PoseParam.start + PoseParam.end) / 2.0 + PoseParam.loop / 2.0;
		float shift = PoseParam.loop - wrap;

		flValue = flValue - PoseParam.loop * floor((flValue + shift) / PoseParam.loop);
	}

	ctlValue = (flValue - PoseParam.start) / (PoseParam.end - PoseParam.start);

	if (ctlValue < 0)
		ctlValue = 0;
	if (ctlValue > 1)
		ctlValue = 1;

	//Assert(IsFinite(ctlValue));

	return ctlValue * (PoseParam.end - PoseParam.start) + PoseParam.start;
}

void C_BasePlayer::set_pose_param(int param, float value)
{
	//auto Studio_SetPoseParameter = (void*)(Engine::Displacement::Signatures[c_signatures::STUDIOSETPOSEPARAM]);
	////auto CBaseAnimating_SetPoseParameter = (void*)(Engine::Displacement::Signatures[c_signatures::SETPOSEPARAM]);

	////using SetPoseParameterFn = int(__thiscall*)(void*, int, float);

	////((SetPoseParameterFn)CBaseAnimating_SetPoseParameter)(this, param, value);

	/*auto result = 0.0f;
	auto* hdr = this->GetModelPtr();

	if (hdr == nullptr || !hdr || param < 0)
		return;

	__asm {
		pushad
		movss xmm2, [value]
		lea eax, [result]
		push eax
		mov edx, param
		mov ecx, hdr
		call Studio_SetPoseParameter
		pop eax
		popad
	}*/

	this->m_flPoseParameter()[param] = value;
}

bool C_BasePlayer::should_fix_modify_eye_pos()
{
	if (get_animation_state() == nullptr)
		return false;

	return get_animation_state()->m_landing || get_animation_state()->m_anim_duck_amount != 0.0f || !(m_fFlags() & FL_ONGROUND);
}

float GetPointScale(float flHitboxRadius, Vector* vecPos, Vector* veсPoint, int iHitbox) {
	auto pWeapon = ctx.m_local()->get_weapon(); // xmm0_2

	float flPointScale;
	float flScale;
	float flEndScale = 0.f;
	float flScaleState; // [esp+4h] [ebp-Ch]
	float ScaleState;
	float pDist = 0.f;

	float  v6; // ST0C_4
	float  v7; // xmm0_4


	float g_flSpread = pWeapon->GetSpread();
	float g_flInaccurarcy = pWeapon->GetInaccuracy();
	flScaleState = flHitboxRadius;

	if (!pWeapon) return 0.f;

	flScale = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_scale;

	flPointScale = (((flScale / 100.0) * 0.79999999) + 0.5) * flHitboxRadius;

	return std::fminf(flHitboxRadius * 0.9f, flPointScale);
}

bool C_BasePlayer::get_multipoints(int ihitbox, std::vector<Vector>& points, matrix3x4_t mx[], bool& only_center, float force_pointscale)
{
	points.clear();

	const model_t* model = this->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = csgo.m_model_info()->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->pHitboxSet(this->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->pHitbox(ihitbox);
	if (!bbox)
		return false;

	// convert rotation angle to a matrix.
	matrix3x4_t rot_matrix;
	Math::AngleMatrix(bbox->rotation, rot_matrix);

	// apply the rotation to the entity input space (local).
	matrix3x4_t matrix;
	Math::ConcatTransforms(mx[bbox->bone], rot_matrix, matrix);

	// extract origin from matrix.
	Vector origin = matrix.GetOrigin();

	// compute raw center point.
	Vector center = (bbox->bbmin + bbox->bbmax) / 2.f;

	float Scale = GetPointScale(bbox->radius, &Vector(ctx.m_local()->m_angEyeAngles().x, ctx.m_local()->m_angEyeAngles().y, ctx.m_local()->m_angEyeAngles().z), &center, ihitbox);

	if (Scale > 0.f) {

		Vector cr = Math::CalcAngleV(center, ctx.m_local()->Weapon_ShootPosition());
		QAngle curAngles = QAngle(cr.x, cr.y, cr.z);
		Vector forward;
		Math::AngleVectors(curAngles, &forward);

		const auto Right = forward.Cross(Vector(0, 0, 1));
		const auto Left = Vector(-Right.x, -Right.y, Right.z);
		const auto Top = Vector(0, 0, 1);

		switch (ihitbox) {

		case HITBOX_HEAD:
			points.emplace_back(center);
			points.emplace_back(Vector(Top * Scale));
			points.emplace_back(Vector(Right * Scale));
			points.emplace_back(Vector(Left * Scale));
			points.emplace_back(Vector((Top + Right) * Scale));
			points.emplace_back(Vector((Top + Left) * Scale));
			break;

		case HITBOX_NECK:
			break;

		case HITBOX_UPPER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_PELVIS:
			points.emplace_back(center);
			points.emplace_back(Vector(Right * Scale));
			points.emplace_back(Vector(Left * Scale));
			break;

		default:
			points.emplace_back(center);
			points.emplace_back(Vector(Right * Scale));
			points.emplace_back(Vector(Left * Scale));

			break;
		}

		// nothing left to do here.
		if (points.empty())
			return false;

		// transform capsule points.
		for (auto& p : points)
			Math::VectorTransform(p, mx[bbox->bone], p);

	}
	return true;
}
