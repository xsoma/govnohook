#include <hooks/hooked.hpp>
#include <menu/menu/menu.hpp>
#include <visuals/visuals.hpp>
#include <visuals/chams.hpp>
#include "menu/menu/menu_v2.h"
#include <visuals/sound_parser.hpp>
#include <visuals/grenades.hpp>

#include <hooks/hooked.hpp>
#include <props/displacement.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <ragebot/prediction.hpp>
#include <misc/movement.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <intrin.h>
#include <render/rendering.hpp>

#include "source.hpp"
#include <intrin.h>
#include <menu/menu/i_menu.hpp>
#include "lua/Clua.h"
#include <Resources/Shaders/PostProcessing.h>

using FnPT = void(__thiscall*)(void*, unsigned int, bool, bool);
int ScreenSize2W, ScreenSize2H;

int knife;// = ctx.m_settings.skinchanger_knife;
int skin;// = ctx.m_settings.skinchanger_knife_skin;
bool enabled;// = ctx.m_settings.skinchanger_enabled;
bool chams_weapons;// = ctx.m_settings.chams_misc[1];

namespace Hooked
{

	long __stdcall EndScene(IDirect3DDevice9* device)
	{
		using Fn = long(__stdcall*)(IDirect3DDevice9* device);
		auto original = vmt.m_device->VCall<Fn>(Index::IDirectX::EndScene);

		if (!original)
			return 0;

		static void* allowed_ret_addr = nullptr; if (!allowed_ret_addr) allowed_ret_addr = _ReturnAddress();
		if (allowed_ret_addr != _ReturnAddress())
			return original(device);

		IDirect3DStateBlock9* pixel_state = NULL; IDirect3DVertexDeclaration9* vertDec; IDirect3DVertexShader9* vertShader;
		device->CreateStateBlock(D3DSBT_PIXELSTATE, &pixel_state);
		device->GetVertexDeclaration(&vertDec);
		device->GetVertexShader(&vertShader);
		pixel_state->Capture();

		static bool m_bInitialized = false;
		if (!m_bInitialized)
		{
			d::create(device);
			m_bInitialized = true;
		}
		else
		{
			PostProcessing::setDevice(device);
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			
			PostProcessing::newFrame();

			//renderer::run(device);
			//feature::visuals->render(false, true);

			menu::draw(device);
			ImGui::EndFrame();

			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		}
		pixel_state->Apply();
		pixel_state->Release();
		device->SetVertexDeclaration(vertDec);
		device->SetVertexShader(vertShader);

		return original(device);
	}

	long __stdcall Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
	{
		using Fn = long(__stdcall*)(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp);
		auto original = vmt.m_device->VCall<Fn>(Index::IDirectX::Reset);

		if (!original)
			return 0;

		renderer::lost(device);
		PostProcessing::onDeviceReset();
		auto result = original(device, pp);
		renderer::create(device);

		return result;
	}

	void __fastcall PaintTraverse(void* ecx, void* edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
	{
		static unsigned int scope_panel;
		if (!scope_panel)
		{
			const char* panelname = csgo.m_panel()->GetName(vguiPanel);

			if (!strcmp(panelname, sxor("HudZoom")))
				scope_panel = vguiPanel;
		}

		//if ((std::string(csgo.m_panel()->GetName(vguiPanel)).find("HudZoom") == std::string::npos || !ctx.m_settings.visuals_no_scope) && vmt.m_panel)
		//	vmt.m_panel->VCall<FnPT>(Index::IPanel::PaintTraverse)(ecx, vguiPanel, forceRepaint, allowForce);

		if (vguiPanel == scope_panel && ctx.m_settings.effects_remove_scope_overlay)
			return;

		vmt.m_panel->VCall<FnPT>(Index::IPanel::PaintTraverse)(ecx, vguiPanel, forceRepaint, allowForce);

		static unsigned int drawPanel = 0;
		if (!drawPanel)
		{
			const char* panelname = csgo.m_panel()->GetName(vguiPanel);

			if (panelname[0] == 'M' && panelname[2] == 't')
				drawPanel = vguiPanel;
		}

		if (vguiPanel != drawPanel)
			return;

		if (csgo.m_engine()->IsInGame() && ctx.m_local() && !ctx.m_local()->IsDead() && ctx.hud_death_notice) {
			static auto had_notices = false;

			if (ctx.m_settings.miscellaneous_persistent_kill_feed) {
				float* localDeathNotice = &ctx.hud_death_notice->localplayer_lifetime_mod;
				if (localDeathNotice && *localDeathNotice < 1000.f)* localDeathNotice = FLT_MAX;
				had_notices = true;
			}
			else if (had_notices)
			{
				const auto death_notices = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(ctx.hud_death_notice) - 20);

				if (death_notices) {
					const auto ClearDeathNotices = reinterpret_cast<void(__thiscall*)(DWORD)>(Memory::Scan(
						sxor("client.dll"), sxor("55 8B EC 83 EC 0C 53 56 8B 71 58")));

					if (ClearDeathNotices)
						ClearDeathNotices(reinterpret_cast<DWORD>(ctx.hud_death_notice) - 20);

					had_notices = false;
				}
			}
		}

		static bool* s_bOverridePostProcessingDisable = *reinterpret_cast<bool**>(Memory::Scan(sxor("client.dll"), sxor("80 3D ? ? ? ? ? 53 56 57 0F 85")) + 0x2);
		*s_bOverridePostProcessingDisable = (csgo.m_engine()->IsInGame() && ctx.m_local() && ctx.m_settings.effects_disable_post_processing);
	}

