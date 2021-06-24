#include <visuals/chams.hpp>
#include "source.hpp"
#include <props/entity.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <hooks/hooked.hpp>
#include <sdk/math/math.hpp>
#include <props/displacement.hpp>
#include <ragebot/lag_comp.hpp>
#include <ragebot/resolver.hpp>
#include "usercmd.hpp"
#include <antiaim/anti_aimbot.hpp>
#include <unordered_map>
#include <algorithm>
#include <menu/menu/menu.hpp>
#include <visuals/visuals.hpp>

#include <thread>
#include <cctype>
#include <fstream>

IMaterial* c_chams::create_material(std::string_view material_name, std::string_view shader_type, std::string_view material_data) {
	const auto key_values = reinterpret_cast<KeyValues*>(csgo.m_mem_alloc()->Alloc(36u));

	key_values->InitKeyValues(shader_type.data());
	key_values->LoadFromBuffer(material_name.data(), material_data.data());

	return csgo.m_material_system()->CreateMaterial(material_name.data(), key_values);
}
IMaterial* c_chams::get_material(int material) {

	if (!csgo.m_engine()->IsInGame() || ctx.m_local() == nullptr)
		return 0;

	static bool init_materials = false;

	if (!init_materials) {
		std::ofstream(sxor("csgo\\materials\\material_flat.vmt")) << R"("UnlitGeneric"
		{
			"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
		})";
		std::ofstream(sxor("csgo\\materials\\material_textured.vmt")) << R"("VertexLitGeneric"
		{
			"$basetexture"	"vgui/white_additive"
			"$model"		"1"
			"$flat"			"0"
			"$nocull"		"1"
			"$halflambert"	"1"
			"$nofog"		"1"
			"$ignorez"		"0"
			"$znearer"		"0"
			"$wireframe"	"0"
		})";
		m_glow = create_material(sxor("csgo\\materials\\glow.vmt"), sxor("VertexLitGeneric"), sxor(R"#("VertexLitGeneric" {
	"$additive" "1"
	"$envmap" "models/effects/cube_white"
	"$envmaptint" "[1 1 1]"
	"$envmapfresnel" "1"
	"$envmapfresnelminmaxexp" "[0 1 2]"
	"$alpha" "1.0"
})#"));

		m_glow1 = create_material(sxor("csgo\\materials\\glow_cus.vmt"), sxor("VertexLitGeneric"), sxor(R"#("VertexLitGeneric" {
	"$additive" "1"
	"$envmap" "models/effects/cube_white"
	"$envmaptint" "[1 1 1]"
	"$envmapfresnel" "1"
	"$envmapfresnelminmaxexp" "[2 0 2]"
	"$alpha" "0.8"
})#"));

		m_glow2 = create_material(sxor("csgo\\materials\\glow_base.vmt"), sxor("VertexLitGeneric"), sxor(R"#("VertexLitGeneric" {
	"$additive" "1"
	"$envmap" "models/effects/cube_white"
	"$envmaptint" "[1 1 1]"
	"$envmapfresnel" "1"
	"$envmapfresnelminmaxexp" "[2 1 0]"
	"$alpha" "0.8"
})#"));
		init_materials = true;
	}

	switch (material) 
	{
		default:
		case 0: /* textured */
			return csgo.m_material_system()->FindMaterial(sxor("material_textured"));
		case 1: /* flat */
			return csgo.m_material_system()->FindMaterial(sxor("material_flat"));
		case 2: /* glowing */
			return m_glow;
		case 3: /* glowing */
			return m_glow1;
		case 4: /* glowing */
			return m_glow2;
	}
}

