#pragma once
#include "sdk.hpp"
#include <deque>
#include <array>
#include <unordered_map>
#include <ragebot/lag_comp.hpp>
#include "source.hpp"

struct c_chams_settings
{
	Color color;
	Color hidden_color;
	Color white_color = Color::White();
	int	  type;
	bool  wireframe;
	int   setting_type;
	IMaterial* setuped_mat;

	bool pulse;
	bool customize;
	float chams_force_pulse_speed;
	float chams_custom_envmapfresnelminmaxexp_value[3];
};

class c_chams
{
public:
	virtual IMaterial* get_material(int material);
	virtual IMaterial* create_material(std::string_view material_name, std::string_view shader_type, std::string_view material_data);
	virtual void player_chams(void* context, void* state, const ModelRenderInfo_t& info, matrix3x4_t* pCustomBoneToWorld);
	virtual void SetIgnorez(const bool enabled, IMaterial* mat);
	virtual void ModulateColor(int type, Color color, IMaterial* material);
	IMaterial* m_glow = nullptr;
	IMaterial* m_glow1 = nullptr;
	IMaterial* m_glow2 = nullptr;
	IMaterial* m_glow3 = nullptr;
	IMaterial* m_glow4 = nullptr;

};