	void __stdcall EngineVGUI_Paint(int mode)
	{
		typedef void(__thiscall * Paint_t)(IEngineVGui*, int);
		if (vmt.m_engine_vgui)
			vmt.m_engine_vgui->VCall<Paint_t>(14)(csgo.m_engine_vgui(), mode);

		typedef void(__thiscall * start_drawing)(void*);
		typedef void(__thiscall * finish_drawing)(void*);

		static start_drawing start_draw = reinterpret_cast<start_drawing>(Memory::Scan(sxor("vguimatsurface.dll"), sxor("55 8B EC 83 E4 C0 83 EC 38")));
		static finish_drawing end_draw = reinterpret_cast<finish_drawing>(Memory::Scan(sxor("vguimatsurface.dll"), sxor("8B 0D ? ? ? ? 56 C6 05")));

		if (mode & 1)
		{
			start_draw(csgo.m_surface());
			g_menuinput.pre();

			static bool bResChange = false;
			static auto prev_time = csgo.m_globals()->framecount;

			if ((csgo.m_globals()->framecount - prev_time) > 5 || ctx.screen_size.Length() == 0.f) {
				csgo.m_engine()->GetScreenSize(ScreenSize2W, ScreenSize2H);
				prev_time = csgo.m_globals()->framecount;
			}

			if (!bResChange && (ScreenSize2W != ctx.screen_size.x
				|| ScreenSize2H != ctx.screen_size.y))
			{
				ctx.screen_size.x = ScreenSize2W;
				ctx.screen_size.y = ScreenSize2H;
				bResChange = true;
			}

			if (bResChange)
				Drawing::CreateFonts();

			feature::sound_parser->start();
			feature::visuals->render(bResChange);
			feature::grenade_tracer->paint();
			
			for (auto current : c_lua::get().hooks.getHooks(("paint")))
				current.func();

			feature::visuals->damage_esp();

			if (bResChange) // yes it was important to some of my visuals (manual anti-aim arrows for example) so i guess this code is fine. (NO)
				bResChange = false;

			end_draw(csgo.m_surface());
		}
	}

	void __fastcall SceneEnd(void* ecx, void* edx)
	{
		if (vmt.m_render_view)
			vmt.m_render_view->VCall<void(__thiscall*)(void*)>(9)(ecx);
		else
			return;

	}

	int __fastcall ListLeavesInBox(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax) {
		typedef int(__thiscall * ListLeavesInBox)(void*, const Vector&, const Vector&, unsigned short*, int);
		static auto ofunc = vmt.m_bsp_tree_query->VCall< ListLeavesInBox >(6);

		// occulusion getting updated on player movement/angle change,
		// in RecomputeRenderableLeaves ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674 );
		// check for return in CClientLeafSystem::InsertIntoTree

		static auto list_leaves = (void*)(Memory::Scan("client.dll", "56 52 FF 50 18") + 5);
		if (!csgo.m_engine()->IsInGame() || _ReturnAddress() != list_leaves) // 89 44 24 14 ( 0x14244489 ) - new / 8B 7D 08 8B ( 0x8B087D8B ) - old
			return ofunc(bsp, mins, maxs, pList, listMax);

		// get current renderable info from stack ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470 )
		auto info = *(RenderableInfo_t**)((uintptr_t)_AddressOfReturnAddress() + 0x14);
		if (!info || !info->m_pRenderable || !info->m_pRenderable->GetIClientUnknown())
			return ofunc(bsp, mins, maxs, pList, listMax);

		// check if disabling occulusion for players ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1491 )
		auto base_entity = info->m_pRenderable->GetIClientUnknown()->GetBaseEntity();
		if (!base_entity || !base_entity->IsPlayer())
			return ofunc(bsp, mins, maxs, pList, listMax);

		// fix render order, force translucent group ( https://www.unknowncheats.me/forum/2429206-post15.html )
		// AddRenderablesToRenderLists: https://i.imgur.com/hcg0NB5.png ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473 )
		info->m_Flags &= ~0x100;
		info->m_Flags2 |= 0x40;

		// extend world space bounds to maximum ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707 )
		static const Vector map_min = Vector(-16384.0f, -16384.0f, -16384.0f);
		static const Vector map_max = Vector(16384.0f, 16384.0f, 16384.0f);
		const auto count = ofunc(bsp, map_min, map_max, pList, listMax);
		return count;
	}

	void __stdcall DrawModelExecute(void* context, void* state, const ModelRenderInfo_t& info, matrix3x4_t* pCustomBoneToWorld)
	{
		using Fn = void(__thiscall*)(void*, void* ctx, void* state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
		static auto ofc = vmt.m_model_render->VCall<Fn>(21);

		if (csgo.m_studio_render()->IsForcedMaterialOverriden())
		{
			return ofc(csgo.m_model_render(), context, state, info, pCustomBoneToWorld);
		}

		if (strstr(info.pModel->szName, sxor("shadow")) != nullptr)
			return;

		// disable rendering of sleeves.
		if (strstr(info.pModel->szName, sxor("sleeve")) != nullptr)
			return;

		// disables csgo's hdr effect.
		static auto hdr = csgo.m_material_system()->FindMaterial(sxor("dev/blurfiltery_nohdr"), sxor("Other textures"));
		hdr->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

		feature::chams->player_chams(context, state, info, pCustomBoneToWorld);

		ofc(csgo.m_model_render(), context, state, info, pCustomBoneToWorld); 
		csgo.m_studio_render()->ForcedMaterialOverride(nullptr);
	}
}