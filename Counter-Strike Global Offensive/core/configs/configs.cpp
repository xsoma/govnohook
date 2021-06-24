#include "configs.h"
#include <shlobj.h>
#include <fileapi.h>
#include <fstream>
#include "inventory/inventorychanger.h"
#include <protobuf/Protobuffs.h>
#include "lua/Clua.h"
std::unordered_map <std::string, float[4]> colors;
C_ConfigManager* cfg_manager = new C_ConfigManager();

void C_ConfigManager::setup()
{
	setup_item(&ctx.m_settings.aimbot_enabled, false, sxor("aimbot_enabled"));

	for (auto i = 0; i < 8; i++)
	{
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_target_hitbox, 6, sxor("aimbot_target_hitbox") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_multi_point, 6, sxor("aimbot_multi_point") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_multi_point_scale, 0, sxor("aimbot_multi_point_scale") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_multi_point_strange, 0, sxor("aimbot_multi_point_strange") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_prefer_safe_point, false, sxor("aimbot_prefer_safe_point") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_force_safe_point_on_limbs, false, sxor("aimbot_force_safe_point_on_limbs") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_automatic_fire, false, sxor("aimbot_automatic_fire") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_automatic_penetration, false, sxor("aimbot_automatic_penetration") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_silent_aim, false, sxor("aimbot_silent_aim") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_minimum_hit_chance, 0, sxor("aimbot_minimum_hit_chance") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_minimum_damage, 0, sxor("aimbot_minimum_damage") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_minimum_autowall_damage, 0, sxor("aimbot_minimum_autowall_damage") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_minimum_override_damage, 0, sxor("aimbot_minimum_override_damage") + std::to_string(i));

		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_automatic_scope, false, sxor("aimbot_automatic_scope") + std::to_string(i));
		//setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_reduce_aim_step, false, sxor("aimbot_reduce_aim_step") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_maximum_fov, 0, sxor("aimbot_maximum_fov") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_log_misses_due_to_spread, false, sxor("aimbot_log_misses_due_to_spread") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_automatic_stope, false, sxor("aimbot_automatic_stope") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_low_fps_mitigations, 5, sxor("aimbot_low_fps_mitigations") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_remove_recoil, false, sxor("aimbot_remove_recoil") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_accuracy_boost, 0, sxor("aimbot_accuracy_boost") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_delay_shot, false, sxor("aimbot_delay_shot") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_quick_stop, false, sxor("aimbot_quick_stop") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_quick_stop_options, 10, sxor("aimbot_quick_stop_options") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_anti_aim_corrections, false, sxor("aimbot_anti_aim_corrections") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_double_tap, false, sxor("aimbot_double_tap") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_double_tap_mode, 0, sxor("aimbot_double_tap_mode") + std::to_string(i));


		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_min_damage_override.key, 0, sxor("aimbot_min_damage_override.key") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_force_safe_point_key.key, 0, sxor("aimbot_force_safe_point_key.key") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_min_damage_override.mode, 0, sxor("aimbot_min_damage_override.mode") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_force_safe_point_key.mode, 0, sxor("aimbot_force_safe_point_key.mode") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_min_damage_override.key, 0, sxor("aimbot_min_damage_override.key") + std::to_string(i));
		setup_item(&ctx.m_settings.aimbot_weapon[i].aimbot_min_damage_override.mode, 0, sxor("aimbot_min_damage_override.mode") + std::to_string(i));

	}
	setup_item(&ctx.m_settings.aimbot_hideshots_exploit_toggle.key, 0, sxor("aimbot_hideshots_exploit_toggle.key"));
	setup_item(&ctx.m_settings.aimbot_hideshots_exploit_toggle.mode, 0, sxor("aimbot_hideshots_exploit_toggle.mode"));
	setup_item(&ctx.m_settings.aimbot_double_tap_key.key, 0, sxor("aimbot_double_tap_key.key"));
	setup_item(&ctx.m_settings.aimbot_double_tap_key.mode, 0, sxor("aimbot_double_tap_key.mode"));

	//setup_item(&ctx.m_settings.aimbot_prefer_body, false, sxor("aimbot_prefer_body"));

	setup_item(&ctx.m_settings.anti_aim_enabled, false, sxor("anti_aim_enabled"));
	setup_item(&ctx.m_settings.anti_aim_pitch, 0, sxor("anti_aim_pitch"));
	setup_item(&ctx.m_settings.anti_aim_yaw_base, 0, sxor("anti_aim_yaw_base"));
	setup_item(&ctx.m_settings.anti_aim_yaw_type, 0, sxor("anti_aim_yaw_type"));
	setup_item(&ctx.m_settings.anti_aim_yaw_custom, 0, sxor("anti_aim_yaw_custom"));
	setup_item(&ctx.m_settings.anti_aim_yaw_jitter_type, 0, sxor("anti_aim_yaw_jitter_type"));
	setup_item(&ctx.m_settings.anti_aim_yaw_jitter_custom, 0, sxor("anti_aim_yaw_jitter_custom"));
	setup_item(&ctx.m_settings.anti_aim_body_yaw, 0, sxor("anti_aim_body_yaw"));
	setup_item(&ctx.m_settings.anti_aim_freestanding_body_yaw, false, sxor("anti_aim_freestanding_body_yaw"));
	setup_item(&ctx.m_settings.anti_aim_lby_yaw_target, 0, sxor("anti_aim_lby_yaw_target"));
	setup_item(&ctx.m_settings.anti_aim_edge_yaw, false, sxor("anti_aim_edge_yaw"));
	setup_item(&ctx.m_settings.anti_aim_freestanding_fake_type, 0, sxor("anti_aim_freestanding_fake_type"));
	setup_item(&ctx.m_settings.anti_aim_fake_limit, 0, sxor("anti_aim_fake_limit"));
	setup_item(&ctx.m_settings.anti_aim_fake_switch_key.key, 0, sxor("anti_aim_fake_switch_key.key"));
	setup_item(&ctx.m_settings.anti_aim_freestand_key.key, 0, sxor("anti_aim_freestand_key.key"));

	setup_item(&ctx.m_settings.anti_aim_yaw_right_switch_key.key, 0, sxor("anti_aim_yaw_right_switch_key.key"));
	setup_item(&ctx.m_settings.anti_aim_yaw_right_switch_key.mode, 0, sxor("anti_aim_yaw_right_switch_key.mode"));
	setup_item(&ctx.m_settings.anti_aim_yaw_left_switch_key.key, 0, sxor("anti_aim_yaw_left_switch_key.key"));
	setup_item(&ctx.m_settings.anti_aim_yaw_left_switch_key.mode, 0, sxor("anti_aim_yaw_left_switch_key.mode"));
	setup_item(&ctx.m_settings.anti_aim_yaw_backward_switch_key.key, 0, sxor("anti_aim_yaw_backward_switch_key.key"));
	setup_item(&ctx.m_settings.anti_aim_yaw_backward_switch_key.mode, 0, sxor("anti_aim_yaw_backward_switch_key.mode"));

	setup_item(&ctx.m_settings.anti_aim_fake_switch_key.mode, 0, sxor("anti_aim_fake_switch_key.mode"));
	setup_item(&ctx.m_settings.anti_aim_freestand_key.mode, 0, sxor("anti_aim_freestand_key.mode"));
	setup_item(&ctx.m_settings.anti_aim_leg_movement, 0, sxor("anti_aim_leg_movement"));

	setup_item(&ctx.m_settings.fake_lag_enabled, false, sxor("fake_lag_enabled"));
	setup_item(&ctx.m_settings.fake_lag_type, 0, sxor("fake_lag_type"));
	setup_item(&ctx.m_settings.fake_lag_variance, 0, sxor("fake_lag_variance"));
	setup_item(&ctx.m_settings.fake_lag_value, 0, sxor("fake_lag_value"));
	setup_item(&ctx.m_settings.fake_lag_shooting, false, sxor("fake_lag_shooting"));

	for (int i = 0; i < 4; i++)
	{
		setup_item(&ctx.m_settings.player_esp_bounding_box_color[i], 1, sxor("player_esp_bounding_box_color") + std::to_string(i));
		setup_item(&ctx.m_settings.player_esp_name_color[i], 1, sxor("player_esp_name_color") + std::to_string(i));
		setup_item(&ctx.m_settings.player_esp_weapon_color[i], 1, sxor("player_esp_weapon_color") + std::to_string(i));
		setup_item(&ctx.m_settings.player_esp_ammo_color[i], 1, sxor("player_esp_ammo_color") + std::to_string(i));
		setup_item(&ctx.m_settings.player_esp_skeleton_color[i], 1, sxor("player_esp_skeleton_color") + std::to_string(i));
		setup_item(&ctx.m_settings.player_esp_out_of_fov_arrow_color[i], 1, sxor("player_esp_out_of_fov_arrow_color") + std::to_string(i));

		setup_item(&ctx.m_settings.colored_models_player_color[i], 1, sxor("colored_models_player_color") + std::to_string(i));
		setup_item(&ctx.m_settings.colored_models_player_behind_wall_color[i], 1, sxor("colored_models_player_behind_wall_color") + std::to_string(i));
		setup_item(&ctx.m_settings.colored_models_teammate_color[i], 1, sxor("colored_models_teammate_color") + std::to_string(i));
		setup_item(&ctx.m_settings.colored_models_teammate_behind_wall_color[i], 1, sxor("colored_models_teammate_behind_wall_color") + std::to_string(i));
		setup_item(&ctx.m_settings.colored_models_local_player_color[i], 1, sxor("colored_models_local_player_color") + std::to_string(i));
		setup_item(&ctx.m_settings.colored_models_local_player_fake_color[i], 1, sxor("colored_models_local_player_fake_color") + std::to_string(i));
		setup_item(&ctx.m_settings.colored_models_hit_capsule_color[i], 1, sxor("colored_models_hit_capsule_color") + std::to_string(i));

		setup_item(&ctx.m_settings.other_esp_dropped_weapons_color[i], 1, sxor("other_esp_dropped_weapons_color") + std::to_string(i));
		setup_item(&ctx.m_settings.other_esp_grenades_color[i], 1, sxor("other_esp_grenades_color") + std::to_string(i));
		setup_item(&ctx.m_settings.other_esp_bomb_color[i], 1, sxor("other_esp_bomb_color") + std::to_string(i));
		setup_item(&ctx.m_settings.other_esp_grenade_trajectory_color[i], 1, sxor("other_esp_grenade_trajectory_color") + std::to_string(i));
		
		
		setup_item(&ctx.m_settings.effects_bullet_tracers_color[i], 1, sxor("effects_bullet_tracers_color") + std::to_string(i));
		setup_item(&ctx.m_settings.effects_override_skybox_color[i], 1, sxor("effects_override_skybox_color") + std::to_string(i));

		setup_item(&ctx.m_settings.other_esp_grenade_warning_beam_color[i], 1, sxor("other_esp_grenade_warning_beam_color") + std::to_string(i));
	}


	setup_item(&ctx.m_settings.player_esp_dormant, false, sxor("player_esp_dormant"));
	setup_item(&ctx.m_settings.player_esp_bounding_box, false, sxor("player_esp_bounding_box"));
	setup_item(&ctx.m_settings.player_esp_health_bar, false, sxor("player_esp_health_bar"));
	setup_item(&ctx.m_settings.player_esp_name, false, sxor("player_esp_name"));
	setup_item(&ctx.m_settings.player_esp_flags, false , sxor("player_esp_flags"));
	setup_item(&ctx.m_settings.player_esp_weapon_icon, false, sxor("player_esp_weapon_icon"));
	setup_item(&ctx.m_settings.player_esp_weapon_text, false, sxor("player_esp_weapon_text"));
	setup_item(&ctx.m_settings.player_esp_ammo, false, sxor("player_esp_ammo"));
	setup_item(&ctx.m_settings.player_esp_distance, false, sxor("player_esp_distance"));
	for (int z = 0; z < 3; z++)
	{
		setup_item(&ctx.m_settings.glow[z].enabled, false, sxor("player_esp_glow_enabled") + std::to_string(z));
		setup_item(&ctx.m_settings.glow[z].m_bFullBloomRender, false, sxor("player_esp_glow_m_bFullBloomRender") + std::to_string(z));
		setup_item(&ctx.m_settings.glow[z].m_bRenderWhenOccluded, false, sxor("player_esp_glow_m_bRenderWhenOccluded") + std::to_string(z));
		setup_item(&ctx.m_settings.glow[z].m_bRenderWhenUnoccluded, false, sxor("player_esp_glow_m_bRenderWhenUnoccluded") + std::to_string(z));

	}
	for (int x = 0; x < 4; x++)
	{
		setup_item(&ctx.m_settings.glow[0].color[x], 1, sxor("player_esp_glow_enemy_color") + std::to_string(x));
		setup_item(&ctx.m_settings.glow[1].color[x], 1, sxor("player_esp_glow_local_color") + std::to_string(x));
	}

	setup_item(&ctx.m_settings.player_esp_hit_marker, false, sxor("player_esp_hit_marker"));
	setup_item(&ctx.m_settings.player_esp_hit_marker_sound, false, sxor("player_esp_hit_marker_sound"));
	setup_item(&ctx.m_settings.player_esp_money, false, sxor("player_esp_money"));
	setup_item(&ctx.m_settings.player_esp_skeleton, false, sxor("player_esp_skeleton"));
	setup_item(&ctx.m_settings.player_esp_out_of_fov_arrow, false, sxor("player_esp_out_of_fov_arrow"));
	setup_item(&ctx.m_settings.player_esp_out_of_fov_arrow_size, 0, sxor("player_esp_out_of_fov_arrow_size"));
	setup_item(&ctx.m_settings.player_esp_out_of_fov_arrow_distance, 0, sxor("player_esp_out_of_fov_arrow_distance"));

	setup_item(&ctx.m_settings.colored_models_player, false, sxor("colored_models_player"));
	setup_item(&ctx.m_settings.colored_models_player_behind_wall, false, sxor("colored_models_player_behind_wall"));
	setup_item(&ctx.m_settings.colored_models_player_type, 0, sxor("colored_models_player_type"));

	setup_item(&ctx.m_settings.colored_models_teammate, false, sxor("colored_models_teammate"));
	setup_item(&ctx.m_settings.colored_models_teammate_behind_wall, false, sxor("colored_models_teammate_behind_wall"));
	setup_item(&ctx.m_settings.colored_models_teammate_type, 0, sxor("colored_models_teammate_type"));

	setup_item(&ctx.m_settings.colored_models_local_player, false, sxor("colored_models_local_player"));
	setup_item(&ctx.m_settings.colored_models_local_player_fake, false, sxor("colored_models_local_player_fake"));
	setup_item(&ctx.m_settings.colored_models_local_player_fake_type, 0, sxor("colored_models_local_player_fake_type"));
	setup_item(&ctx.m_settings.colored_models_local_player_type, 0, sxor("colored_models_local_player_type"));

	setup_item(&ctx.m_settings.colored_models_hit_capsule, false, sxor("colored_models_hit_capsule"));

	setup_item(&ctx.m_settings.other_esp_radar, false, sxor("other_esp_radar"));
	setup_item(&ctx.m_settings.other_esp_dropped_weapons, 4, sxor("other_esp_dropped_weapons"));
	setup_item(&ctx.m_settings.other_esp_dropped_weapons_ammo, false, sxor("other_esp_dropped_weapons_ammo"));
	setup_item(&ctx.m_settings.other_esp_grenades, false, sxor("other_esp_grenades"));
	setup_item(&ctx.m_settings.other_esp_crosshair, false, sxor("other_esp_crosshair"));
	setup_item(&ctx.m_settings.other_esp_bomb, false, sxor("other_esp_bomb"));
	setup_item(&ctx.m_settings.other_esp_grenade_trajectory, false, sxor("other_esp_grenade_trajectory"));
	setup_item(&ctx.m_settings.other_esp_grenade_proximity_warning, false, sxor("other_esp_grenade_proximity_warning"));
	setup_item(&ctx.m_settings.other_esp_grenade_warning_beam_rainbow, false, sxor("other_esp_grenade_warning_beam_rainbow"));
	setup_item(&ctx.m_settings.other_esp_grenade_warning_beam_model_name, "", sxor("other_esp_grenade_warning_beam_model_name:"));

	setup_item(&ctx.m_settings.other_esp_grenade_warning_beam_width, 0, sxor("other_esp_grenade_warning_beam_width"));
	setup_item(&ctx.m_settings.other_esp_grenade_warning_beam_end_width, 0, sxor("other_esp_grenade_warning_beam_end_width"));
	setup_item(&ctx.m_settings.other_esp_grenade_warning_beam_halo_scale, 0, sxor("other_esp_grenade_warning_beam_halo_scale"));
	setup_item(&ctx.m_settings.other_esp_grenade_warning_beam_segments, 0, sxor("other_esp_grenade_warning_beam_segments"));

	setup_item(&ctx.m_settings.other_esp_spectators, false, sxor("other_esp_spectators"));
	setup_item(&ctx.m_settings.other_esp_penetration_reticle, false, sxor("other_esp_penetration_reticle"));


	setup_item(&ctx.m_settings.effects_remove_flashbang_effect, false, sxor("effects_remove_flashbang_effect"));
	setup_item(&ctx.m_settings.effects_remove_smoke_grenades, false, sxor("effects_remove_smoke_grenades"));
	setup_item(&ctx.m_settings.effects_remove_fog, false, sxor("effects_remove_fog"));
	setup_item(&ctx.m_settings.effects_visual_recoil_adjustment, 0, sxor("effects_visual_recoil_adjustment"));
	setup_item(&ctx.m_settings.effects_transparent_walls, 0, sxor("effects_transparent_walls"));
	setup_item(&ctx.m_settings.effects_transparent_props, 0, sxor("effects_transparent_props"));
	setup_item(&ctx.m_settings.effects_brightness_adjustment, 0, sxor("effects_brightness_adjustment"));
	setup_item(&ctx.m_settings.effects_night_adjustment, 0, sxor("effects_night_adjustment"));

	setup_item(&ctx.m_settings.effects_remove_scope_overlay, false, sxor("effects_remove_scope_overlay"));
	setup_item(&ctx.m_settings.effects_instant_scope, false, sxor("effects_instant_scope"));
	setup_item(&ctx.m_settings.effects_disable_post_processing, false, sxor("effects_disable_post_processing"));
	setup_item(&ctx.m_settings.effects_force_third_person_alive, false, sxor("effects_force_third_person_alive"));
	setup_item(&ctx.m_settings.effects_force_third_person_key.key, 0, sxor("effects_force_third_person_key.key"));
	setup_item(&ctx.m_settings.effects_force_third_person_key.mode, 0, sxor("effects_force_third_person_key.mode"));
	setup_item(&ctx.m_settings.effects_force_third_person_distance, 0, sxor("effects_force_third_person_distance"));

	setup_item(&ctx.m_settings.effects_force_third_person_dead, false, sxor("effects_force_third_person_dead"));
	setup_item(&ctx.m_settings.effects_disable_rendering_of_teammates, false, sxor("effects_disable_rendering_of_teammates"));
	setup_item(&ctx.m_settings.effects_bullet_tracers, false, sxor("effects_bullet_tracers"));
	setup_item(&ctx.m_settings.effects_bullet_impact, false, sxor("effects_bullet_impact"));
	setup_item(&ctx.m_settings.effects_override_skybox, false, sxor("effects_override_skybox"));
	setup_item(&ctx.m_settings.effects_aspect_ratio, 0, sxor("effects_aspect_ratio"));
	

	setup_item(&ctx.m_settings.anti_aim_autopeek_key.key, 0, sxor("anti_aim_autopeek_key.key"));
	setup_item(&ctx.m_settings.anti_aim_fakeduck_key.key, 0, sxor("anti_aim_fakeduck_key.key"));
	setup_item(&ctx.m_settings.anti_aim_autopeek_key.mode, 0, sxor("anti_aim_autopeek_key.mode"));
	setup_item(&ctx.m_settings.anti_aim_fakeduck_key.mode, 0, sxor("anti_aim_fakeduck_key.mode"));

	setup_item(&ctx.m_settings.miscellaneous_override_fov, 90, sxor("miscellaneous_override_fov"));
	setup_item(&ctx.m_settings.miscellaneous_override_zoom_fov, 100, sxor("miscellaneous_override_zoom_fov"));
	setup_item(&ctx.m_settings.miscellaneous_knifebot, false, sxor("miscellaneous_knifebot"));

	setup_item(&ctx.m_settings.miscellaneous_knifebot_knifebot_settings, 3, sxor("miscellaneous_knifebot_knifebot_settings"));

	setup_item(&ctx.m_settings.miscellaneous_zeusbot, false, sxor("miscellaneous_zeusbot"));
	setup_item(&ctx.m_settings.miscellaneous_reveal_completitive_ranks, false, sxor("miscellaneous_reveal_completitive_ranks"));
	setup_item(&ctx.m_settings.miscellaneous_reveal_overwatch_players, false, sxor("miscellaneous_reveal_overwatch_players"));
	setup_item(&ctx.m_settings.miscellaneous_clan_tag_spammer, false, sxor("miscellaneous_clan_tag_spammer"));
	setup_item(&ctx.m_settings.miscellaneous_log_weapon_purchases, false, sxor("miscellaneous_log_weapon_purchases"));
	setup_item(&ctx.m_settings.miscellaneous_log_damage_dealt, false, sxor("miscellaneous_log_damage_dealt"));
	setup_item(&ctx.m_settings.miscellaneous_persistent_kill_feed, false, sxor("miscellaneous_persistent_kill_feed"));
	setup_item(&ctx.m_settings.miscellaneous_unlock_hidden_cvars, false, sxor("miscellaneous_unlock_hidden_cvars"));


	setup_item(&ctx.m_settings.movement_infinite_duck, false, sxor("movement_infinite_duck"));
	setup_item(&ctx.m_settings.movement_bunny_hop, false, sxor("movement_bunny_hop"));
	setup_item(&ctx.m_settings.movement_air_strafe, false, sxor("movement_air_strafe"));

	setup_item(&ctx.m_settings.movement_air_strafe_direction, 3, sxor("movement_air_strafe_direction"));
	setup_item(&ctx.m_settings.movement_air_strafe_smoothing, 0, sxor("movement_air_strafe_smoothing"));
	setup_item(&ctx.m_settings.movement_air_duck, 0, sxor("movement_air_duck"));
	setup_item(&ctx.m_settings.movement_jump_at_edge, false, sxor("movement_jump_at_edge"));
	setup_item(&ctx.m_settings.movement_jump_at_edge_key.key, 0, sxor("movement_jump_at_edge_key.key"));
	setup_item(&ctx.m_settings.movement_jump_at_edge_key.mode, 0, sxor("movement_jump_at_edge_key.mode"));

	for (int i = 0; i < 4; i++)
	{
		setup_item(&ctx.m_settings.menu_color[i], 1, sxor("menu_color") + std::to_string(i));
	}

	setup_item(&ctx.m_settings.settings_anti_untrusted, false, sxor("settings_anti_untrusted"));
	setup_item(&ctx.m_settings.settings_hide_from_obs, false, sxor("settings_hide_from_obs"));
	setup_item(&ctx.m_settings.settings_low_fps_warning, false, sxor("settings_low_fps_warning"));
	setup_item(&ctx.m_settings.settings_lock_menu_layout, false, sxor("settings_lock_menu_layout"));

	setup_item(&ctx.m_settings.scripts, sxor("scripts:loaded"));

	ctx.m_settings.other_esp_grenade_warning_beam_rainbow = true;
	ctx.m_settings.other_esp_grenade_warning_beam_model_name = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	ctx.m_settings.other_esp_grenade_warning_beam_model_name = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	ctx.m_settings.other_esp_grenade_warning_beam_halo_scale = 0;//0
	ctx.m_settings.other_esp_grenade_warning_beam_width = 11;//11
	ctx.m_settings.other_esp_grenade_warning_beam_end_width = 11;//11
	ctx.m_settings.other_esp_grenade_warning_beam_segments = 40;//40

	ctx.m_settings.menu_color[0] = 132.f / 255.f;
	ctx.m_settings.menu_color[1] = 100.f / 255.f;
	ctx.m_settings.menu_color[2] = 255.f / 255.f;

}