void c_chams::SetIgnorez(const bool enabled, IMaterial* mat)
{
	if (!csgo.m_engine()->IsInGame() || ctx.m_local() == nullptr || mat == nullptr)
		return;

	if (mat && !mat->IsErrorMaterial()) {
		mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, enabled);
		//mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, enabled);
		mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
	}
}
void c_chams::ModulateColor(int type, Color color, IMaterial* material)
{
	if (!csgo.m_engine()->IsInGame() || ctx.m_local() == nullptr || material == nullptr)
		return;

	if (material && !material->IsErrorMaterial())
	{
		material->ColorModulate(color[0] / 255.f, color[1] / 255.f, color[2] / 255.f);

		if (type >= 2) {
			auto envmaptint = material->FindVar(sxor("$envmaptint"), nullptr);

			if (envmaptint)
				envmaptint->SetVecValue(Vector((color.r() / 255.f), (color.g() / 255.f), (color.b() / 255.f)));

		}
	}

	float c[] = { color[0] / 255.f, color[1] / 255.f, color[2] / 255.f };

	csgo.m_render_view()->SetColorModulation(c);
}
void c_chams::player_chams(void* context, void* state, const ModelRenderInfo_t& info, matrix3x4_t* pCustomBoneToWorld)
{
	typedef int(__thiscall* OriginalFn)(void*, void* ctx, void* state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

	if (!csgo.m_engine()->IsInGame())
		return;

	const char* m_cModelName = csgo.m_model_info()->GetModelName(info.pModel);
	C_BasePlayer* player = reinterpret_cast<C_BasePlayer*>(csgo.m_entity_list()->GetClientEntity(info.entity_index));

	if (!player)
		return;

	if (player && !player->IsDormant() && player->IsPlayer())
	{
		if (player->m_iHealth() > 0)
		{
			float c[] = { 0.f,0.f,0.f };

			csgo.m_render_view()->GetColorModulation(c);
			csgo.m_studio_render()->ForcedMaterialOverride(nullptr);

			if (player->m_iTeamNum() != ctx.m_local()->m_iTeamNum() && ctx.m_settings.colored_models_player)
			{
				auto mat = get_material(ctx.m_settings.colored_models_player_type);

				if (!mat || mat->IsErrorMaterial() || mat == nullptr)
					return;

				float old_blend = csgo.m_render_view()->GetBlend();
				mat->IncrementReferenceCount();
				mat->SetMaterialVarFlag(MATERIAL_VAR_UNUSED, true);

				if (ctx.m_settings.colored_models_player_behind_wall)
				{
					Color enemy_hidden_chams_color = ctx.flt2color(ctx.m_settings.colored_models_player_behind_wall_color);

					csgo.m_render_view()->SetBlend(enemy_hidden_chams_color.a() / 255.f);

					SetIgnorez(true, mat);
					ModulateColor(ctx.m_settings.colored_models_player_type, enemy_hidden_chams_color, mat);
					mat->AlphaModulate(enemy_hidden_chams_color.a() / 255.f);

					csgo.m_studio_render()->ForcedMaterialOverride(mat);
					vmt.m_model_render->VCall<OriginalFn>(21)(csgo.m_model_render(), context, state, info, pCustomBoneToWorld);
				}

				Color enemy_chams_color = ctx.flt2color(ctx.m_settings.colored_models_player_color);

				csgo.m_render_view()->SetBlend(enemy_chams_color.a() / 255.f);

				SetIgnorez(false, mat);
				ModulateColor(ctx.m_settings.colored_models_player_type, enemy_chams_color, mat);
				mat->AlphaModulate(enemy_chams_color.a() / 255.f);

				csgo.m_studio_render()->ForcedMaterialOverride(mat);
				vmt.m_model_render->VCall<OriginalFn>(21)(csgo.m_model_render(), context, state, info, pCustomBoneToWorld);

				csgo.m_render_view()->SetBlend(old_blend);
			}
			else if (player->m_iTeamNum() == ctx.m_local()->m_iTeamNum() && player != ctx.m_local() && ctx.m_settings.colored_models_teammate)
			{
				auto mat = get_material(ctx.m_settings.colored_models_teammate_type);

				if (!mat || mat->IsErrorMaterial() || mat == nullptr)
					return;

				float old_blend = csgo.m_render_view()->GetBlend();
				mat->IncrementReferenceCount();
				mat->SetMaterialVarFlag(MATERIAL_VAR_UNUSED, true);

				if (ctx.m_settings.colored_models_teammate_behind_wall)
				{
					Color teammate_hidden_chams_color = ctx.flt2color(ctx.m_settings.colored_models_teammate_behind_wall_color);

					csgo.m_render_view()->SetBlend(teammate_hidden_chams_color.a() / 255.f);

					SetIgnorez(true, mat);
					ModulateColor(ctx.m_settings.colored_models_player_type, teammate_hidden_chams_color, mat);
					mat->AlphaModulate(teammate_hidden_chams_color.a() / 255.f);

					csgo.m_studio_render()->ForcedMaterialOverride(mat);
					vmt.m_model_render->VCall<OriginalFn>(21)(csgo.m_model_render(), context, state, info, pCustomBoneToWorld);
				}

				Color teammate_chams_color = ctx.flt2color(ctx.m_settings.colored_models_teammate_color);

				csgo.m_render_view()->SetBlend(teammate_chams_color.a() / 255.f);

				SetIgnorez(false, mat);
				ModulateColor(ctx.m_settings.colored_models_teammate_type, teammate_chams_color, mat);
				mat->AlphaModulate(teammate_chams_color.a() / 255.f);

				csgo.m_studio_render()->ForcedMaterialOverride(mat);
				vmt.m_model_render->VCall<OriginalFn>(21)(csgo.m_model_render(), context, state, info, pCustomBoneToWorld);

				csgo.m_render_view()->SetBlend(old_blend);
			}
			else if (player == ctx.m_local() && (ctx.m_settings.colored_models_local_player || ctx.m_settings.colored_models_local_player_fake))
			{
				auto mat = get_material(ctx.m_settings.colored_models_local_player_type);
				auto fake_mat = get_material(ctx.m_settings.colored_models_local_player_fake_type);

				if (!mat || mat->IsErrorMaterial() || mat == nullptr)
					return;

				if (!fake_mat || fake_mat->IsErrorMaterial() || fake_mat == nullptr)
					return;

				float old_blend = csgo.m_render_view()->GetBlend();

				mat->IncrementReferenceCount();
				fake_mat->IncrementReferenceCount();

				if (ctx.m_settings.colored_models_local_player_fake)
				{
					Color local_chams_color = ctx.flt2color(ctx.m_settings.colored_models_local_player_fake_color);

					csgo.m_render_view()->SetBlend(local_chams_color.a() / 255.f);

					SetIgnorez(false, fake_mat);
					ModulateColor(ctx.m_settings.colored_models_local_player_fake_type, local_chams_color, fake_mat);
					fake_mat->AlphaModulate(local_chams_color.a() / 255.f);

					csgo.m_studio_render()->ForcedMaterialOverride(fake_mat);
					vmt.m_model_render->VCall<OriginalFn>(21)(csgo.m_model_render(), context, state, info, ctx.fake_matrix);
				}
				if (ctx.m_settings.colored_models_local_player)
				{
					Color local_chams_color = ctx.flt2color(ctx.m_settings.colored_models_local_player_color);

					csgo.m_render_view()->SetBlend(local_chams_color.a() / 255.f);

					SetIgnorez(false, mat);
					ModulateColor(ctx.m_settings.colored_models_local_player_type, local_chams_color, mat);
					mat->AlphaModulate(local_chams_color.a() / 255.f);

					csgo.m_studio_render()->ForcedMaterialOverride(mat);
					vmt.m_model_render->VCall<OriginalFn>(21)(csgo.m_model_render(), context, state, info, pCustomBoneToWorld);
				}
				else
				{
					csgo.m_studio_render()->ForcedMaterialOverride(nullptr);
					vmt.m_model_render->VCall<OriginalFn>(21)(csgo.m_model_render(), context, state, info, pCustomBoneToWorld);
				}
				csgo.m_render_view()->SetBlend(old_blend);
			}
			csgo.m_render_view()->SetColorModulation(c);
		}
	}
}