#pragma once
#include "sdk.hpp"
#include <deque>
#include <array>
#include <unordered_map>
#include <ragebot/lag_comp.hpp>
#include "source.hpp"

struct C_HitMatrixEntry {
    int ent_index;
    ModelRenderInfo_t info;
    DrawModelState_t state;
    matrix3x4_t pBoneToWorld[128] = {};
    float time;
    matrix3x4_t model_to_world;
};
class c_hit_chams
{
public:
    std::vector<C_HitMatrixEntry> m_Hitmatrix;
    void AddHitmatrix(C_BasePlayer* player, matrix3x4_t* bones);
    void OnPostScreenEffects();
};
