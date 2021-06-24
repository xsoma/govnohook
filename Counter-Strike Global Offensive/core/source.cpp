#include "source.hpp"
#include <hooks/hooked.hpp>
#include <hooks/detour/detours.h>
#include <props/displacement.hpp>

#include <props/player.hpp>
#include "minhook/minhook.h"
#include <antiaim/anti_aimbot.hpp>
#include <misc/misc.hpp>
#include <ragebot/resolver.hpp>
#include <visuals/visuals.hpp>
#include "usercmd.hpp"
#include <ragebot/lag_comp.hpp>
#include <visuals/chams.hpp>
#include <ragebot/autowall.hpp>
#include <ragebot/rage_aimbot.hpp>
#include <ragebot/prediction.hpp>
#include <props/weapon.hpp>
#include <visuals/sound_parser.hpp>
#include <features/inventory/parser.h>
#include <misc/music_player.hpp>
#include <visuals/grenades.hpp>
#include "weather_controller.hpp"
#include <sdk.hpp>
#include <legitbot/aimbot.hpp>
#include <configs/configs.h>
#include <visuals/grenade_warning.h>

#include <visuals/chams_hit.h>
#include <steam/steam_api.h>
#include <protobuf/Protobuffs.h>
#include "inventory/inventorychanger.h"
#include "inventory/items.h"
#include "lua/Clua.h"
#include "lua/CLuaHook.h"
#include <core/source.hpp>
#include <fstream>
#include <sdk/notify/notify.h>

#include <sdk/props/prop_manager.hpp>
#include <menu/menu/i_menu.hpp>
#include <menu/setup/settings.h>
#include <features/visuals/grenade_warning.h>
#include <features/netchannel/net_channel.h>

inline unsigned int get_virtual(void* _class, unsigned int index) { return static_cast<unsigned int>((*static_cast<int**>(_class))[index]); }
#define FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MIN 4.0f
#define FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MAX 10.0f
constexpr auto _box_pttrn = LIT(("\x85\xC0\x74\x2D\x83\x7D\x10\x00\x75\x1C"));

class VPlane
{
public:

	Vector		m_Normal;
	float		m_Dist;
};

typedef VPlane Frustum[6];

typedef void(__thiscall* StandardBlendingRules_t)(C_BasePlayer*, CStudioHdr*, Vector*, Quaternion*, float_t, int32_t);
typedef void(__thiscall* BuildTransformations_t)(C_BasePlayer*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, int, BYTE*);
typedef void(__thiscall* DataUpdate_universal_t)(IClientNetworkable*, int32_t);
typedef bool(__cdecl* ReportHit_t)(CCSUsrMsg_ReportHit*);
typedef int(__thiscall* SetViewmodelOffsets_t)(void*, int, float, float, float);
typedef bool(__stdcall* IsUsingStaticPropDebugModes_t)();
typedef bool(__thiscall* ShouldCollide_t)(C_BasePlayer*, int, int);
typedef float(__stdcall* AdjustFrameTime_t)(float);
typedef void(__thiscall* GetColorModulation_t)(void*, float*, float*, float*);
typedef void(__vectorcall* CL_Move_t)(float, bool);
typedef void(__stdcall* CL_ReadPackets_t)(bool);
typedef void(__stdcall* Host_RunFrame_Input_t)(float, bool);
typedef void(__stdcall* Host_RunFrame_Client_t)(bool);
typedef const char* (__thiscall* GetForeignFallbackFontName_t)(void*);
typedef const char* (__thiscall* GetForeignFallbackFontName_t)(void*);
typedef bool(__thiscall* ShouldSkipAnimationFrame_t)(C_BaseAnimating*);
typedef QAngle* (__thiscall* EyeAngles_t)(void*);
typedef void(__thiscall* PhysicsSimulate_t)(void*);
typedef void(__thiscall* CalcAbsoluteVelocity_t)(C_BasePlayer*);
typedef const Vector& (__thiscall* GetRenderOrigin_t)(C_BaseAnimating*);
typedef void(__thiscall* SetLocalOrigin_t)(void*, const Vector&);
typedef void(__thiscall* ModifyEyePosition_t)(CCSGOPlayerAnimState*, Vector&);
typedef void(__thiscall* DoExtraBonesProcessing_t)(uintptr_t*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, uint8_t*, void*);
typedef void(__thiscall* UpdateClientSideAnimations_t)(C_BasePlayer*);
typedef void(__thiscall* SetupVelocity_t)(CCSGOPlayerAnimState*);
typedef void(__thiscall* SetUpMovement_t)(CCSGOPlayerAnimState*);
typedef void(__thiscall* RenderPopView_t)(void*, void*, Frustum);
typedef bool(__thiscall* ShouldFlipModel_t)(C_BaseViewModel*);
typedef bool(__thiscall* isHLTV_t)(IVEngineClient*);
typedef int(__stdcall* IsBoxVisible_t)(const Vector&, const Vector&);
typedef bool(__thiscall* SetupBones_t)(void*, matrix3x4_t*, int, int, float);
typedef bool(__thiscall* DispatchUserMessage_t)(void*, int type, unsigned int a3, unsigned int length, const void* msg_data);
typedef void(__thiscall* CalcViewBob_t)(void*, Vector&);
//typedef void(__thiscall* PlayStepSound_t)(void*, Vector&, surface_data_t*, float, bool);
typedef void(__thiscall* AddViewModelBob_t)(C_PredictedViewModel*, void*, Vector&, QAngle&);
typedef void(__thiscall* FireEvents_t)(IVEngineClient*);
typedef INetChannelInfo* (__thiscall* GetNetChannelInfo_t)(IVEngineClient*);
using ProcessPacket_t = void(__fastcall*)(void*, bool);

typedef bool(__thiscall* OverrideConfig_t)(IMaterialSystem*, MaterialSystem_Config_t&, bool);
typedef float(__thiscall* GetAlphaModulation_t)(IMaterial*);
typedef bool(__thiscall* ShouldHitEntity_t)(void*, IHandleEntity*, int);
typedef void(__thiscall* ClipRayCollideable_t)(void*, const Ray_t&, uint32_t, ICollideable*, CGameTrace*);
typedef void(__thiscall* AttachmentHelper_t)(IClientRenderable*, void*);
typedef void(__thiscall* AnimEventHook_t)(void*, int);
typedef void(__thiscall* CheckForSequenceChange_t)(void*, void*, int, bool, bool);
typedef void(__thiscall* AdjustPlayerTimeBase_t)(void*, int);
typedef int(*ProcessInterpolatedList_t)(void);
using TraceRay_t = void(__thiscall*)(void*, const Ray_t&, unsigned int, ITraceFilter*, trace_t*);
using DrawSetColor_t = void(__thiscall*)(void*, int, int, int, int);
using CalcView_t = void(__thiscall*)(C_BasePlayer*, Vector&, QAngle&, float&, float&, float&);