void C_ConfigManager::add_item(void* pointer, const char* name, const std::string& type) {
	items.push_back(new C_ConfigItem(std::string(name), pointer, type));
}

void C_ConfigManager::setup_item(int* pointer, int value, const std::string& name)
{
	add_item(pointer, name.c_str(), sxor("int"));
	*pointer = value;
}
void C_ConfigManager::setup_item(float* pointer, float value, const std::string& name)
{
	add_item(pointer, name.c_str(), sxor("float"));
	*pointer = value;
}

void C_ConfigManager::setup_item(bool* pointer, bool value, const std::string& name)
{
	add_item(pointer, name.c_str(), sxor("bool"));
	*pointer = value;
}

void C_ConfigManager::setup_item(Color* pointer, Color value, const std::string& name)
{
	colors[name][0] = (float)value.r() / 255.0f;
	colors[name][1] = (float)value.g() / 255.0f;
	colors[name][2] = (float)value.b() / 255.0f;
	colors[name][3] = (float)value.a() / 255.0f;

	add_item(pointer, name.c_str(), sxor("Color"));
	*pointer = value;
}

void C_ConfigManager::setup_item(std::vector< int >* pointer, int size, const std::string& name)
{
	add_item(pointer, name.c_str(), sxor("vector<int>"));
	pointer->clear();

	for (int i = 0; i < size; i++)
		pointer->push_back(FALSE);
}
void C_ConfigManager::setup_item(std::vector< std::string >* pointer, const std::string& name)
{
	add_item(pointer, name.c_str(), sxor("vector<string>"));
}

