#include "chams_hit.h"
#include <visuals/chams.hpp>

template <typename FuncType>
__forceinline static FuncType call_virtual(void* ppClass, int index)
{
    int* pVTable = *(int**)ppClass; //-V206
    int dwAddress = pVTable[index]; //-V108
    return (FuncType)(dwAddress);
}

void c_hit_chams::AddHitmatrix(C_BasePlayer* player, matrix3x4_t* bones) {
    auto& hit = m_Hitmatrix.emplace_back();

    if (!player->valid(true, true))
        return;

    std::memcpy(hit.pBoneToWorld, bones, player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
    hit.time = csgo.m_globals()->realtime + 3.f;

    static int m_nSkin = player->FindInDataMap(player->GetPredDescMap(), ("m_nSkin"));
    static int m_nBody = player->FindInDataMap(player->GetPredDescMap(), ("m_nBody"));

    hit.info.origin = player->get_abs_origin();
    hit.info.angles = player->get_abs_angles();

    auto renderable = player->GetClientRenderable();

    if (!renderable)
        return;

    auto model = player->GetModel();

    if (!model)
        return;

    auto hdr = *(studiohdr_t**)(player->m_pStudioHdr());

    if (!hdr)
        return;

    hit.state.m_pStudioHdr = hdr;
    hit.state.m_pStudioHWData = csgo.m_mdl_cache()->GetHardwareData(model->studio);
    hit.state.m_pRenderable = renderable;
    hit.state.m_drawFlags = 0;

    hit.info.pRenderable = renderable;
    hit.info.pModel = model;
    hit.info.pLightingOffset = nullptr;
    hit.info.pLightingOrigin = nullptr;
    hit.info.hitboxset = player->m_nHitboxSet();
    hit.info.skin = (int)(uintptr_t(player) + m_nSkin);
    hit.info.body = (int)(uintptr_t(player) + m_nBody);
    hit.info.entity_index = player->entindex();
    hit.info.instance = call_virtual<ModelInstanceHandle_t(__thiscall*)(void*) >(renderable, 30)(renderable);
    hit.info.flags = 0x1;

    hit.info.pModelToWorld = &hit.model_to_world;
    hit.state.m_pModelToWorld = &hit.model_to_world;

    hit.model_to_world.AngleMatrixs(Vector(hit.info.angles.x, hit.info.angles.y, hit.info.angles.z), hit.info.origin);
}

void c_hit_chams::OnPostScreenEffects()
{
    if (m_Hitmatrix.empty() || !ctx.m_settings.colored_models_hit_capsule)
        return;

    typedef int(__thiscall* OriginalFn)(void*, void* ctx, DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);

    auto ctxs = csgo.m_material_system()->GetRenderContext();

    if (!ctxs)
        return;

    bool should_draw = false;
    auto it = m_Hitmatrix.begin();
    while (it != m_Hitmatrix.end()) {

        if (!it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
            !it->info.pRenderable || !it->info.pModelToWorld || !it->info.pModel) {
            ++it;
            should_draw = false;
            continue;
        }
        auto ent = csgo.m_entity_list()->GetClientEntity(it->info.entity_index);
        if (!ent) {
            it = m_Hitmatrix.erase(it);
            should_draw = false;
            continue;
        }

        auto alpha = 1.0f;
        auto delta = csgo.m_globals()->realtime - it->time;
        if (delta > 0.0f) {
            alpha -= delta;
            if (delta > 1.0f) {
                it = m_Hitmatrix.erase(it);
                should_draw = false;
                continue;
            }
        }

        auto alpha_c = ctx.m_settings.colored_models_hit_capsule_color[3];

        auto material = feature::chams->m_glow;

        if (!material || material->IsErrorMaterial())
            return;

        float normal_color[3] =
        {
            ctx.m_settings.colored_models_hit_capsule_color[0] / 255.0f,
            ctx.m_settings.colored_models_hit_capsule_color[1] / 255.0f,
            ctx.m_settings.colored_models_hit_capsule_color[2] / 255.0f
        };

        Color dcolor = ctx.flt2color(ctx.m_settings.colored_models_hit_capsule_color);

        csgo.m_render_view()->SetBlend(alpha_c * alpha);
        material->ColorModulate(normal_color[0] / 255.f, normal_color[1] / 255.f, normal_color[2] / 255.f);

        auto envmaptint = material->FindVar(sxor("$envmaptint"), nullptr);

        if (envmaptint)
            envmaptint->SetVecValue(Vector((dcolor[0] / 255.f), (dcolor[1] / 255.f), (dcolor[2] / 255.f)));

        material->IncrementReferenceCount();
        material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

        csgo.m_studio_render()->ForcedMaterialOverride(material);
        //m_modelrender()->DrawModelExecute(ctx, it->state, it->info, it->pBoneToWorld);

        vmt.m_model_render->VCall<OriginalFn>(21)(csgo.m_model_render(), ctxs, it->state, it->info, it->pBoneToWorld);

        csgo.m_studio_render()->ForcedMaterialOverride(nullptr);
        ++it;
    }
}