class zInterfaceReg
{
private:
	using InstantiateInterfaceFn = void* (*)();
public:
	InstantiateInterfaceFn m_CreateFn;
	const char* m_pName;
	zInterfaceReg* m_pNext;
};
template<typename T>
static T* zget_interface(const char* mod_name, const char* interface_name, bool exact = false) {
	T* iface = nullptr;
	zInterfaceReg* register_list;
	int part_match_len = strlen(interface_name); //-V103

	DWORD interface_fn = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA(mod_name), ("CreateInterface")));

	if (!interface_fn) {
		return nullptr;
	}

	unsigned int jump_start = (unsigned int)(interface_fn)+4;
	unsigned int jump_target = jump_start + *(unsigned int*)(jump_start + 1) + 5;

	register_list = **reinterpret_cast<zInterfaceReg***>(jump_target + 6);

	for (zInterfaceReg* cur = register_list; cur; cur = cur->m_pNext) {
		if (exact == true) {
			if (strcmp(cur->m_pName, interface_name) == 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
		else {
			if (!strncmp(cur->m_pName, interface_name, part_match_len) && std::atoi(cur->m_pName + part_match_len) > 0) //-V106
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
	}
	return iface;
}

DWORD OriginalSetupBones;
DWORD OriginalDispathMessege;
DWORD OriginalUpdateClientSideAnimations;
DWORD OriginalSetupVelocity;
DWORD OriginalSetUpMovement;
DWORD OriginalCalcAbsoluteVelocity;
DWORD OriginalStandardBlendingRules;
DWORD OriginalBuildTransformations;
DWORD OriginalShouldCollide;
DWORD OriginalDoExtraBonesProcessing;
DWORD OriginalGetForeignFallbackFontName;
DWORD OriginalShouldSkipAnimationFrame;
DWORD OriginalCL_ReadPackets;
DWORD OriginalModifyEyePosition;
DWORD OriginalPhysicsSimulate;
DWORD OriginalAdjustPlayerTimeBase;
DWORD OriginalCL_Move;
DWORD OriginalShouldHitEntity;
DWORD OriginalGetRenderOrigin;
DWORD OriginalSetViewmodelOffsets;
DWORD OriginalGetColorModulation;
DWORD OriginalIsUsingStaticPropDebugModes;
DWORD OriginalReportHit;
DWORD OriginalCalcView;
DWORD OriginalPreDataUpdate;
DWORD OriginalPostDataUpdate;
DWORD OriginalEyeAngles;
DWORD OriginalProcessInterpolatedList;
DWORD OriginalCalcViewBob;
DWORD OriginalAddViewModelBob;
DWORD OriginalSetLocalOrigin;
DWORD OriginalPlayStepSound;
DWORD OriginalAdjustFrameTime;
DWORD OriginalShouldFlipModel;
DWORD OriginalRenderPopView;
DWORD OriginalAnimEventHook;
DWORD OriginalCheckForSequenceChange;
DWORD OriginalHost_RunFrame_Input;
DWORD OriginalHost_RunFrame_Client;
DWORD OriginalPerformScreenOverlay;
using GCRetrieveMessage = unsigned long(__thiscall*)(void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
using GCSendMessage = unsigned long(__thiscall*)(void*, uint32_t unMsgType, const void* pubData, uint32_t cubData);

inline GCRetrieveMessage oGCRetrieveMessage;
inline GCSendMessage oGCSendMessage;
using AddRenderableFn = int(__thiscall*)(void*, IClientRenderable* p_renderable, int, RenderableTranslucencyType_t, int, int);
inline AddRenderableFn oAddRenderable;
typedef bool(__thiscall* tDispatchUserMessage)(void*, int type, unsigned int a3, unsigned int length, const void* msg_data);
inline tDispatchUserMessage otDispatchUserMessage;

void __fastcall hkPerformScreenOverlay(void* _this, void* edx, int x, int y, int w, int h);
decltype(&hkPerformScreenOverlay) oPerformScreenOverlay;

GetAlphaModulation_t OriginalGetAlphaModulation;

RecvVarProxyFn m_flSimulationTime;
RecvVarProxyFn m_flLowerBodyYawTarget;
RecvVarProxyFn m_flAbsYaw;
RecvVarProxyFn m_flPlaybackRate;
RecvVarProxyFn m_flWeight;
RecvVarProxyFn m_flCycle;
RecvVarProxyFn m_Sequence;
RecvVarProxyFn m_nSequence;
RecvVarProxyFn m_flVelocityModifier;
RecvVarProxyFn m_vecForce;
RecvVarProxyFn m_ViewModel;

ClientEffectCallback oImpact;

//ShouldHitEntity_t oShouldHitEntity = nullptr;

c_csgo	   csgo;
c_vmthooks vmt;
c_context  ctx;

struct lagcomp_mt
{
	lagcomp_mt() {  };
	bool job_done = false;
};

namespace Hooked
{
	INetChannelInfo* __fastcall GetNetChannelInfo(IVEngineClient* _this, int edx)
	{

		static void* offs = nullptr;
		if (ctx.init_finished && !offs)
			offs = (void*)Memory::Scan(sxor("client.dll"), sxor("8B C8 85 C9 0F 84 ? ? ? ? 8B 01 51"));

		static auto ofc = vmt.m_engine->VCall<GetNetChannelInfo_t>(78);

		if (_ReturnAddress() == offs)
			return nullptr;

		return ofc(_this);
	}
	_declspec(noinline)void DoExtraBonesProcessing_Detour(uintptr_t* ecx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_computed, void* context)
	{
		if (!ctx.is_updating_fake)
			return;
	}
	bool __cdecl ReportHit(CCSUsrMsg_ReportHit* info)
	{
		if (info) {
			WorldHitmarkerData_t data;

			data.m_alpha = 1.f;
			data.m_time = csgo.m_globals()->curtime;

			data.m_pos_x = info->pos_x;
			data.m_pos_y = info->pos_y;
			data.m_pos_z = info->pos_z;

			world_hitmarker.push_back(data);
		}

		return ((ReportHit_t)OriginalReportHit)(info);
	}
	bool __fastcall ShouldSkipAnimationFrame(C_BaseAnimating* ecx, uint32_t*)
	{
		VIRTUALIZER_START;

		auto player = reinterpret_cast<C_BasePlayer*>(uintptr_t(ecx) - 4);

		if (player && ctx.m_local() && (player == ctx.m_local() || player->m_iTeamNum() != ctx.m_local()->m_iTeamNum()))
			return false;

		return reinterpret_cast<ShouldSkipAnimationFrame_t>(OriginalShouldSkipAnimationFrame)(ecx);

		VIRTUALIZER_END;
	}
	void __fastcall CheckForSequenceChange(void* ecx, int edx, void* hdr, int cur_sequence, bool force_new_sequence, bool interpolate)
	{
		reinterpret_cast<CheckForSequenceChange_t>(OriginalCheckForSequenceChange)(ecx, hdr, cur_sequence, force_new_sequence, false);
	}
	void __fastcall AnimEventHook(void* ecx, int edx, int data_type)
	{
		reinterpret_cast<AnimEventHook_t>(OriginalAnimEventHook)(ecx, data_type);
	}
	void __fastcall DoExtraBonesProcessing(uintptr_t* ecx, uint32_t, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_computed, void* context)
	{
		DoExtraBonesProcessing_Detour(ecx, hdr, pos, q, matrix, bone_computed, context);
	}
	DECLSPEC_NOINLINE void modify_eye_position_server(CCSGOPlayerAnimState* ecx, Vector& pos)
	{
		VIRTUALIZER_START;

		if (ecx && ecx->m_player != nullptr && (ecx->m_landing || ecx->m_anim_duck_amount != 0.0f || !ecx->m_player->m_hGroundEntity()))
		{
			int bone = ecx->m_player->LookupBone(sxor("head_0"));

			if (bone != -1)
			{
				Vector vecHeadPos;
				QAngle temp;
				ecx->m_player->GetBonePosition(bone, vecHeadPos, temp);

				auto flHeadHeight = vecHeadPos.z + 1.7f;
				if (pos.z > flHeadHeight)
				{
					auto tmp = 0.0f;
					tmp = (fabsf(pos.z - flHeadHeight) - 4.0f) * 0.16666667f;
					if (tmp >= 0.0f)
						tmp = fminf(tmp, 1.0f);
					pos.z = ((flHeadHeight - pos.z)
						* (((tmp * tmp) * 3.0f) - (((tmp * tmp) * 2.0f) * tmp)))
						+ pos.z;
				}
			}
		}
		VIRTUALIZER_END;
	}
	void __fastcall CalcView(C_BasePlayer* m_player, uint32_t, Vector& m_eye_origin, QAngle& m_eye_angles, float& zNear, float& zFar, float& fov)
	{
		VIRTUALIZER_START;;

		((CalcView_t)OriginalCalcView)(m_player, m_eye_origin, m_eye_angles, zNear, zFar, fov);

		if (ctx.m_local() != nullptr && m_player != nullptr) {
			if (m_player == ctx.m_local() && ctx.fakeducking)
			{
				if (csgo.m_input()->m_fCameraInThirdPerson) // fix the up and down bob while fake ducking
					m_eye_origin.z = ctx.m_local()->get_abs_origin().z + 64.f;
			}

			m_player->delay_unscope(fov);

			if (m_player->m_nIsAutoMounting() > 0)
			{
				auto vectors = csgo.m_game_rules()->GetViewVectors();
				auto currentautomoveorigin = vectors->m_vDuckView + m_player->GetAutoMoveOrigin();
				auto automovetargetend = vectors->m_vDuckHullMax + m_player->GetAutomoveTargetEnd();

				float time = (csgo.m_globals()->curtime - (m_player->GetAutomoveTargetTime() - (m_player->GetAutomoveTargetTime() - m_player->GetAutomoveStartTime())))
					/ (m_player->GetAutomoveTargetTime() - m_player->GetAutomoveStartTime());
				time = Math::clamp(time, 0.0f, 1.0f);
				float fuck = powf(time, 0.6214906f);
				fuck = fminf(fuck + 0.5f, 1.0f);
				m_eye_origin = (automovetargetend - currentautomoveorigin) * fuck + currentautomoveorigin;
			}
		}

		VIRTUALIZER_END;
	}
	int ProcessInterpolatedList()
	{
		auto allow_extrapolation_addr = *(bool**)(Engine::Displacement::Signatures[c_signatures::ALLOW_EXTRAPOLATION] + 1);

		if (allow_extrapolation_addr)
			*allow_extrapolation_addr = false;

		return (ProcessInterpolatedList_t(OriginalProcessInterpolatedList))();
	}
	_declspec(noinline)const char* GetForeignFallbackFontName_Detour(uintptr_t* ecx)
	{
		if (strlen(Drawing::LastFontName) > 1)
			return Drawing::LastFontName;
		else
			return reinterpret_cast<GetForeignFallbackFontName_t>(OriginalGetForeignFallbackFontName)(ecx);
	}
	const char* __fastcall GetForeignFallbackFontName(uintptr_t* ecx, uint32_t)
	{
		return GetForeignFallbackFontName_Detour(ecx);
	}
	void __fastcall PhysicsSimulate(uintptr_t* ecx, uint32_t)
	{
		auto player = reinterpret_cast<C_BasePlayer*>(ecx);
		auto& m_nSimulationTick = *reinterpret_cast<int*>(uintptr_t(player) + 0x2AC);
		auto cctx = reinterpret_cast<C_CommandContext*>(uintptr_t(player) + 0x34FC);

		if (!player
			|| player->IsDead()
			|| csgo.m_globals()->tickcount == m_nSimulationTick
			|| player != ctx.m_local()
			|| !cctx->needsprocessing
			|| csgo.m_engine()->IsPlayingDemo()
			|| csgo.m_engine()->IsHLTV()
			|| player->m_fFlags() & 0x40)
		{
			reinterpret_cast<PhysicsSimulate_t>(OriginalPhysicsSimulate)(ecx);
			return;
		}

		player->m_vphysicsCollisionState() = 0;

		if ((ctx.hold_aim || !ctx.m_local()->IsDead()) && ctx.hold_angles == player->m_angEyeAngles())
			++ctx.hold_aim_ticks;

		auto pred = &Engine::Prediction::Instance();

		const bool bValid = (g_PacketManager.m_aCommands->tick_count != 0x7FFFFFFF);

		if (!bValid)
		{
			m_nSimulationTick = csgo.m_globals()->tickcount;
			cctx->needsprocessing = false;
			return;
		}
		else
		{
			((PhysicsSimulate_t)OriginalPhysicsSimulate)(ecx);

		}
	}
	void __fastcall ClipRayCollideable(void* ecx, void* edx, const Ray_t& ray, uint32_t fMask, ICollideable* pCollide, CGameTrace* pTrace)
	{
		static auto ofc = vmt.m_engine_trace->VCall<ClipRayCollideable_t>(4);

		if (pCollide) {
			// extend the tracking
			const auto old_max = pCollide->OBBMaxs().z;

			//auto v9 = (C_BasePlayer*)(uintptr_t(pCollide) - 4);

		//	if (v9 && *(void**)v9 && !v9->IsDead() && *(int*)(uintptr_t(v9) + 0x64) < 64 && v9->get_weapon() && v9->get_weapon()->is_knife())
			pCollide->OBBMaxs().z += 5; // if the player is holding a knife and ducking in air we can still trace to this faggot and hit him

			ofc(ecx, ray, fMask, pCollide, pTrace);

			// restore Z
			pCollide->OBBMaxs().z = old_max;
		}
		else
			return ofc(ecx, ray, fMask, pCollide, pTrace);
	}
	void __fastcall TraceRay(void* thisptr, void*, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace)
	{
		static auto ofc = vmt.m_engine_trace->VCall<TraceRay_t>(5);

		//if (!ctx.in_hbp)
		return ofc(thisptr, ray, fMask, pTraceFilter, pTrace);
	}
	_declspec(noinline)void ModifyEyePosition_Detour(CCSGOPlayerAnimState* ecx, Vector& pos)
	{
		//auto islocal = ecx != nullptr && ecx->ent == ctx.m_local() && !ctx.m_local()->IsDead();

		ecx->m_smooth_height_valid = false;

		if (ctx.fix_modify_eye_pos && ecx != nullptr)
			((ModifyEyePosition_t)OriginalModifyEyePosition)(ecx, pos);
	}
	void __fastcall ModifyEyePosition(CCSGOPlayerAnimState* ecx, void* edx, Vector& pos)
	{
		ModifyEyePosition_Detour(ecx, pos);
	}
	_declspec(noinline)void CalcViewBob_Detour(C_BasePlayer* ecx, Vector& pos)
	{
		auto islocal = ecx != nullptr && ecx == ctx.m_local() && !ctx.m_local()->IsDead();

		if (!islocal)
			((CalcViewBob_t)OriginalCalcViewBob)(ecx, pos);
	}
	void __fastcall CalcViewBob(C_BasePlayer* ecx, void* edx, Vector& pos)
	{
		CalcViewBob_Detour(ecx, pos);
	}
	_declspec(noinline)void AddViewModelBob_Detour(C_PredictedViewModel* ecx, C_BasePlayer* owner, Vector& eyePosition, QAngle& eyeAngles)
	{
		const auto islocal = ecx != nullptr && owner == ctx.m_local() && !ctx.m_local()->IsDead();

		if (!islocal)
			reinterpret_cast<AddViewModelBob_t>(OriginalAddViewModelBob)(ecx, owner, eyePosition, eyeAngles);
	}
	void __fastcall AddViewModelBob(C_PredictedViewModel* ecx, void* edx, C_BasePlayer* owner, Vector& eyePosition, QAngle& eyeAngles)
	{
		AddViewModelBob_Detour(ecx, owner, eyePosition, eyeAngles);
	}
	_declspec(noinline)void UpdateClientSideAnimations(C_BasePlayer* ecx)
	{
		VIRTUALIZER_START;;

		C_BasePlayer* pEntity = ecx;
		auto v4 = pEntity && pEntity->entindex() < 64;
		if (pEntity)
		{
			if (pEntity == ctx.m_local())
			{
				if (!ctx.updating_anims)
				{
					for (int i = 0; i < 128; i++)
						ctx.local_matrix[i].SetOrigin(pEntity->get_abs_origin() - ctx.local_origin[i]);

					for (int i = 0; i < 128; i++)
						ctx.fake_matrix[i].SetOrigin(pEntity->get_abs_origin() - ctx.desync_origin[i]);

					memcpy(pEntity->m_CachedBoneData().Base(), ctx.local_matrix, sizeof(matrix3x4_t) * pEntity->m_CachedBoneData().Count());

					const auto oldBones = pEntity->m_BoneAccessor().m_pBones;
					pEntity->m_BoneAccessor().m_pBones = ctx.local_matrix;
					pEntity->attachment_helper();
					pEntity->m_BoneAccessor().m_pBones = oldBones;

					return;
				}
				else
					return reinterpret_cast<UpdateClientSideAnimations_t>(OriginalUpdateClientSideAnimations)(pEntity);
			}
			else {
				if (ctx.updating_anims || pEntity->IsDormant() || !ctx.m_local() || ctx.m_local()->m_iTeamNum() == pEntity->m_iTeamNum() || pEntity->entindex() > 64)
					return reinterpret_cast<UpdateClientSideAnimations_t>(OriginalUpdateClientSideAnimations)(pEntity);
			}
		}
		else
			reinterpret_cast<UpdateClientSideAnimations_t>(OriginalUpdateClientSideAnimations)(pEntity);

		//#ifdef VIRTUALIZER
		VIRTUALIZER_END;
		//#endif // VIRTUALIZER


	}
	bool copy_bone_cache(C_BasePlayer* pl, matrix3x4_t* a2, int a3)
	{
		if (!a2 || a3 == -1)
			return true;

		if (pl->m_CachedBoneData().Count() < a3)
			return 0;

		if (a2)
		{
			memcpy(a2, pl->m_CachedBoneData().Base(), 48 * pl->m_CachedBoneData().Count());
		}
		return 1;
	}
	bool __fastcall cl_clock_correction_get_bool(void* pConVar, void* ebx)
	{
		static auto CL_ReadPackets_ret = reinterpret_cast<void*>(Memory::Scan(sxor("engine.dll"), sxor("85 C0 0F 95 C0 84 C0 75 0C")));

		if (!csgo.m_engine()->IsInGame() || CL_ReadPackets_ret != _ReturnAddress())
			return vmt.m_cl_clock_correction->VCall<bool(__thiscall*)(void*)>(13)(pConVar);

		return false;
	}
	bool __fastcall cl_smooth_get_bool(void* pConVar, void* ebx)
	{
		VIRTUALIZER_START;

		static auto GetPredictionErrorSmoothingVector_ret = reinterpret_cast<void*>(Memory::Scan(sxor("client.dll"), sxor("85 C0 0F 84 BF ? ? ? A1")));

		if (!csgo.m_engine()->IsInGame() || GetPredictionErrorSmoothingVector_ret != _ReturnAddress() || !(ctx.cheat_option_flags & hook_should_return_cl_smooth))
			return vmt.cl_smooth->VCall<bool(__thiscall*)(void*)>(13)(pConVar);

		ctx.cheat_option_flags &= ~hook_should_return_cl_smooth;

		VIRTUALIZER_END;

		return false;
	}
	void __fastcall DrawSetColor(void* _this, void* edx, int r, int g, int b, int a)
	{
		static auto ofc = vmt.m_surface->VCall<DrawSetColor_t>(15);

		ofc(_this, r, g, b, a);
	}
	void WriteUsercmd(void* buf, CUserCmd* incmd, CUserCmd* outcmd) {
		using WriteUsercmd_t = void(__fastcall*)(void*, CUserCmd*, CUserCmd*);
		static WriteUsercmd_t WriteUsercmdF = reinterpret_cast<WriteUsercmd_t>(Memory::Scan(
			sxor("client.dll"), sxor("55  8B  EC  83  E4  F8  51  53  56  8B  D9")));

		__asm
		{
			mov     ecx, buf
			mov     edx, incmd
			push    outcmd
			call    WriteUsercmdF
			add     esp, 4
		}
	}
	bool __fastcall WriteUsercmdDeltaToBuffer(void* ecx, void* edx, int slot, void* buf, int from, int to, bool new_cmd)
	{
		VIRTUALIZER_START;

		using Fn = bool(__thiscall*)(void*, int, void*, int, int, bool);
		static auto ofc = vmt.m_client->VCall<Fn>(24);

		if (ctx.shift_amount <= 0)
			return ofc(ecx, slot, buf, from, to, new_cmd);

		if (from != -1)
			return true;

		int _from = -1;

		uintptr_t frame_ptr{};
		__asm {
			mov frame_ptr, ebp;
		}

		int* backup_commands = reinterpret_cast<int*>(frame_ptr + 0xFD8);
		int* new_commands = reinterpret_cast<int*>(frame_ptr + 0xFDC);
		int32_t newcmds = *new_commands;

		const auto shift_amt = ctx.shift_amount;

		ctx.shift_amount = 0;
		*backup_commands = 0;

		int choked_modifier = newcmds + shift_amt;

		if (choked_modifier > 62)
			choked_modifier = 62;

		*new_commands = choked_modifier;

		const int next_cmdnr = csgo.m_client_state()->m_iChockedCommands + csgo.m_client_state()->m_iLastOutgoingCommand + 1;
		int _to = next_cmdnr - newcmds + 1;
		if (_to <= next_cmdnr)
		{
			while (ofc(ecx, slot, buf, _from, _to, true))
			{
				_from = _to++;
				if (_to > next_cmdnr)
				{
					goto LABEL_11; // jump out of scope.
				}
			}
			return false;
		}
	LABEL_11:

		auto* ucmd = csgo.m_input()->GetUserCmd(_from);
		if (!ucmd)
			return true;

		CUserCmd to_cmd{};
		CUserCmd from_cmd{};

		from_cmd = *ucmd;
		to_cmd = from_cmd;

		++to_cmd.command_number;
		to_cmd.tick_count += ctx.tickrate + 2 * ctx.tickrate;

		if (newcmds > choked_modifier)
			return true;

		for (int i = (choked_modifier - newcmds + 1); i > 0; --i)
		{
			WriteUsercmd(buf, &to_cmd, &from_cmd);

			from_cmd = to_cmd;
			++to_cmd.command_number;
			++to_cmd.tick_count;
		}
		VIRTUALIZER_END;

		return true;
	}
	bool __fastcall SetupBones(void* ECX, void* EDX, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
	{
		VIRTUALIZER_START;

		if (!ECX)
			return reinterpret_cast<SetupBones_t>(OriginalSetupBones)(0, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

		auto* player = reinterpret_cast<C_BasePlayer*>(uintptr_t(ECX) - 4);

		if (reinterpret_cast<int>(ECX) == 0x4 || player == nullptr || *reinterpret_cast<int*>(uintptr_t(player) + 0x64) > 64 || player->GetClientClass() != nullptr && player->GetClientClass()->m_ClassID != class_ids::CCSPlayer)
			return reinterpret_cast<SetupBones_t>(OriginalSetupBones)(ECX, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

		const auto is_local = player == ctx.m_local() && player != nullptr;

		if (ctx.setup_bones || !ctx.m_local() || !is_local && player->m_iTeamNum() == ctx.m_local()->m_iTeamNum() || player->m_CachedBoneData().Count() == 0)
		{
			if (player->GetModelPtr() && player->GetModelPtr()->m_pStudioHdr) {
				for (auto i = 0; i < player->GetModelPtr()->m_boneFlags.Count(); i++) {
					auto pBone = &player->GetModelPtr()->m_boneFlags[i];

					if (!pBone)
						continue;

					*pBone &= ~5;
				}
			}

			for (int i = 0; i < player->m_anim_overlay().Count(); ++i) {
				auto& elem = player->m_anim_overlay().Element(i);

				if (player != elem.m_pOwner)
					elem.m_pOwner = (void*)player;
			}

			auto prev_p = (/*!is_local ||*/ player->get_animation_state() == nullptr) ? 0 : player->get_animation_state()->m_weapon_last_bone_setup;

			if (prev_p != 0)
				player->get_animation_state()->m_weapon_last_bone_setup = player->get_animation_state()->m_weapon;

			const auto old_effects = player->m_fEffects();
			player->m_fEffects() |= 8;

			auto m_orig = reinterpret_cast<SetupBones_t>(OriginalSetupBones)(ECX, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

			player->m_fEffects() = old_effects;

			if (prev_p != 0)
				player->get_animation_state()->m_weapon_last_bone_setup = prev_p;

			return m_orig;
		}
		else {
			if (player != ctx.m_local())
			{
				const auto idx = *reinterpret_cast<int*>(uintptr_t(player) + 0x64) - 1;

				if (idx < 64)
					return copy_bone_cache(player, pBoneToWorldOut, nMaxBones);

				return player->m_CachedBoneData().Count() > 0;
			}
			else
			{
				if (!pBoneToWorldOut || nMaxBones == -1)
					return true;

				if (pBoneToWorldOut && ctx.local_matrix)
				{
					memcpy(pBoneToWorldOut, ctx.local_matrix, sizeof(matrix3x4_t) * player->m_CachedBoneData().Count());
					return true;
				}

				return false;
			}
		}
		VIRTUALIZER_END;
	}
	void __fastcall UpdateClientSideAnimation(C_BasePlayer* ecx, void* edx)
	{
		UpdateClientSideAnimations(ecx);
	}
	_declspec(noinline)bool __fastcall ShouldFlipModel(C_BaseViewModel* thisptr, void* edx)
	{
		auto m_original = ((ShouldFlipModel_t)OriginalShouldFlipModel)(thisptr);

		//if (ctx.m_local() && thisptr && (CBaseHandle)ctx.m_local()->m_hViewModel() == thisptr->GetRefEHandle()
		//	&& ctx.latest_weapon_data && ctx.latest_weapon_data->pad_0090[0])
		//{
		//	//if (m_weapon() && m_weapon()->is_knife() && ctx.m_settings.misc_knife_hand_switch)
		//	//	return !m_original;
		//}

		return m_original;
	}
	void __fastcall StandardBlendingRules(C_BasePlayer* ent, void* a2, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int32_t bonemask)
	{
		if (ent != nullptr && ent && *reinterpret_cast<int*>(uintptr_t(ent) + 0x64) <= 64)
		{
			if (!(ent->m_fEffects() & 8))
				ent->m_fEffects() |= 8;

			reinterpret_cast<StandardBlendingRules_t>(OriginalStandardBlendingRules)(ent, hdr, pos, q, curtime, bonemask & ~0xC0000u);

			if (ent->m_fEffects() & 8)
				ent->m_fEffects() &= ~8u;

			return;
		}

		reinterpret_cast<StandardBlendingRules_t>(OriginalStandardBlendingRules)(ent, hdr, pos, q, curtime, bonemask);
	}
	void __fastcall BuildTransformations(C_BasePlayer* ent, void* a2, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& cameraTransform, int boneMask, BYTE* boneComputed)
	{
		if (ent != nullptr && ent && *reinterpret_cast<int*>(uintptr_t(ent) + 0x64) <= 64)
		{
			const auto flags = ent->m_fEffects();

			if (!(flags & 8))
				ent->m_fEffects() |= 8;

			reinterpret_cast<BuildTransformations_t>(OriginalBuildTransformations)(ent, hdr, pos, q, cameraTransform, boneMask, boneComputed);

			if (flags & 8)
				ent->m_fEffects() |= 8;

			return;
		}

		reinterpret_cast<BuildTransformations_t>(OriginalBuildTransformations)(ent, hdr, pos, q, cameraTransform, boneMask, boneComputed);
	}
	_declspec(noinline)int SetViewmodelOffsets_Detour(void* ent, int something, float x, float y, float z)
	{
		/*if (ctx.m_settings.visuals_viewmodel_control[0] != 0 || ctx.m_settings.visuals_viewmodel_control[1] != 0 || ctx.m_settings.visuals_viewmodel_control[2] != 0)
		{
			x = ctx.m_settings.visuals_viewmodel_control[0];
			y = ctx.m_settings.visuals_viewmodel_control[1];
			z = ctx.m_settings.visuals_viewmodel_control[2];
		}*/

		return reinterpret_cast<SetViewmodelOffsets_t>(OriginalSetViewmodelOffsets)(ent, something, x, y, z);
	}
	int __fastcall SetViewmodelOffsets(void* ecx, void* edx, int something, float x, float y, float z)
	{
		return SetViewmodelOffsets_Detour(ecx, something, x, y, z);
	}
	float __fastcall GetScreenAspectRatio(void* pEcx, void* pEdx, int32_t iWidth, int32_t iHeight)
	{
		using FnAR = float(__thiscall*)(void*, void*, int32_t, int32_t);
		auto original = vmt.m_engine->VCall<FnAR>(101)(pEcx, pEdx, iWidth, iHeight);

		if (ctx.m_settings.effects_aspect_ratio != 0)
			return float(ctx.m_settings.effects_aspect_ratio) / 80.f;
		else
			return ((float)ctx.screen_size.x / (float)ctx.screen_size.y);
	}
	void __fastcall FireEvents(void* ecx, void* edx)
	{
		using FnAR = void(__thiscall*)(void*);
		static auto ofc = vmt.m_engine->VCall<FnAR>(59);

		if (!ctx.m_local() || ctx.m_local()->IsDead())
			return ofc(ecx);

		auto m_current_event = *reinterpret_cast<CEventInfo**>(reinterpret_cast<uintptr_t>(csgo.m_client_state()) + 0x4E6C);

		if (!m_current_event)
			return ofc(ecx);

		CEventInfo* next = nullptr;
		do
		{
			next = *(CEventInfo**)((uintptr_t)m_current_event + 0x38);
			m_current_event->fire_delay = 0.0f;

			m_current_event = next;

		} while (next);

		ofc(ecx);
	}

	void __fastcall GetColorModulation(IMaterial* material, void* edx, float* r, float* g, float* b)
	{
		(GetColorModulation_t(OriginalGetColorModulation))(material, r, g, b);

		if (ctx.m_settings.effects_brightness_adjustment == 2 && material != nullptr && !material->IsErrorMaterial() && !material->GetMaterialVarFlag(MATERIAL_VAR_UNUSED))
		{
			//const auto name = material->GetName();

			//// exclude stuff we dont want modulated
			//if (strstr(group, "Other") || strstr(name, "player") || strstr(name, "chams") ||
			//	strstr(name, "weapon") || strstr(name, "glow"))
			//	return;

			const auto group = hash_32_fnv1a_const(material->GetTextureGroupName());

			if (group != hash_32_fnv1a_const("World textures") && group != hash_32_fnv1a_const("StaticProp textures") && (group != hash_32_fnv1a_const("SkyBox textures")))
				return;

			const bool is_prop = (group == hash_32_fnv1a_const("StaticProp textures"));

			//const auto base_color = ctx.flt2color(ctx.m_settings.colors_world_color);
			Color base_color = Color(255 - ctx.m_settings.effects_night_adjustment * 2.5f, 255 - ctx.m_settings.effects_night_adjustment * 2.5f, 255 - ctx.m_settings.effects_night_adjustment * 2.5f);


			*r *= (is_prop ? (max(1, base_color.r() * 1.f) / 255.f) : (max(1, base_color.r()) / 255.f));
			*g *= (is_prop ? (max(1, base_color.g() * 1.f) / 255.f) : (max(1, base_color.g()) / 255.f));
			*b *= (is_prop ? (max(1, base_color.b() * 1.f) / 255.f) : (max(1, base_color.b()) / 255.f));
		}
	}
	bool /*__stdcall*/ IsUsingStaticPropDebugModes()
	{
		const auto org = reinterpret_cast<IsUsingStaticPropDebugModes_t>(OriginalIsUsingStaticPropDebugModes)();//OriginalIsUsingStaticPropDebugModes();

		return (/*ctx.m_settings.misc_visuals_world_modulation[1] ? true : */org);
	}
	bool __fastcall IsHLTV(IVEngineClient* _this, void* EDX)
	{
		static auto ofc = vmt.m_engine->VCall<isHLTV_t>(93);
		VIRTUALIZER_START;

		static auto const accumulate_layers_call = reinterpret_cast<void*>(Memory::Scan(sxor("client.dll"), sxor("84 C0 75 0D F6 87")));
		static auto const setup_velocity = reinterpret_cast<void*>(Memory::Scan(sxor("client.dll"), sxor("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80")));

		if ((_ReturnAddress() == accumulate_layers_call
			|| _ReturnAddress() == setup_velocity) && ctx.m_local() && !ctx.m_local()->IsDead())
			return true;

		VIRTUALIZER_END;

		return ofc(_this);
	}
	float old_remainder = 0;
	void __vectorcall CL_Move(float accumulated_extra_samples, bool bFinalTick)
	{
		ctx.is_in_teleport = false;
		ctx.skip_communication = true;

		if (ctx.m_local()) {
			ctx.ticks_allowed++;

			ctx.cmd_tickcount = csgo.m_globals()->tickcount;
			ctx.current_tickcount = csgo.m_globals()->tickcount;

			if (csgo.m_client_state() && csgo.m_client_state()->m_ptrNetChannel)
				ctx.out_sequence_nr = csgo.m_client_state()->m_ptrNetChannel->out_sequence_nr;
			else
				ctx.out_sequence_nr = 0;

			ctx.active_keybinds[1].mode = 0;
			ctx.active_keybinds[13].mode = 0;

			ctx.fakeducking_prev_state = ctx.fakeducking;
			ctx.fakeducking = ctx.get_key_press(ctx.m_settings.anti_aim_fakeduck_key) && ctx.m_local() != nullptr && ctx.m_local()->m_fFlags() & FL_ONGROUND && ctx.m_local()->m_flDuckSpeed() >= 4.0f;//&& int(1.0f / csgo.m_globals()->interval_per_tick) == 64;

			auto doubletap_toggled = ctx.get_key_press(ctx.m_settings.aimbot_double_tap_key);
			auto hideshots_toggled = ctx.get_key_press(ctx.m_settings.aimbot_hideshots_exploit_toggle);

			if (doubletap_toggled || hideshots_toggled) {
				ctx.exploit_allowed = true;
				ctx.has_exploit_toggled = true;


				if (ctx.main_exploit == 0)
					ctx.main_exploit = hideshots_toggled ? 1 : 2;
				else {
					if (!doubletap_toggled && ctx.main_exploit > 1 || ctx.prev_exploit_states[1] != hideshots_toggled && hideshots_toggled)
						ctx.main_exploit = 1;
					if (!hideshots_toggled && ctx.main_exploit < 2 || ctx.prev_exploit_states[0] != doubletap_toggled && doubletap_toggled)
						ctx.main_exploit = 2;
				}

				ctx.prev_exploit_states[0] = doubletap_toggled;
				ctx.prev_exploit_states[1] = hideshots_toggled;

				if (doubletap_toggled)
					ctx.active_keybinds[1].mode = ctx.m_settings.aimbot_double_tap_key.mode + 1;

				if (hideshots_toggled)
					ctx.active_keybinds[13].mode = ctx.m_settings.aimbot_hideshots_exploit_toggle.mode + 1;
			}
			else
			{
				ctx.ticks_allowed = 0;
				ctx.main_exploit = 0;
				ctx.exploit_allowed = false;
				ctx.has_exploit_toggled = false;
			}
		}
		if (ctx.has_exploit_toggled && !ctx.fakeducking)
		{
			auto can_charge = false;

			if (ctx.latest_weapon_data != nullptr)
			{
				auto charge = std::fmaxf(ctx.latest_weapon_data->flCycleTime, 0.5f);

				if (ctx.fakeducking_prev_state || ctx.do_autostop)
				{
					ctx.last_speedhack_time = csgo.m_globals()->realtime;
					can_charge = false;
				}

				if (std::fabsf(csgo.m_globals()->realtime - ctx.last_speedhack_time) >= charge)
					can_charge = true;
			}

			if (ctx.charged_commands < 15 && ctx.exploit_allowed && can_charge && m_weapon() != nullptr && !m_weapon()->IsGrenade())
			{
				ctx.is_charging = true;
				ctx.charged_commands++;
				ctx.last_time_charged = csgo.m_globals()->realtime;
				return;
			}
		}

		if (ctx.doubletap_now)
		{
			if (ctx.shifted_teleport_amount < ctx.shifting_amount)
			{
				++ctx.shifted_teleport_amount;
				(CL_Move_t(OriginalCL_Move))(0, 0);
				return;
			}

			ctx.doubletap_now = false;
			ctx.charged_commands = 0;
			//return (CL_Move_t(OriginalCL_Move))(accumulated_extra_samples, bFinalTick);
		}
		else
		{
			ctx.shifted_teleport_amount = 0;
		}

		(CL_Move_t(OriginalCL_Move))(accumulated_extra_samples, bFinalTick);

		if (!ctx.skip_communication && ctx.m_settings.aimbot_enabled)
		{
			if (!ctx.m_local()
				|| ctx.m_local()->IsDead()
				|| ctx.m_local()->m_fFlags() & 0x40
				|| csgo.m_game_rules() && csgo.m_game_rules()->IsFreezeTime())
				return;
			else if (csgo.m_client_state() != nullptr) {
				INetChannel* net_channel = csgo.m_client_state()->m_ptrNetChannel;
				if (net_channel != nullptr && !(csgo.m_client_state()->m_iChockedCommands % 4)) {
					const auto current_choke = net_channel->choked_packets;
					const auto out_sequence_nr = net_channel->out_sequence_nr;

					net_channel->choked_packets = 0;
					net_channel->out_sequence_nr = ctx.out_sequence_nr;

					net_channel->send_datagram(0);

					net_channel->out_sequence_nr = out_sequence_nr;
					net_channel->choked_packets = current_choke;
				}
			}
		}
	}
	void m_flSimulationTimeHook(CRecvProxyData* pData, void* pStruct, void* pOut)
	{
		//C_BasePlayer* m_player = (C_BasePlayer*)pStruct;

		if (m_flSimulationTime)
			m_flSimulationTime(pData, pStruct, pOut);
	}
	void m_flAbsYawHook(CRecvProxyData* pData, void* pStruct, void* pOut)
	{
		VIRTUALIZER_START;

		static auto m_hPlayer = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSRagdoll"), sxor("m_hPlayer"));
		const auto player_handle = reinterpret_cast<CBaseHandle*>(reinterpret_cast<DWORD>(pStruct) + m_hPlayer);
		const auto abs_yaw = Math::normalize_angle(pData->m_Value.m_Float);

		if (*player_handle != 0xFFFFFFFF && *player_handle != -1 && ctx.m_local() != nullptr)
		{
			C_BasePlayer* hplayer = dynamic_cast<C_BasePlayer*>(csgo.m_entity_list()->GetClientEntityFromHandle(*player_handle));

			if (hplayer && hplayer->GetIClientEntity() != nullptr)
			{
				auto* player = (C_BasePlayer*)hplayer->GetIClientEntity()->GetBaseEntity();
				if (player != nullptr && player->entindex() > 0 && player->entindex() < 64 && player->entindex() != ctx.m_local()->entindex() && player->m_iTeamNum() != ctx.m_local()->m_iTeamNum())
				{
					player_info info;
					resolver_records* r_log = &feature::resolver->player_records[player->entindex() - 1];

					if (r_log != nullptr && abs(csgo.m_globals()->tickcount - r_log->did_store_abs_yaw) > 15)
					{
						const auto delta = Math::angle_diff(abs_yaw, player->m_angEyeAngles().y);
						const auto r_method = delta > 0.f ? -1 : 1;

						if (abs(delta) > 0.f && abs(delta) < 65.f) {
							r_log->last_abs_yaw_side = r_method;

							if (abs(Math::AngleDiff(r_log->last_angle.y, player->m_angEyeAngles().y)) <= 120.f && (r_log->last_desync_delta - abs(delta)) > 5.f)
							{
								r_log->last_low_delta = abs(delta);
								r_log->last_low_delta_diff = r_log->last_desync_delta - abs(delta);

								if (r_log->last_low_delta < 41.f)
									r_log->last_low_delta_time = csgo.m_globals()->realtime;

								r_log->had_low_delta = true;
							}
							else
							{
								r_log->had_low_delta = false;
								r_log->last_low_delta = 60.f;
							}
						}

						if (abs(delta) > 30)
						{
							r_log->had_low_delta = false;
							r_log->last_low_delta = 60.f;
						}

						r_log->did_store_abs_yaw = csgo.m_globals()->tickcount;
					}
				}
			}
		}

		m_flAbsYaw(pData, pStruct, pOut);
		VIRTUALIZER_END;
	}
	void m_flPlaybackHook(CRecvProxyData* pData, void* pStruct, void* pOut)
	{
		m_flPlaybackRate(pData, pStruct, pOut);
		auto pAnimOverlay = (C_AnimationLayer*)pStruct;
		if (pAnimOverlay) {
			auto player = (C_BasePlayer*)pAnimOverlay->m_pOwner;
			if (!player || player != ctx.m_local() || (pAnimOverlay->m_nOrder != 4 && pAnimOverlay->m_nOrder != 5) || (csgo.m_globals()->realtime - ctx.last_time_layers_fixed) > 0.5f)
				return;

			pAnimOverlay->m_flPlaybackRate = ctx.local_layers[ANGLE_POSDELTA][pAnimOverlay->m_nOrder].m_flPlaybackRate;
		}
	}
	void m_SequenceHook(CRecvProxyData* pData, void* pStruct, void* pOut)
	{
		m_Sequence(pData, pStruct, pOut);
		auto pAnimOverlay = (C_AnimationLayer*)pStruct;
		if (pAnimOverlay) {
			auto player = (C_BasePlayer*)pAnimOverlay->m_pOwner;
			if (!player || player != ctx.m_local() || (pAnimOverlay->m_nOrder != 4 && pAnimOverlay->m_nOrder != 5) || (csgo.m_globals()->realtime - ctx.last_time_layers_fixed) > 0.5f)
				return;

			pAnimOverlay->m_nSequence = ctx.local_layers[ANGLE_POSDELTA][pAnimOverlay->m_nOrder].m_nSequence;
		}
	}
	void m_flWeightHook(CRecvProxyData* pData, void* pStruct, void* pOut)
	{
		m_flWeight(pData, pStruct, pOut);
		auto pAnimOverlay = (C_AnimationLayer*)pStruct;
		if (pAnimOverlay) {
			auto player = (C_BasePlayer*)pAnimOverlay->m_pOwner;
			if (!player || player != ctx.m_local() || (pAnimOverlay->m_nOrder != 4 && pAnimOverlay->m_nOrder != 5) || (csgo.m_globals()->realtime - ctx.last_time_layers_fixed) > 0.5f)
				return;

			pAnimOverlay->m_flWeight = ctx.local_layers[ANGLE_POSDELTA][pAnimOverlay->m_nOrder].m_flWeight;

		}
	}
	void m_flCycleHook(CRecvProxyData* pData, void* pStruct, void* pOut)
	{
		m_flCycle(pData, pStruct, pOut);
		auto pAnimOverlay = (C_AnimationLayer*)pStruct;
		if (pAnimOverlay) {
			auto player = (C_BasePlayer*)pAnimOverlay->m_pOwner;
			if (!player || player != ctx.m_local() || (pAnimOverlay->m_nOrder != 4 && pAnimOverlay->m_nOrder != 5) || (csgo.m_globals()->realtime - ctx.last_time_layers_fixed) > 0.5f)
				return;

			pAnimOverlay->m_flCycle = ctx.local_layers[ANGLE_POSDELTA][pAnimOverlay->m_nOrder].m_flCycle;

		}
	}
	int	__stdcall IsBoxVisible(const Vector& mins, const Vector& maxs)
	{
		static auto ofc = vmt.m_engine->VCall<IsBoxVisible_t>(32);

		if (!memcmp(_ReturnAddress(), _box_pttrn.operator std::string().c_str(), 10))
			return 1;

		return ofc(mins, maxs);
	}
	bool __fastcall OverrideConfig(IMaterialSystem* ecx, void* edx, MaterialSystem_Config_t& config, bool bForceUpdate)
	{
		static auto ofc = vmt.m_material_system->VCall<OverrideConfig_t>(21);

		if (ctx.m_settings.effects_brightness_adjustment == 1)
			config.m_nFullbright = true;

		return ofc(ecx, config, bForceUpdate);
	}
	float __fastcall GetAlphaModulation(IMaterial* ecx, void* edx)
	{
		static auto ofc = (GetAlphaModulation_t)OriginalGetAlphaModulation;

		if (!ecx || ecx->IsErrorMaterial())
			return ofc(ecx);

		const auto get_group = hash_32_fnv1a_const(ecx->GetTextureGroupName());

		if (get_group != hash_32_fnv1a_const("StaticProp textures"))
			return ofc(ecx);

		return 0.67f; //your value
	}
	_declspec(noinline)bool ShouldHitEntity_Detour(void* pThis, IClientEntity* pHandleEntity, int contentsMask)
	{
		if ((DWORD)pHandleEntity == 0x7f7fffff || (DWORD)pHandleEntity < 0x20000)
			return false;

		if (ctx.force_low_quality_autowalling)
		{
			//if (auto Entity = pHandleEntity->GetBaseEntity())
			//{
			//	// Entities have a "Disable Bone Followers" thingy
			//	//if (entity_cast<CBoneFollower>(Entity))	// Really stupid fix that I don't know why it's needed... Here, inside ShouldHitEntity it's a CBoneFollower but the trace contains a prop_dynamic
			//	//{											// If I search for GetModelKeyValueText I find some bone following shit but that still doesn't explain the discrepancy between this function and the trace
			//	//	if (auto Owner = Entity->GetOwnerEntity())
			//	//		pHandleEntity = Owner;
			//	//}
			//	if (hash_32_fnv1a(Entity->GetClientClass()->m_pNetworkName + strlen(Entity->GetClientClass()->m_pNetworkName) - 10, 10) == hash_32_fnv1a_const("Projectile")) // IsProjectile function maybe
			//		return false;
			//}

			if (pHandleEntity == feature::grenade_tracer->m_collision_ent)//feature::grenades->is_broken((C_BasePlayer*)pHandleEntity))
				return false;
		}

		const auto result = reinterpret_cast<ShouldHitEntity_t>(OriginalShouldHitEntity)(pThis, pHandleEntity, contentsMask);

		if (result && csgo.m_static_prop()->IsStaticProp(pHandleEntity))
			return false;

		return result;
	}
	bool __fastcall ShouldHitEntity(void* pThis, void* edx, IClientEntity* pHandleEntity, int contentsMask)
	{
		return ShouldHitEntity_Detour(pThis, pHandleEntity, contentsMask);
	}
	bool __fastcall IsConnected(void* ecx, void* edx)
	{
		/*string: "IsLoadoutAllowed"
		- follow up v8::FunctionTemplate::New function
		- inside it go to second function that is being called after "if" statement.
		- after that u need to open first function that is inside it. [before (*(int (**)(void))(*(_DWORD *)dword_152350E4 + 516))();]
		*/
		/*
		.text:103A2110 57                          push    edi
		.text:103A2111 8B F9                       mov     edi, ecx
		.text:103A2113 8B 0D AC E5+                mov     ecx, dword_14F8E5AC
		.text:103A2119 8B 01                       mov     eax, [ecx]
		.text:103A211B 8B 40 6C                    mov     eax, [eax+6Ch]
		.text:103A211E FF D0                       call    eax             ; Indirect Call Near Procedure
		.text:103A2120 84 C0                       test    al, al          ; Logical Compare <-
		.text:103A2122 75 04                       jnz     short loc_103A2128 ; Jump if Not Zero (ZF=0)
		.text:103A2124 B0 01                       mov     al, 1
		.text:103A2126 5F                          pop     edi
		*/

		using Fn = bool(__thiscall*)(void* ecx);
		static auto ofc = vmt.m_engine->VCall<Fn>(27);

		//static void* is_loadout_allowed = (void*)(Memory::Scan(sxor("client.dll"), sxor("84 C0 75 04 B0 01 5F")));

		if (csgo.m_engine()->IsInGame() && _ReturnAddress() == (void*)(Engine::Displacement::Signatures[c_signatures::RET_ISLOADOUT_ALLOWED]))
			return false;

		return ofc(ecx);
	}
	void impact_callback(const CEffectData& data)
	{
		auto org = data.origin;

		if (ctx.m_settings.effects_bullet_impact && !org.IsZero() && ctx.m_local() && fabs(ctx.last_shot_time_clientside - csgo.m_globals()->realtime) <= csgo.m_globals()->interval_per_tick)
			csgo.m_debug_overlay()->AddBoxOverlay(data.origin, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f), Vector(0.f, 0.f, 0.f), 255, 0, 0, 127, 4.f);

		oImpact(data);
	}
	int GetNewAnimation(const uint32_t model, const int sequence, C_BaseViewModel* viewModel) {

		// This only fixes if the original knife was a default knife.
		// The best would be having a function that converts original knife's sequence
		// into some generic enum, then another function that generates a sequence
		// from the sequences of the new knife. I won't write that.
		enum ESequence {
			SEQUENCE_DEFAULT_DRAW = 0,
			SEQUENCE_DEFAULT_IDLE1 = 1,
			SEQUENCE_DEFAULT_IDLE2 = 2,
			SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
			SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
			SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
			SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
			SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
			SEQUENCE_DEFAULT_LOOKAT01 = 12,

			SEQUENCE_BUTTERFLY_DRAW = 0,
			SEQUENCE_BUTTERFLY_DRAW2 = 1,
			SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
			SEQUENCE_BUTTERFLY_LOOKAT03 = 15,

			SEQUENCE_FALCHION_IDLE1 = 1,
			SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
			SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
			SEQUENCE_FALCHION_LOOKAT01 = 12,
			SEQUENCE_FALCHION_LOOKAT02 = 13,

			SEQUENCE_DAGGERS_IDLE1 = 1,
			SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
			SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
			SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
			SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,

			SEQUENCE_BOWIE_IDLE1 = 1,
		};

		auto random_sequence = [](const int low, const int high) -> int {
			return rand() % (high - low + 1) + low;
		};

		// Hashes for best performance.
		switch (model) {
		case hash_32_fnv1a_const(("models/weapons/v_knife_butterfly.mdl")):
		{
			switch (sequence) {
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
			default:
				return sequence + 1;
			}
		}
		case hash_32_fnv1a_const(("models/weapons/v_knife_falchion_advanced.mdl")):
		{
			switch (sequence) {
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_FALCHION_IDLE1;
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return random_sequence(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence - 1;
			}
		}
		case hash_32_fnv1a_const(("models/weapons/v_knife_push.mdl")):
		{
			switch (sequence) {
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_DAGGERS_IDLE1;
			case SEQUENCE_DEFAULT_LIGHT_MISS1:
			case SEQUENCE_DEFAULT_LIGHT_MISS2:
				return random_sequence(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return random_sequence(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
			case SEQUENCE_DEFAULT_HEAVY_HIT1:
			case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
			case SEQUENCE_DEFAULT_LOOKAT01:
				return sequence + 3;
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence + 2;
			}
		}
		case hash_32_fnv1a_const(("models/weapons/v_knife_survival_bowie.mdl")):
		{
			switch (sequence) {
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_BOWIE_IDLE1;
			default:
				return sequence - 1;
			}
		}
		case hash_32_fnv1a_const(("models/weapons/v_knife_ursus.mdl")):
		case hash_32_fnv1a_const(("models/weapons/v_knife_skeleton.mdl")):
		case hash_32_fnv1a_const(("models/weapons/v_knife_outdoor.mdl")):
		case hash_32_fnv1a_const(("models/weapons/v_knife_canis.mdl")):
		case hash_32_fnv1a_const(("models/weapons/v_knife_cord.mdl")):
		{
			switch (sequence) {
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
			default:
				return sequence + 1;
			}
		}
		case hash_32_fnv1a_const(("models/weapons/v_knife_stiletto.mdl")):
		{
			switch (sequence) {
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(12, 13);
			}
		}
		case hash_32_fnv1a_const(("models/weapons/v_knife_widowmaker.mdl")):
		{
			switch (sequence) {
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(14, 15);
			}
		}

		default:
			return sequence;
		}
	}

	static int fixed_p;

	void skinfix(int itemfix)
	{
		fixed_p = itemfix;
	}
	void DoSequenceRemapping(CRecvProxyData* data, C_BaseViewModel* entity) {

		if (!csgo.m_engine()->IsInGame() || !ctx.m_local() || ctx.m_local()->IsDead() || parser::knifes.list.empty())
			return;

		auto* const owner = csgo.m_entity_list()->GetClientEntityFromHandle((CBaseHandle)entity->m_hOwner());
		if (owner != ctx.m_local() || entity->get_viewmodel_weapon() == -1)
			return;

		auto* const view_model_weapon = (C_WeaponCSBaseGun*)csgo.m_entity_list()->GetClientEntityFromHandle(entity->get_viewmodel_weapon());

		if (!view_model_weapon || !view_model_weapon->is_knife())
			return;

		/*auto idx = view_model_weapon->m_iItemDefinitionIndex();

		const auto* const override_model = parser::knifes.list[ctx.m_settings.skinchanger_knife].model_player_path.c_str();

		auto& sequence = data->m_Value.m_Int;
		sequence = GetNewAnimation(hash_32_fnv1a_const(override_model), sequence, entity);*/

		const auto entry = k_weapon_info.find(view_model_weapon->m_iItemDefinitionIndex());

		if (entry == k_weapon_info.end())
			return;

		if (&entry->second == nullptr)
			return;

		const auto weaponInfo = &entry->second;
		auto& sequence = data->m_Value.m_Int;
		sequence = GetNewAnimation(hash_32_fnv1a_const(weaponInfo->model), sequence, entity);
	}
	void m_nViewModel(CRecvProxyData* pData, void* pStruct, void* pOut)
	{


		int default_t = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_default_t.mdl");
		int default_ct = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
		int iBayonet = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
		int iButterfly = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
		int iFlip = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_flip.mdl");
		int iGut = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_gut.mdl");
		int iKarambit = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_karam.mdl");
		int iM9Bayonet = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
		int iHuntsman = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_tactical.mdl");
		int iFalchion = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
		int iDagger = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_push.mdl");
		int iBowie = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
		int iGunGame = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_gg.mdl");
		int Navaja = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_gypsy_jackknife.mdl");
		int Stiletto = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_stiletto.mdl");
		int Ursus = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_ursus.mdl");
		int Talon = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_widowmaker.mdl");
		int d1 = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_css.mdl");
		int d2 = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_skeleton.mdl");
		int d3 = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_outdoor.mdl");
		int d4 = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_canis.mdl");
		int d5 = csgo.m_model_info()->GetModelIndex("models/weapons/v_knife_cord.mdl");


		const auto local = ctx.m_local();


		if (local)
		{
			if (!local->IsDead() && (
				pData->m_Value.m_Int == default_t ||
				pData->m_Value.m_Int == default_ct ||
				pData->m_Value.m_Int == iBayonet ||
				pData->m_Value.m_Int == iFlip ||
				pData->m_Value.m_Int == iGunGame ||
				pData->m_Value.m_Int == iGut ||
				pData->m_Value.m_Int == iKarambit ||
				pData->m_Value.m_Int == iM9Bayonet ||
				pData->m_Value.m_Int == iHuntsman ||
				pData->m_Value.m_Int == iBowie ||
				pData->m_Value.m_Int == iButterfly ||
				pData->m_Value.m_Int == iFalchion ||
				pData->m_Value.m_Int == iDagger ||
				pData->m_Value.m_Int == Navaja ||
				pData->m_Value.m_Int == Stiletto ||
				pData->m_Value.m_Int == Ursus ||
				pData->m_Value.m_Int == Talon ||
				pData->m_Value.m_Int == d1 ||
				pData->m_Value.m_Int == d2 ||
				pData->m_Value.m_Int == d3 ||
				pData->m_Value.m_Int == d4 ||
				pData->m_Value.m_Int == d5))
			{


				if (fixed_p)
					pData->m_Value.m_Int = fixed_p;




			}
		}
		if (m_ViewModel)
			m_ViewModel(pData, pStruct, pOut);

	}
	void m_nSequenceHook(CRecvProxyData* proxy_data_const, void* entity, void* output) {
		// Remove the constness from the proxy data allowing us to make changes.
		auto* const proxy_data = const_cast<CRecvProxyData*>(proxy_data_const);

		auto* const view_model = static_cast<C_BaseViewModel*>(entity);

		DoSequenceRemapping(proxy_data, view_model);

		// Call the original function with our edited data.
		m_nSequence(proxy_data_const, entity, output);
	}
	void m_flVelocityModifierHook(CRecvProxyData* data, void* entity, void* output) {
		m_flVelocityModifier(data, entity, output);
	}
	void m_vecForceHook(CRecvProxyData* pData, void* pStruct, void* pOut) {
		// convert to ragdoll.
		static auto m_hPlayer = Engine::PropManager::Instance()->GetOffset(sxor("DT_CSRagdoll"), sxor("m_hPlayer"));
		const auto player_handle = reinterpret_cast<CBaseHandle*>(reinterpret_cast<DWORD>(pStruct) + m_hPlayer);
		player_info info;

		//if (*player_handle != 0xFFFFFFFF && *player_handle != -1 && ctx.m_local() != nullptr && ctx.m_settings.misc_ragdoll_force)
		//{
		//	//if (ctx.m_local()->m_iTeamNum() ) {
		//		// get m_vecForce.
		//		Vector vel = { pData->m_Value.m_Vector[0], pData->m_Value.m_Vector[1], pData->m_Value.m_Vector[2] };

		//		vel.x = 0;
		//		vel.y = 0;
		//		if (vel.z <= 1.f)
		//			vel.z = 2.f;

		//		// give some speed to all directions.
		//		vel.z *= 2000.f;

		//		// boost z up a bit.
		//		//if (vel.z <= 1.f)
		//		//	vel.z = 2.f;

		//		//vel.z *= 2.f;

		//		// don't want crazy values for this... probably unlikely though?
		//		//Math::clamp(vel.x, FLT_MIN, FLT_MAX);
		//		//Math::clamp(vel.y, FLT_MIN, FLT_MAX);
		//		Math::clamp(vel.z, FLT_MIN, FLT_MAX);

		//		// set new velocity.
		//		pData->m_Value.m_Vector[0] = vel.x;
		//		pData->m_Value.m_Vector[1] = vel.y;
		//		pData->m_Value.m_Vector[2] = vel.z;
		//	//}
		//}

		if (m_vecForce)
			m_vecForce(pData, pStruct, pOut);
	}
	unsigned long __fastcall GCRetrieveMessageHook(void* ecx, void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
	{
		//static auto oGCRetrieveMessage = gc.get_original<GCRetrieveMessage>(2);
		auto status = oGCRetrieveMessage(ecx, punMsgType, pubDest, cubDest, pcubMsgSize);
		if (status == k_EGCResultOK)
		{

			void* thisPtr = nullptr;
			__asm mov thisPtr, ebx;
			auto oldEBP = *reinterpret_cast<void**>((uint32_t)_AddressOfReturnAddress() - 4);

			uint32_t messageType = *punMsgType & 0x7FFFFFFF;
			write.ReceiveMessage(thisPtr, oldEBP, messageType, pubDest, cubDest, pcubMsgSize);
		}
		return status;
	}
	unsigned long __fastcall GCSendMessageHook(void* ecx, void*, uint32_t unMsgType, const void* pubData, uint32_t cubData)
	{
		//static auto oGCSendMessage = gc.get_original<GCSendMessage>(0);
		bool sendMessage = write.PreSendMessage(unMsgType, const_cast<void*>(pubData), cubData);
		if (!sendMessage)
			return k_EGCResultOK;
		return oGCSendMessage(ecx, unMsgType, const_cast<void*>(pubData), cubData);
	}
	int __fastcall AddRenderable(void* ecx, void* edx, IClientRenderable* p_renderable, int unk1, RenderableTranslucencyType_t n_type, int unk2, int unk3)
	{
		if (!p_renderable)
			return oAddRenderable(ecx, p_renderable, unk1, n_type, unk2, unk3);

		auto i_client_unknown = p_renderable->GetIClientUnknown();
		if (!i_client_unknown)
			return oAddRenderable(ecx, p_renderable, unk1, n_type, unk2, unk3);

		auto c_base_entity = i_client_unknown->GetBaseEntity();
		if (!c_base_entity)
			return oAddRenderable(ecx, p_renderable, unk1, n_type, unk2, unk3);

		auto p_entity = (C_BaseEntity*)c_base_entity;
		if (!p_entity)
			return oAddRenderable(ecx, p_renderable, unk1, n_type, unk2, unk3);

		if (!p_entity->IsPlayer())
			return oAddRenderable(ecx, p_renderable, unk1, n_type, unk2, unk3);

		n_type = RENDERABLE_IS_TRANSLUCENT;
		return oAddRenderable(ecx, p_renderable, unk1, n_type, unk2, unk3);
	}

	void __fastcall hkPerformScreenOverlay(void* ecx, void* edx, int x, int y, int w, int h)
	{
		return;
		//return oPerformScreenOverlay(ecx, edx, x, y, w, h);
	}
	bool __fastcall hkDispatchUserMessage(void* ecx, void* edx, int type, unsigned int a3, unsigned int length, const void* msg_data)
	{
		if (csgo.m_game_rules())
		{
			if (!csgo.m_game_rules()->IsValveDS())
			{
				if ((type == 7 || type == 8 || type == 5)) {
					return true;
				}
			}
		}

		return otDispatchUserMessage(ecx, type, a3, length, msg_data);
	}
}

bool create_log_file(std::string error_message)
{
	std::fstream file(sxor("C:\\crashlog\\error.log"), std::ios::out | std::ios::in | std::ios::trunc);
	file.close();

	file.open(sxor("C:\\crashlog\\error.log"), std::ios::out | std::ios::in);
	if (!file.is_open())
	{
		file.close();
		return false;
	}

	file.clear();

	file << error_message;

	file.close();

	return true;
}
template <typename T>
T* CaptureInterface(HANDLE modulehandle, const char* strInterface)
{
	typedef T* (*CreateInterfaceFn)(const char* szName, int iReturn);
	CreateInterfaceFn CreateInterface = (CreateInterfaceFn)GetProcAddress((HMODULE)(modulehandle), sxor("CreateInterface"));
	return CreateInterface(strInterface, 0);
}
class InterfaceReg
{
private:
	using InstantiateInterfaceFn = void* (*)();
public:
	InstantiateInterfaceFn m_CreateFn;
	const char* m_pName;
	InterfaceReg* m_pNext;
};
template<typename T>
static T* get_interface(const char* mod_name, const char* interface_name, bool exact = false) {
	T* iface = nullptr;
	InterfaceReg* register_list;
	int part_match_len = strlen(interface_name);

	DWORD interface_fn = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA(mod_name), sxor("CreateInterface")));

	if (!interface_fn) {
		return nullptr;
	}

	unsigned int jump_start = (unsigned int)(interface_fn)+4;
	unsigned int jump_target = jump_start + *(unsigned int*)(jump_start + 1) + 5;

	register_list = **reinterpret_cast<InterfaceReg***>(jump_target + 6);

	for (InterfaceReg* cur = register_list; cur; cur = cur->m_pNext) {
		if (exact == true) {
			if (strcmp(cur->m_pName, interface_name) == 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
		else {
			if (!strncmp(cur->m_pName, interface_name, part_match_len) && std::atoi(cur->m_pName + part_match_len) > 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
	}
	return iface;
}

namespace Source
{
	HWND Window = nullptr;

	DECLSPEC_NOINLINE bool Create()
	{
		VIRTUALIZER_START;
		auto& pPropManager = Engine::PropManager::Instance();

		csgo.m_client.set((IBaseClientDLL*)CreateInterface("client.dll", "VClient"));

		if (!csgo.m_client())
		{
#ifdef DEBUG
			Win32::Error("IBaseClientDLL is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		if (!pPropManager->Create(csgo.m_client()))
		{
#ifdef DEBUG
			Win32::Error("Engine::PropManager::Create failed (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		if (!Engine::Displacement::Create())
		{
#ifdef DEBUG
			Win32::Error("Engine::Displacement::Create failed (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_entity_list.set((IClientEntityList*)CreateInterface("client.dll", "VClientEntityList"));

		if (!csgo.m_entity_list())
		{
#ifdef DEBUG
			Win32::Error("IClientEntityList is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_movement.set((IGameMovement*)CreateInterface("client.dll", "GameMovement"));

		if (!csgo.m_movement())
		{
#ifdef DEBUG
			Win32::Error("IGameMovement is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_prediction.set((IPrediction*)CreateInterface("client.dll", "VClientPrediction"));

		if (!csgo.m_prediction())
		{
#ifdef DEBUG
			Win32::Error("IPrediction is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_move_helper.set((IMoveHelper*)(Engine::Displacement::Signatures[MOVEHELPER]));

		if (!csgo.m_move_helper())
		{
#ifdef DEBUG
			Win32::Error("IMoveHelper is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_input.set((IInput*)(Engine::Displacement::Signatures[IINPUT]));

		if (!csgo.m_input())
		{
#ifdef DEBUG
			Win32::Error("IInput is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_globals.set(**(CGlobalVarsBase***)((*(DWORD**)csgo.m_client())[0] + 0x1F));

		if (!csgo.m_globals())
		{
#ifdef DEBUG
			Win32::Error("CGlobalVars is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_engine.set((IVEngineClient*)CreateInterface("engine.dll", "VEngineClient"));

		if (!csgo.m_engine())
		{
#ifdef DEBUG
			Win32::Error("IVEngineClient is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_panel.set((IPanel*)CreateInterface("vgui2.dll", "VGUI_Panel"));

		if (!csgo.m_panel())
		{
#ifdef DEBUG
			Win32::Error("IPanel is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_surface.set((ISurface*)CreateInterface("vguimatsurface.dll", "VGUI_Surface"));

		if (!csgo.m_surface())
		{
#ifdef DEBUG
			Win32::Error("ISurface is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_engine_vgui.set((IEngineVGui*)CreateInterface("engine.dll", "VEngineVGui001", true));

		if (!csgo.m_engine_vgui())
		{
#ifdef DEBUG
			Win32::Error("IEngineVGui is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_client_state.set((CClientState*)(**(std::uintptr_t**)(Engine::Displacement::Signatures[CLIENT_STATE] + 1))); //ik we can grab it from vfunc but imma just took what i did b4

		if (!csgo.m_client_state())
		{
#ifdef DEBUG
			Win32::Error("CClientState is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_engine_cvars.set((ICvar*)CreateInterface("vstdlib.dll", "VEngineCvar"));

		if (!csgo.m_engine_cvars())
		{
#ifdef DEBUG
			Win32::Error("ICvar is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_engine_trace.set((IEngineTrace*)CreateInterface("engine.dll", "EngineTraceClient004", true));

		if (!csgo.m_engine_trace())
		{
#ifdef DEBUG
			Win32::Error("IEngineTrace is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_model_info.set((IVModelInfo*)CreateInterface("engine.dll", "VModelInfoClient004", true));

		if (!csgo.m_model_info())
		{
#ifdef DEBUG
			Win32::Error("IVModelInfo is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_input_system.set((InputSystem*)CreateInterface("inputsystem.dll", "InputSystemVersion001", true));

		if (!csgo.m_input_system())
		{
#ifdef DEBUG
			Win32::Error("IInput is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_model_render.set((IVModelRender*)(CreateInterface("engine.dll", "VEngineModel016", true)));

		if (!csgo.m_model_render())
		{
#ifdef DEBUG
			Win32::Error("IVModelRender is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_studio_render.set((StudioRender*)(CreateInterface("studiorender.dll", "VStudioRender026", true)));

		if (!csgo.m_studio_render())
		{
#ifdef DEBUG
			Win32::Error("studiorender is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}
		csgo.m_render_view.set((IVRenderView*)CreateInterface("engine.dll", "VEngineRenderView014", true));

		if (!csgo.m_render_view())
		{
#ifdef DEBUG
			Win32::Error("IVRenderView is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_material_system.set((IMaterialSystem*)CreateInterface("materialsystem.dll", "VMaterialSystem080", true));

		if (!csgo.m_material_system())
		{
#ifdef DEBUG
			Win32::Error("IMaterialSystem is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_phys_props.set((IPhysicsSurfaceProps*)CreateInterface("vphysics.dll", "VPhysicsSurfaceProps001", true));

		if (!csgo.m_phys_props())
		{
#ifdef DEBUG
			Win32::Error("IPhysicsSurfaceProps is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_debug_overlay.set((IVDebugOverlay*)CreateInterface("engine.dll", "VDebugOverlay004", true));

		if (!csgo.m_debug_overlay())
		{
#ifdef DEBUG
			Win32::Error("IVDebugOverlay is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_event_manager.set((IGameEventManager*)(CreateInterface("engine.dll", "GAMEEVENTSMANAGER002", true)));

		if (!csgo.m_event_manager())
		{
#ifdef DEBUG
			Win32::Error("IGameEventManager is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_static_prop.set((IStaticPropMgr*)(CreateInterface("engine.dll", "StaticPropMgrServer002", true)));

		if (!csgo.m_static_prop())
		{
#ifdef DEBUG
			Win32::Error("IStaticPropMgr is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_beams.set(*(IViewRenderBeams**)(Engine::Displacement::Signatures[BEAMS] + 1));

		if (!csgo.m_beams())
		{
#ifdef DEBUG
			Win32::Error("IViewRenderBeams is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_localize.set((ILocalize*)(CreateInterface("localize.dll", "Localize_001", true)));

		if (!csgo.m_localize())
		{
#ifdef DEBUG
			Win32::Error("ILocalize is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}
		csgo.m_filesystem.set(get_interface<IBaseFileSystem>("filesystem_stdio.dll", "VBaseFileSystem"));

		if (!csgo.m_filesystem())
		{
#ifdef DEBUG
			Win32::Error("IBaseFileSystem is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}
		csgo.m_glow_object.set(*reinterpret_cast<CGlowObjectManager**>(Engine::Displacement::Signatures[GLOW_OBJECTS] + 3));

		if (!csgo.m_glow_object())
		{
#ifdef DEBUG
			Win32::Error("CGlowObjectManager is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_mdl_cache.set((IMDLCache*)(CreateInterface("datacache.dll", "MDLCache004", true)));

		if (!csgo.m_mdl_cache())
		{
#ifdef DEBUG
			Win32::Error("IMDLCache is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_engine_sound.set((IEngineSound*)(CreateInterface("engine.dll", "IEngineSoundClient")));

		if (!csgo.m_engine_sound())
		{
#ifdef DEBUG
			Win32::Error("IEngineSound is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		csgo.m_mem_alloc.set(*(IMemAlloc**)(GetProcAddress(GetModuleHandleA(("tier0.dll")), ("g_pMemAlloc"))));

		if (!csgo.m_mem_alloc())
		{
#ifdef DEBUG
			Win32::Error("IMemAlloc is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			return false;
		}

		ctx.init_finished = false;

		for (CClientEffectRegistration* head = ctx.m_effects_head(); head; head = head->next)
		{
			if (strstr(head->effectName, ("Impact")) && strlen(head->effectName) <= 8) {
				oImpact = head->function;
				head->function = &Hooked::impact_callback;
				break;
			}
		}

		feature::menu = new c_menu;
		feature::misc = new c_misc;
		feature::anti_aim = new c_antiaimbot;
		feature::resolver = new c_resolver;
		feature::visuals = new c_visuals;
		feature::usercmd = new c_usercmd;
		feature::lagcomp = new c_lagcomp;
		feature::chams = new c_chams;
		feature::autowall = new c_autowall;
		feature::ragebot = new c_aimbot;
		feature::sound_parser = new c_dormant_esp;
		feature::music_player = new c_music_player;
		feature::weather = new c_weather_controller;
		feature::legitbot = new c_legitaimbot;
		feature::hitchams = new c_hit_chams;
		feature::grenade_prediction = new c_grenade_prediction;
		feature::grenade_tracer = new c_grenade_tracer;
		feature::net_channel = new c_net_channel;

		ctx.last_shot_info.point.clear();
		ctx.last_shot_info.hit = false;
		ctx.last_shot_info.find = false;
		ctx.last_shot_info.last_time_hit = 0;
		ctx.last_shot_info.resolver_index = 0;
		ctx.last_shot_info.entindex = -1;

		auto m_pClientMode = **(void***)((*(DWORD**)csgo.m_client())[10] + 5);

		if (m_pClientMode)
		{
			vmt.m_clientmode = std::make_shared<Memory::VmtSwap>(m_pClientMode);
			vmt.m_clientmode->Hook(&Hooked::CreateMove, 24);
			vmt.m_clientmode->Hook(&Hooked::OverrideView, 18);
			vmt.m_clientmode->Hook(&Hooked::DoPostScreenEffects, 44);
			vmt.m_clientmode->Hook(&Hooked::GetViewModelFOV, 35);
		}
		else
		{
#ifdef DEBUG
			Win32::Error("IClientMode is nullptr (Source::%s)", __FUNCTION__);
#endif // DEBUG
			Destroy();
			create_log_file(sxor("ERROR 30"));
			return false;
		}

		vmt.m_client = std::make_shared<Memory::VmtSwap>(csgo.m_client());
		vmt.m_prediction = std::make_shared<Memory::VmtSwap>(csgo.m_prediction());
		vmt.m_panel = std::make_shared<Memory::VmtSwap>(csgo.m_panel());
		vmt.m_surface = std::make_shared<Memory::VmtSwap>(csgo.m_surface());
		vmt.m_engine_vgui = std::make_shared<Memory::VmtSwap>(csgo.m_engine_vgui());
		vmt.m_render_view = std::make_shared<Memory::VmtSwap>(csgo.m_render_view());
		vmt.m_engine = std::make_shared<Memory::VmtSwap>(csgo.m_engine());
		vmt.m_model_render = std::make_shared<Memory::VmtSwap>(csgo.m_model_render());
		vmt.m_material_system = std::make_shared<Memory::VmtSwap>(csgo.m_material_system());
		vmt.m_engine_trace = std::make_shared<Memory::VmtSwap>(csgo.m_engine_trace());
		vmt.m_bsp_tree_query = std::make_shared<Memory::VmtSwap>(csgo.m_engine()->GetBSPTreeQuery());
		vmt.m_engine_sound = std::make_shared<Memory::VmtSwap>(csgo.m_engine_sound());
		vmt.m_movement = std::make_shared<Memory::VmtSwap>(csgo.m_movement());
		vmt.m_device = std::make_shared<Memory::VmtSwap>((void*)Engine::Displacement::Signatures[c_signatures::DX9DEVICE]);
		vmt.m_net_channel = std::make_shared<Memory::VmtSwap>((DWORD**)csgo.m_client_state()->m_ptrNetChannel);
		vmt.m_clientstate = std::make_shared<Memory::VmtSwap>((CClientState*)(uint32_t(csgo.m_client_state()) + 8));

		vmt.m_engine->Hook(&Hooked::IsHLTV, 93);
		vmt.m_engine->Hook(&Hooked::IsBoxVisible, 32);
		vmt.m_engine->Hook(&Hooked::IsConnected, 27);
		vmt.m_bsp_tree_query->Hook(&Hooked::ListLeavesInBox, 6);
		vmt.m_engine->Hook(&Hooked::GetNetChannelInfo, 78);
		vmt.m_engine->Hook(&Hooked::GetScreenAspectRatio, 101);
		vmt.m_engine->Hook(&Hooked::FireEvents, 59);

		vmt.m_movement->Hook(&Hooked::ProcessMovement, 1);

		vmt.m_device->Hook(&Hooked::EndScene, Index::IDirectX::EndScene);
		vmt.m_device->Hook(&Hooked::Reset, Index::IDirectX::Reset);

		vmt.m_engine_sound->Hook(&Hooked::EmitSound1, 5);
		vmt.m_engine_trace->Hook(&Hooked::ClipRayCollideable, 4);

		vmt.m_client->Hook(&Hooked::FrameStageNotify, Index::IBaseClientDLL::FrameStageNotify);
		vmt.m_client->Hook(&Hooked::WriteUsercmdDeltaToBuffer, 24);

		vmt.m_client->Hook(&Hooked::LevelInitPostEntity, 6);
		vmt.m_client->Hook(&Hooked::LevelInitPreEntity, 5);
		vmt.m_client->Hook(&Hooked::LevelShutdown, 7);

		vmt.m_surface->Hook(&Hooked::LockCursor, Index::ISurface::LockCursor);
		vmt.m_surface->Hook(&Hooked::DrawSetColor, 15);
		//vmt.m_prediction->Hook(&Hooked::SetupMove, Index::IPrediction::SetupMove);
		vmt.m_prediction->Hook(&Hooked::RunCommand, Index::IPrediction::RunCommand);
		vmt.m_prediction->Hook(&Hooked::InPrediction, Index::IPrediction::InPrediction);

		vmt.m_panel->Hook(&Hooked::PaintTraverse, Index::IPanel::PaintTraverse);
		vmt.m_engine_vgui->Hook(&Hooked::EngineVGUI_Paint, 14); //index is not in enum idk why

		vmt.m_model_render->Hook(&Hooked::DrawModelExecute, 21);
		vmt.m_material_system->Hook(&Hooked::OverrideConfig, 21);

		auto r_jiggle_bones = csgo.m_engine_cvars()->FindVar(sxor("r_jiggle_bones"));
		csgo.m_engine_cvars()->FindVar(sxor("sv_allowupload"))->SetValue(true);

		r_jiggle_bones->SetValue(0);

		typedef uint32_t SteamPipeHandle;
		typedef uint32_t SteamUserHandle;
		SteamUserHandle hSteamUser = ((SteamUserHandle(__cdecl*)(void))GetProcAddress(GetModuleHandle(sxor("steam_api.dll")), sxor("SteamAPI_GetHSteamUser")))();
		SteamPipeHandle hSteamPipe = ((SteamPipeHandle(__cdecl*)(void))GetProcAddress(GetModuleHandle(sxor("steam_api.dll")), sxor("SteamAPI_GetHSteamPipe")))();
		SteamClient = ((ISteamClient * (__cdecl*)(void))GetProcAddress(GetModuleHandle(sxor("steam_api.dll")), sxor("SteamClient")))();
		SteamGameCoordinator = (ISteamGameCoordinator*)SteamClient->GetISteamGenericInterface(hSteamUser, hSteamPipe, sxor("SteamGameCoordinator001"));
		SteamUser = (ISteamUser*)SteamClient->GetISteamUser(hSteamUser, hSteamPipe, sxor("SteamUser019"));
		SteamFriends = SteamClient->GetISteamFriends(hSteamUser, hSteamPipe, sxor("SteamFriends015"));
		static auto SteamInventory = SteamClient->GetISteamInventory(hSteamUser, hSteamPipe, sxor("STEAMINVENTORY_INTERFACE_V002"));
		auto _ = SteamFriends->GetLargeFriendAvatar(SteamUser->GetSteamID());
		SteamUtils = SteamClient->GetISteamUtils(hSteamPipe, sxor("SteamUtils009"));

		auto cl_smooth = csgo.m_engine_cvars()->FindVar(sxor("cl_smooth"));
		//auto cl_grenadepreview = csgo.m_engine_cvars()->FindVar(sxor("cl_grenadepreview"));
		ctx.cv_console_window_open = csgo.m_engine_cvars()->FindVar(sxor("console_window_open"));

		game_events::init();

		feature::music_player->init();

		ctx.fix_modify_eye_pos = false;
		ctx.can_store_netvars = false;
		ctx.process_movement_sound_fix = false;
		ctx.breaks_lc = false;

		feature::ragebot->precomputed_seeds.fill(std::tuple<float, float, float, float, float>(0.f, 0.f, 0.f, 0.f, 0.f));
		feature::ragebot->precomputed_mini_seeds.fill(std::tuple<float, float, float, float, float>(0.f, 0.f, 0.f, 0.f, 0.f));
		feature::ragebot->build_mini_hc_table();
		feature::ragebot->build_seed_table();

		ctx.active_keybinds_visible = 0;
		ctx.main_exploit = 0;
		ctx.last_velocity_modifier = -1.f;
		ctx.cheat_option_flags = 0;
		ctx.prev_exploit_states[0] = false;
		ctx.prev_exploit_states[1] = false;

		//m_flSimulationTime = pPropManager->Hook(Hooked::m_flSimulationTimeHook, ("DT_BaseEntity"), ("m_flSimulationTime"));
		m_flAbsYaw = pPropManager->Hook(Hooked::m_flAbsYawHook, sxor("DT_CSRagdoll"), sxor("m_flAbsYaw"));
		m_nSequence = pPropManager->Hook(Hooked::m_nSequenceHook, sxor("DT_BaseViewModel"), sxor("m_nSequence"));
		m_ViewModel = pPropManager->Hook(Hooked::m_nViewModel, sxor("DT_BaseViewModel"), sxor("m_nModelIndex"));
		m_vecForce = pPropManager->Hook(Hooked::m_vecForceHook, sxor("DT_CSRagdoll"), sxor("m_vecForce"));
		m_flVelocityModifier = pPropManager->Hook(Hooked::m_flVelocityModifierHook, sxor("DT_CSPlayer"), sxor("m_flVelocityModifier"));

		static auto PhysicsSimulate = (DWORD)(Memory::Scan(sxor("client.dll"), sxor("56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23")));
		OriginalPhysicsSimulate = (DWORD)DetourFunction((byte*)PhysicsSimulate, (byte*)Hooked::PhysicsSimulate);

		static auto BuildTransformations = (DWORD)(Memory::Scan(sxor("client.dll"), sxor("55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 89 7C 24 1C")));
		OriginalBuildTransformations = (DWORD)DetourFunction((byte*)BuildTransformations, (byte*)Hooked::BuildTransformations);

		static auto ShouldFlipModel = (DWORD)(Memory::Scan(sxor("client.dll"), sxor("8B 89 ? ? ? ? 56 57 83 F9 FF")));
		OriginalShouldFlipModel = (DWORD)DetourFunction((byte*)ShouldFlipModel, (byte*)Hooked::ShouldFlipModel);

		static auto CheckForSequenceChange = (DWORD)(Memory::Scan(sxor("client.dll"), sxor("55 8B EC 51 53 8B 5D 08 56 8B F1 57 85")));
		OriginalCheckForSequenceChange = (DWORD)DetourFunction((byte*)CheckForSequenceChange, (byte*)Hooked::CheckForSequenceChange);

		OriginalStandardBlendingRules = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_STANDARDBLENDINGRULES], (byte*)Hooked::StandardBlendingRules);
		OriginalDoExtraBonesProcessing = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_DOEXTRABONESPROCESSING], (byte*)Hooked::DoExtraBonesProcessing);
		OriginalGetForeignFallbackFontName = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_GETFOREIGNFALLBACKFONT], (byte*)Hooked::GetForeignFallbackFontName);
		OriginalUpdateClientSideAnimations = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_UPDATECLIENTSIDEANIMS], (byte*)Hooked::UpdateClientSideAnimation);
		OriginalSetupBones = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_SETUPBONES], (byte*)Hooked::SetupBones);
		OriginalCalcViewBob = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_CALCVIEWBOB], (byte*)Hooked::CalcViewBob);
		OriginalAddViewModelBob = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_ADDVIEWMODELBOB], (byte*)Hooked::AddViewModelBob);
		OriginalShouldSkipAnimationFrame = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_SHOULDSKIPANIMFRAME], (byte*)Hooked::ShouldSkipAnimationFrame);
		OriginalModifyEyePosition = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_MODIFYEYEPOS], (byte*)Hooked::ModifyEyePosition);
		OriginalCL_Move = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_CL_MOVE], (byte*)Hooked::CL_Move);
		OriginalProcessInterpolatedList = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_PROCESSINTERPLIST], (byte*)Hooked::ProcessInterpolatedList);
		OriginalSetViewmodelOffsets = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_SETVIEWMODELOFFSETS], (byte*)Hooked::SetViewmodelOffsets);
		OriginalGetColorModulation = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_GETCOLORMODULATION], (byte*)Hooked::GetColorModulation);
		OriginalIsUsingStaticPropDebugModes = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_ISUSINGSTATICPROPDBGMODES], (byte*)Hooked::IsUsingStaticPropDebugModes);

		OriginalReportHit = (DWORD)DetourFunction((byte*)Engine::Displacement::Signatures[c_signatures::PTR_REPORTHIT], (byte*)Hooked::ReportHit);

		static auto PerformScreenOverlay = (DWORD)(Memory::Scan(sxor("client.dll"), sxor("55 8b ec 51 a1 ? ? ? ? 53 56 8b d9")));
		OriginalPerformScreenOverlay = (DWORD)DetourFunction((byte*)PerformScreenOverlay, (byte*)Hooked::hkPerformScreenOverlay);



		IClientLeafSystem* ClientLeafSystem = CaptureInterface<IClientLeafSystem>(GetModuleHandle("client.dll"), ("ClientLeafSystem002"));
		g_pIClientLeafSystem = zget_interface<IClientLeafSystem>(("client.dll"), ("ClientLeafSystem"));

		const auto sendmessage_target = reinterpret_cast<void*>(get_virtual(SteamGameCoordinator, 0));
		const auto retrievemessage_target = reinterpret_cast<void*>(get_virtual(SteamGameCoordinator, 2));
		const auto ClientLeafSystem_target = reinterpret_cast<void*>(get_virtual(ClientLeafSystem, 7));
		static auto CViewRender_PerformScreenOverlay = Memory::Scan("client.dll", "55 8b ec 51 a1 ? ? ? ? 53 56 8b d9");
		const auto dispatch_target = reinterpret_cast<void*>(get_virtual(csgo.m_client(), 38));

		MH_Initialize();

		MH_CreateHook(sendmessage_target, &Hooked::GCSendMessageHook, reinterpret_cast<void**>(&oGCSendMessage));
		MH_CreateHook(retrievemessage_target, &Hooked::GCRetrieveMessageHook, reinterpret_cast<void**>(&oGCRetrieveMessage));
		MH_CreateHook(ClientLeafSystem_target, &Hooked::AddRenderable, reinterpret_cast<void**>(&oAddRenderable));
		MH_CreateHook(dispatch_target, &Hooked::hkDispatchUserMessage, reinterpret_cast<void**>(&otDispatchUserMessage));

		MH_EnableHook(MH_ALL_HOOKS);

		Window = FindWindowA(("Valve001"), NULL);
		Hooked::oldWindowProc = (WNDPROC)SetWindowLongPtr(Window, GWL_WNDPROC, (LONG_PTR)Hooked::WndProc);
		feature::menu->_menu_opened = true;

		ctx.ticks_allowed = 0;
		cfg_manager->setup();

		g_Inventory->ParseSkins();

		c_lua::get().initialize();
		parser::parse();
		_events.emplace_back(sxor("Initialized and ready."));

		ctx.init_finished = true;
		VIRTUALIZER_END;
		return true;
	}

	void QueueJobs()
	{
		return;
	}

	void Destroy()
	{
		VIRTUALIZER_START;

		if ((DWORD)ctx.clantag_cvar > 0x2000) {
			csgo.m_engine_cvars()->UnregisterConCommand(ctx.clantag_cvar);
			csgo.m_mem_alloc()->Free((void*)ctx.clantag_cvar);
			ctx.clantag_cvar = nullptr;
		}

		DetourRemove((byte*)OriginalUpdateClientSideAnimations, (byte*)Hooked::UpdateClientSideAnimation);
		DetourRemove((byte*)OriginalStandardBlendingRules, (byte*)Hooked::StandardBlendingRules);
		DetourRemove((byte*)OriginalDoExtraBonesProcessing, (byte*)Hooked::DoExtraBonesProcessing);
		DetourRemove((byte*)OriginalGetForeignFallbackFontName, (byte*)Hooked::GetForeignFallbackFontName);
		DetourRemove((byte*)OriginalPhysicsSimulate, (byte*)Hooked::PhysicsSimulate);
		DetourRemove((byte*)OriginalSetupBones, (byte*)Hooked::SetupBones);
		DetourRemove((byte*)OriginalBuildTransformations, (byte*)Hooked::BuildTransformations);
		DetourRemove((byte*)OriginalModifyEyePosition, (byte*)Hooked::ModifyEyePosition);
		DetourRemove((byte*)OriginalShouldFlipModel, (byte*)Hooked::ShouldFlipModel);
		DetourRemove((byte*)OriginalCL_Move, (byte*)Hooked::CL_Move);
		DetourRemove((byte*)OriginalSetViewmodelOffsets, (byte*)Hooked::SetViewmodelOffsets);
		DetourRemove((byte*)OriginalCalcViewBob, (byte*)Hooked::CalcViewBob);
		DetourRemove((byte*)OriginalReportHit, (byte*)Hooked::ReportHit);
		DetourRemove((byte*)OriginalAddViewModelBob, (byte*)Hooked::AddViewModelBob);
		DetourRemove((byte*)OriginalCheckForSequenceChange, (byte*)Hooked::CheckForSequenceChange);
		DetourRemove((byte*)OriginalShouldSkipAnimationFrame, (byte*)Hooked::ShouldSkipAnimationFrame);
		DetourRemove((byte*)OriginalGetColorModulation, (byte*)Hooked::GetColorModulation);
		DetourRemove((byte*)OriginalIsUsingStaticPropDebugModes, (byte*)Hooked::IsUsingStaticPropDebugModes);
		DetourRemove((byte*)OriginalProcessInterpolatedList, (byte*)Hooked::ProcessInterpolatedList);
		for (CClientEffectRegistration* head = ctx.m_effects_head(); head; head = head->next)
		{
			if (strstr(head->effectName, sxor("Impact")) && strlen(head->effectName) <= 8) {
				head->function = oImpact;
				break;
			}
		}

		Engine::PropManager::Instance()->Hook(m_flAbsYaw, sxor("DT_CSRagdoll"), sxor("m_flAbsYaw"));
		Engine::PropManager::Instance()->Hook(m_nSequence, sxor("DT_BaseViewModel"), sxor("m_nSequence"));
		Engine::PropManager::Instance()->Hook(m_vecForce, sxor("DT_CSRagdoll"), sxor("m_vecForce"));
		Engine::PropManager::Instance()->Hook(m_ViewModel, sxor("DT_BaseViewModel"), sxor("m_nModelIndex"));
		Engine::PropManager::Instance()->Hook(m_flVelocityModifier, sxor("DT_CSPlayer"), sxor("m_flVelocityModifier"));

		vmt.m_bsp_tree_query.reset();
		vmt.m_movement.reset();
		vmt.m_material_system.reset();
		vmt.m_engine_sound.reset();
		vmt.m_client.reset();
		vmt.m_prediction.reset();
		vmt.m_panel.reset();
		vmt.m_clientmode.reset();
		vmt.m_surface.reset();
		vmt.m_render_view.reset();
		vmt.m_engine.reset();
		vmt.m_clientstate.reset();
		vmt.m_engine_vgui.reset();
		vmt.m_model_render.reset();
		vmt.m_net_channel.reset();
		vmt.m_device.reset();
		vmt.m_cl_clock_correction.reset();
		vmt.m_cl_grenadepreview.reset();
		vmt.cl_smooth.reset();
		vmt.m_engine_trace.reset();
		//g_menu.on_unload();

		for (int i = 0; i < g_InventorySkins.size(); i++)
			g_InventorySkins.erase(i);

		_inv.inventory.itemCount = g_InventorySkins.size();
		csgo.m_engine()->ExecuteClientCmd("econ_clear_inventory_images");
		write.SendClientHello();
		write.SendMatchmakingClient2GCHello();

		MH_DisableHook(MH_ALL_HOOKS);
		csgo.m_input_system()->EnableInput(true);

		SetWindowLongPtr(Window, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(Hooked::oldWindowProc));

		VIRTUALIZER_END;
	}

	void* CreateInterface(const std::string& image_name, const std::string& name, bool force /*= false */)
	{
		VIRTUALIZER_START;

		auto image = GetModuleHandleA(image_name.c_str());

		if (!image)
			return nullptr;

		auto fn = (CreateInterfaceFn)(GetProcAddress(image, sxor("CreateInterface")));

		if (!fn)
			return nullptr;

		if (force) {
			auto factory = fn(name.c_str(), nullptr);
			return factory;
		}

		char format[1024] = { };

		for (auto i = 0u; i < 1000u; i++)
		{
			sprintf_s(format, sxor("%s%03u"), name.c_str(), i);

			auto factory = fn(format, nullptr);

			if (factory) {
				return factory;
			}
		}
		VIRTUALIZER_END;

		return nullptr;
	}

	__declspec(noinline) void* CreateInterface(ULONG64 offset)
	{

		int cpu_info[4] = { 0 };
		__cpuid(cpu_info, 1);
		auto ptr = (void*)((DWORD)offset ^ cpu_info[0]);
		return ptr;
	}
}

namespace feature
{
	Encrypted_t <c_menu > menu = nullptr;
	Encrypted_t < c_misc > misc = nullptr;
	Encrypted_t < c_antiaimbot > anti_aim = nullptr;
	Encrypted_t < c_resolver > resolver = nullptr;
	Encrypted_t < c_visuals > visuals = nullptr;
	Encrypted_t < c_usercmd > usercmd = nullptr;
	Encrypted_t < c_lagcomp > lagcomp = nullptr;
	Encrypted_t < c_chams > chams = nullptr;
	Encrypted_t < c_autowall > autowall = nullptr;
	Encrypted_t < c_aimbot > ragebot = nullptr;
	Encrypted_t < c_dormant_esp > sound_parser = nullptr;
	Encrypted_t < c_music_player > music_player = nullptr;
	Encrypted_t < c_weather_controller > weather = nullptr;
	Encrypted_t < c_legitaimbot > legitbot = nullptr;
	Encrypted_t < c_grenade_prediction > grenade_prediction = nullptr;
	Encrypted_t < c_grenade_tracer > grenade_tracer = nullptr;
	Encrypted_t < c_hit_chams > hitchams = nullptr;
	Encrypted_t <c_net_channel > net_channel = nullptr;
}

C_WeaponCSBaseGun* m_weapon()
{
	auto client = ctx.m_local();

	if (!client || client->IsDead())
	{//|| client->m_hActiveWeapon() <= 0) {
		ctx.latest_weapon_data = nullptr;
		return nullptr;
	}

	return (C_WeaponCSBaseGun*)(csgo.m_entity_list()->GetClientEntityFromHandle(client->m_hActiveWeapon()));
}