void C_ConfigManager::setup_item(std::string* pointer, const std::string& value, const std::string& name)
{
	add_item(pointer, name.c_str(), sxor("string"));
	*pointer = value; //-V820
}

void C_ConfigManager::save(std::string config)
{
	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() -> void
	{
		static TCHAR path[MAX_PATH];


		folder = sxor("darkraihook\\");
		file = sxor("darkraihook\\") + config;

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	json allJson;

	for (auto it : items)
	{
		json j;

		j[sxor("name")] = it->name;
		j[sxor("type")] = it->type;

		if (!it->type.compare(sxor("int")))
			j[sxor("value")] = (int)*(int*)it->pointer; //-V206
		else if (!it->type.compare(sxor("float")))
			j[sxor("value")] = (float)*(float*)it->pointer;
		else if (!it->type.compare(sxor("bool")))
			j[sxor("value")] = (bool)*(bool*)it->pointer;
		else if (!it->type.compare(sxor("Color")))
		{
			auto c = *(Color*)(it->pointer);

			std::vector<int> a = { c.r(), c.g(), c.b(), c.a() };
			json ja;

			for (auto& i : a)
				ja.push_back(i);

			j[sxor("value")] = ja.dump();
		}
		else if (!it->type.compare(sxor("vector<int>")))
		{
			auto& ptr = *(std::vector<int>*)(it->pointer);
			json ja;

			for (auto& i : ptr)
				ja.push_back(i);

			j[sxor("value")] = ja.dump();
		}
		else if (!it->type.compare(sxor("vector<string>")))
		{
			auto& ptr = *(std::vector<std::string>*)(it->pointer);
			json ja;

			for (auto& i : ptr)
				ja.push_back(i);

			j[sxor("value")] = ja.dump();
		}
		else if (!it->type.compare(sxor("string")))
			j[sxor("value")] = (std::string) * (std::string*)it->pointer;

		allJson.push_back(j);
	}

	auto get_type = [](menu_item_type type)
	{
		switch (type) //-V719
		{
		case CHECK_BOX:
			return "bool";
		case COMBO_BOX:
		case SLIDER_INT:
			return "int";
		case SLIDER_FLOAT:
			return "float";
		case COLOR_PICKER:
			return "Color";
		case KEY_BIND:
			return "KeyBind";
		}
	};

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto& script = c_lua::get().scripts.at(i);

		for (auto& item : c_lua::get().items.at(i))
		{
			if (item.second.type == NEXT_LINE)
				continue;

			json j;
			auto type = (std::string)get_type(item.second.type);

			j[sxor("name")] = item.first;
			j[sxor("type")] = type;

			if (!type.compare(sxor("bool")))
				j[sxor("value")] = item.second.check_box_value;
			else if (!type.compare(sxor("int")))
				j[sxor("value")] = item.second.type == COMBO_BOX ? item.second.combo_box_value : item.second.slider_int_value;
			else if (!type.compare(sxor("float")))
				j[sxor("value")] = item.second.slider_float_value;
			else if (!type.compare(sxor("Color")))
			{
				std::vector <float> color =
				{
					item.second.color_picker_value[0],
					item.second.color_picker_value[1],
					item.second.color_picker_value[2],
					item.second.color_picker_value[3]
				};

				json j_color;

				for (auto& i : color)
					j_color.push_back(i);

				j[sxor("value")] = j_color.dump();
			}

			allJson.push_back(j);
		}
	}

	std::string data;

	Base64 base64;
	base64.encode(allJson.dump(), &data);

	std::ofstream ofs;
	ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	ofs << std::setw(4) << data << std::endl;
	ofs.close();


	static TCHAR path[MAX_PATH];
	std::string zfolder;
	//SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path);
	zfolder = ("darkraihook\\");
	auto zfile = zfolder + "inventory.cfg";

	WritePrivateProfileStringA("darkaihook", "count", std::to_string(_inv.inventory.itemCount).c_str(), zfile.c_str());

	int i = 0;
	for (auto weapon : g_InventorySkins)
	{
		if (!weapon.second.wId)
			continue;

		std::string itemid = "inventory" + std::to_string(i) + "_id";
		std::string paintkitstr = "inventory" + std::to_string(i) + "_paintkit";
		std::string wearkitstr = "inventory" + std::to_string(i) + "_wear";
		std::string seedkitstr = "inventory" + std::to_string(i) + "_seed";
		std::string inusetstr = "inventory" + std::to_string(i) + "_inuset";
		std::string inusectstr = "inventory" + std::to_string(i) + "_inusect";
		std::string qual = "inventory" + std::to_string(i) + "_qual";
		std::string name = "inventory" + std::to_string(i) + "_name";
		std::string stattrak = "inventory" + std::to_string(i) + "_stattrak";


		std::string st1 = "inventory" + std::to_string(i) + "_st1";
		std::string st2 = "inventory" + std::to_string(i) + "_st2";
		std::string st3 = "inventory" + std::to_string(i) + "_st3";
		std::string st4 = "inventory" + std::to_string(i) + "_st4";

		std::string isequct = "inventory" + std::to_string(i) + "_isequct";

		std::string isequt = "inventory" + std::to_string(i) + "_isequt";




		WritePrivateProfileStringA("darkaihook", itemid.c_str(), std::to_string(weapon.second.wId).c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", paintkitstr.c_str(), std::to_string(weapon.second.paintKit).c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", wearkitstr.c_str(), std::to_string(weapon.second.wear).c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", seedkitstr.c_str(), std::to_string(weapon.second.seed).c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", inusetstr.c_str(), weapon.second.in_use_t ? "true" : "false", zfile.c_str());
		WritePrivateProfileStringA("darkaihook", inusectstr.c_str(), weapon.second.in_use_ct ? "true" : "false", zfile.c_str());
		WritePrivateProfileStringA("darkaihook", qual.c_str(), std::to_string(weapon.second.quality).c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", name.c_str(), weapon.second.name.c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", stattrak.c_str(), std::to_string(weapon.second.stattrak).c_str(), zfile.c_str());

		WritePrivateProfileStringA("darkaihook", st1.c_str(), std::to_string(weapon.second.sicker[0]).c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", st2.c_str(), std::to_string(weapon.second.sicker[1]).c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", st3.c_str(), std::to_string(weapon.second.sicker[2]).c_str(), zfile.c_str());
		WritePrivateProfileStringA("darkaihook", st4.c_str(), std::to_string(weapon.second.sicker[3]).c_str(), zfile.c_str());

		i++;
	}

	//ctx.updated_skin = true;
}

void C_ConfigManager::load(std::string config, bool load_script_items)
{
	static auto find_item = [](std::vector< C_ConfigItem* > items, std::string name) -> C_ConfigItem*
	{
		for (int i = 0; i < (int)items.size(); i++) //-V202
			if (!items[i]->name.compare(name))
				return items[i];

		return nullptr;
	};

	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() ->void
	{
		static TCHAR path[MAX_PATH];
		folder = sxor("darkraihook\\");
		file = sxor("darkraihook\\") + config;
		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	std::string data;

	std::ifstream ifs;
	ifs.open(file + '\0');

	ifs >> data;
	ifs.close();

	if (data.empty())
		return;

	Base64 base64;

	std::string decoded_data;
	base64.decode(data, &decoded_data);

	std::ofstream ofs;
	ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	ofs << decoded_data;
	ofs.close();

	json allJson;

	std::ifstream ifs_final;
	ifs_final.open(file + '\0');

	ifs_final >> allJson;
	ifs_final.close();

	base64.encode(allJson.dump(), &data);

	std::ofstream final_ofs;
	final_ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	final_ofs << data;
	final_ofs.close();

	for (auto it = allJson.begin(); it != allJson.end(); ++it)
	{
		json j = *it;

		std::string name = j[sxor("name")];
		std::string type = j[sxor("type")];

		auto script_item = std::count_if(name.begin(), name.end(),
			[](char& c)
			{
				return c == '.';
			}
		) >= 2;

		if (load_script_items && script_item)
		{
			std::string script_name;
			auto first_point = false;

			for (auto& c : name)
			{
				if (c == '.')
				{
					if (first_point)
						break;
					else
						first_point = true;
				}

				script_name.push_back(c);
			}

			auto script_id = c_lua::get().get_script_id(script_name);

			if (script_id == -1)
				continue;

			for (auto& current_item : c_lua::get().items.at(script_id))
			{
				if (current_item.first == name)
				{
					if (!type.compare(sxor("bool")))
					{
						current_item.second.type = CHECK_BOX;
						current_item.second.check_box_value = j[sxor("value")].get<bool>();
					}
					else if (!type.compare(sxor("int")))
					{
						if (current_item.second.type == COMBO_BOX)
							current_item.second.combo_box_value = j[sxor("value")].get<int>();
						else
							current_item.second.slider_int_value = j[sxor("value")].get<int>();
					}
					else if (!type.compare(sxor("float")))
						current_item.second.slider_float_value = j[sxor("value")].get<float>();
					else if (!type.compare(sxor("Color")))
					{
						std::vector<int> a;
						json ja = json::parse(j[sxor("value")].get<std::string>().c_str());

						for (json::iterator it = ja.begin(); it != ja.end(); ++it)
							a.push_back(*it);

						colors.erase(name);
						current_item.second.color_picker_value[0] = a[0];
						current_item.second.color_picker_value[1] = a[1];
						current_item.second.color_picker_value[2] = a[2];
						current_item.second.color_picker_value[3] = a[3];
					}
				}
			}
		}
		else if (!load_script_items && !script_item)
		{
			auto item = find_item(items, name);

			if (item)
			{
				if (!type.compare(sxor("int")))
					*(int*)item->pointer = j[sxor("value")].get<int>(); //-V206
				else if (!type.compare(sxor("float")))
					*(float*)item->pointer = j[sxor("value")].get<float>();
				else if (!type.compare(sxor("bool")))
					*(bool*)item->pointer = j[sxor("value")].get<bool>();

				else if (!type.compare(sxor("Color")))
				{
					std::vector<int> a;
					json ja = json::parse(j[sxor("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						a.push_back(*it);

					colors.erase(item->name);
					*(Color*)item->pointer = Color(a[0], a[1], a[2], a[3]);
				}
				else if (!type.compare(sxor("vector<int>")))
				{
					auto ptr = static_cast<std::vector <int>*> (item->pointer);
					ptr->clear();

					json ja = json::parse(j[sxor("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						ptr->push_back(*it);
				}
				else if (!type.compare(sxor("vector<string>")))
				{
					auto ptr = static_cast<std::vector <std::string>*> (item->pointer);
					ptr->clear();

					json ja = json::parse(j[sxor("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						ptr->push_back(*it);
				}
				else if (!type.compare(sxor("string")))
					*(std::string*)item->pointer = j[sxor("value")].get<std::string>();
			}
		}
	}


	static TCHAR path[MAX_PATH];
	std::string zfolder;
	//SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path);
	zfolder = ("darkraihook\\");
	auto zfile = zfolder + "inventory.cfg";

	g_InventorySkins.clear();
	char value_l[32] = { '\0' };
	GetPrivateProfileStringA("darkaihook", "count", "", value_l, 32, zfile.c_str());
	_inv.inventory.itemCount = atoi(value_l);

	for (int i = 0; i < _inv.inventory.itemCount; i++)
	{
		std::string itemid = "inventory" + std::to_string(i) + "_id";
		std::string paintkitstr = "inventory" + std::to_string(i) + "_paintkit";
		std::string wearkitstr = "inventory" + std::to_string(i) + "_wear";
		std::string seedkitstr = "inventory" + std::to_string(i) + "_seed";
		std::string inusetstr = "inventory" + std::to_string(i) + "_inuset";
		std::string inusectstr = "inventory" + std::to_string(i) + "_inusect";
		std::string qual = "inventory" + std::to_string(i) + "_qual";
		std::string name = "inventory" + std::to_string(i) + "_name";
		std::string stattrak = "inventory" + std::to_string(i) + "_stattrak";



		std::string st1 = "inventory" + std::to_string(i) + "_st1";
		std::string st2 = "inventory" + std::to_string(i) + "_st2";
		std::string st3 = "inventory" + std::to_string(i) + "_st3";
		std::string st4 = "inventory" + std::to_string(i) + "_st4";

		std::string isequct = "inventory" + std::to_string(i) + "_isequct";
		std::string isequt = "inventory" + std::to_string(i) + "_isequt";

		wskin skinInfo;

		GetPrivateProfileStringA("darkaihook", itemid.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.wId = atoi(value_l);
		GetPrivateProfileStringA("darkaihook", paintkitstr.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.paintKit = atoi(value_l);
		GetPrivateProfileStringA("darkaihook", wearkitstr.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.wear = atof(value_l);
		GetPrivateProfileStringA("darkaihook", seedkitstr.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.seed = atoi(value_l);
		GetPrivateProfileStringA("darkaihook", inusetstr.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.in_use_t = !strcmp(value_l, "true");
		GetPrivateProfileStringA("darkaihook", inusectstr.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.in_use_ct = !strcmp(value_l, "true");
		GetPrivateProfileStringA("darkaihook", qual.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.quality = atoi(value_l);
		GetPrivateProfileStringA("darkaihook", name.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.name = value_l;
		GetPrivateProfileStringA("darkaihook", stattrak.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.stattrak = atoi(value_l);


		GetPrivateProfileStringA("darkaihook", st1.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.sicker[0] = atoi(value_l);
		GetPrivateProfileStringA("darkaihook", st2.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.sicker[1] = atoi(value_l);
		GetPrivateProfileStringA("darkaihook", st3.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.sicker[2] = atoi(value_l);
		GetPrivateProfileStringA("darkaihook", st4.c_str(), "", value_l, 32, zfile.c_str());
		skinInfo.sicker[3] = atoi(value_l);

		g_InventorySkins.insert({ RandomInt(20000, 200000), skinInfo });
	}

	csgo.m_engine()->ExecuteClientCmd("econ_clear_inventory_images");
	write.SendClientHello();
	write.SendMatchmakingClient2GCHello();

	//ctx.updated_skin = true;
}

void C_ConfigManager::remove(std::string config)
{
	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() -> void
	{
		static TCHAR path[MAX_PATH];


		folder = sxor("darkraihook\\");
		file = sxor("darkraihook\\") + config;


		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();

	std::string path = file + '\0';
	std::remove(path.c_str());
}

void C_ConfigManager::config_files()
{
	std::string folder;

	auto get_dir = [&folder]() -> void
	{
		static TCHAR path[MAX_PATH];

		folder = sxor("darkraihook\\");

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	files.clear();

	std::string path = folder + sxor("/*.gay");
	WIN32_FIND_DATA fd;

	HANDLE hFind = FindFirstFile(path.c_str(), &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				files.push_back(fd.cFileName);
		} while (FindNextFile(hFind, &fd));

		FindClose(hFind);
	}
}