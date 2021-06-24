#include <core/hooks/hooked.hpp>
#include <sdk/props/displacement.hpp>
#include <sdk/props/player.hpp>
#include <sdk/props/weapon.hpp>
#include <features/ragebot/prediction.hpp>
#include "movement.hpp"
#include <features/ragebot/rage_aimbot.hpp>
#include <features/antiaim/anti_aimbot.hpp>
#include <playsoundapi.h>
#include <features/visuals/visuals.hpp>
#include <intrin.h>
#include <features/ragebot/resolver.hpp>
#include <features/ragebot/lag_comp.hpp>
#include <menu/menu/menu.hpp>
#include <features/ragebot/autowall.hpp>
#include <features/visuals/visuals.hpp>
#include "sdk.hpp"
#include "source.hpp"
#include "music_player.hpp"
#include <features/legitbot/aimbot.hpp>
#include <visuals/sound_parser.hpp>
game_events::PlayerHurtListener player_hurt_listener;
game_events::BulletImpactListener bullet_impact_listener;
game_events::PlayerDeathListener player_death_listener;
game_events::RoundEndListener round_end_listener;
game_events::RoundStartListener round_start_listener;
game_events::BombPlantListener bomb_plant_listener;
game_events::PurchaseListener item_purchase_listener;
game_events::WeaponFireListener weapon_fire_listener;

game_events::BombStartPlantListener bomb_plant_start_listener;
game_events::BombStopPlantListener bomb_plant_end_listener;
game_events::BombStartDefuseListener bomb_defuse_start_listener;
game_events::BombStopDefuseListener bomb_defuse_end_listener;

void game_events::init()
{
	csgo.m_event_manager()->AddListener(&item_purchase_listener, sxor("item_purchase"), false);
	csgo.m_event_manager()->AddListener(&player_hurt_listener, sxor("player_hurt"), false);
	csgo.m_event_manager()->AddListener(&bullet_impact_listener, sxor("bullet_impact"), false);
	csgo.m_event_manager()->AddListener(&weapon_fire_listener, sxor("weapon_fire"), false);
	csgo.m_event_manager()->AddListener(&player_death_listener, sxor("player_death"), false);
	csgo.m_event_manager()->AddListener(&round_end_listener, sxor("round_end"), false);
	csgo.m_event_manager()->AddListener(&round_start_listener, sxor("round_start"), false);
	csgo.m_event_manager()->AddListener(&bomb_plant_listener, sxor("bomb_planted"), false);

	csgo.m_event_manager()->AddListener(&bomb_plant_start_listener, sxor("bomb_beginplant"), false);
	csgo.m_event_manager()->AddListener(&bomb_plant_end_listener, sxor("bomb_abortplant"), false);
	csgo.m_event_manager()->AddListener(&bomb_defuse_start_listener, sxor("bomb_begindefuse"), false);
	csgo.m_event_manager()->AddListener(&bomb_defuse_end_listener, sxor("bomb_abortdefuse"), false);
}

void game_events::BombStartPlantListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	const auto userid = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));

	player_info info;

	ctx.bomb_planting = true;
	ctx.plant_time = csgo.m_globals()->curtime;
	ctx.bomb_site = game_event->GetInt(sxor("site"));
	//_events.emplace_back(std::string("bomb site planted: ") + std::to_string(game_event->GetInt(sxor("site"))));
}
void game_events::BombStopPlantListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	const auto userid = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));

	player_info info;

	ctx.bomb_planting = false;
	ctx.plant_time = 0;
	ctx.bomb_site = 0;
}

void game_events::WeaponFireListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	feature::resolver->listener(game_event);

	const auto userid = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));
	auto entity = csgo.m_entity_list()->GetClientEntity(userid);

	if (!entity || entity != ctx.m_local())
		return;

	if (!entity->IsDead() && ctx.m_settings.colored_models_hit_capsule)
	{
		if (!feature::ragebot->m_target)
			return;

		c_player_records* log = &feature::lagcomp->records[feature::ragebot->m_target->entindex() - 1];

		if (!log || !log->best_record || !log->best_record->resolved_matrix)
			return;

		feature::hitchams->AddHitmatrix(feature::ragebot->m_target, log->best_record->resolved_matrix);
	}
}

void game_events::BombStartDefuseListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	const auto userid = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));

	const auto haskit = game_event->GetBool(sxor("haskit"));

	player_info info;


	/*if (ctx.m_settings.misc_notification.at(0) && csgo.m_engine()->GetPlayerInfo(userid, &info))
		_events.emplace_back(std::string(sxor("bomb is being defused ") + std::string(haskit ? sxor(" (using kit) ") : "") + sxor(" by ") + std::string{ info.name }.substr(0, 24)));*/
}

void game_events::BombStopDefuseListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	const auto userid = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));

	player_info info;


	/*if (ctx.m_settings.misc_notification.at && csgo.m_engine()->GetPlayerInfo(userid, &info))
		_events.emplace_back(std::string(std::string{ info.name }.substr(0, 24) + sxor(" stopped defusing the bomb")));*/
}

void game_events::BombPlantListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	//const auto &entity = csgo.m_entity_list()->GetClientEntity(csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid"))));

	//const int bombsite = game_event->GetInt(sxor("site"));
	const auto userid = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));

	player_info info;

	ctx.bomb_planting = false;
	ctx.plant_time = 0;
	ctx.bomb_site = game_event->GetInt(sxor("site"));
}

std::string hitgroup_to_name(const int hitgroup) {
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return sxor("head");
	case HITGROUP_CHEST:
		return sxor("chest");
	case HITGROUP_STOMACH:
		return sxor("stomach");
	case HITGROUP_LEFTARM:
		return sxor("left arm");
	case HITGROUP_RIGHTARM:
		return sxor("right arm");
	case HITGROUP_LEFTLEG:
		return sxor("left leg");
	case HITGROUP_RIGHTLEG:
		return sxor("right leg");
	default:
		return sxor("body");
	}
}

int hitgroup_to_idx(const int hitgroup) {
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return 0;
	case HITGROUP_CHEST:
		return HITBOX_CHEST;
	case HITGROUP_STOMACH:
		return HITBOX_BODY;
	case HITGROUP_LEFTARM:
		return HITBOX_LEFT_HAND;
	case HITGROUP_RIGHTARM:
		return HITBOX_RIGHT_HAND;
	case HITGROUP_LEFTLEG:
		return HITBOX_LEFT_FOOT;
	case HITGROUP_RIGHTLEG:
		return HITBOX_RIGHT_FOOT;
	default:
		return HITBOX_BODY;
	}
}

void game_events::PlayerHurtListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr/* || !m_weapon() || !ctx.latest_weapon_data*/)
		return;

	const auto& entity = csgo.m_entity_list()->GetClientEntity(csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid"))));

	if (!entity || !entity->GetClientClass() || !entity->IsPlayer())
		return;

	auto entity_attacker = csgo.m_entity_list()->GetClientEntity(csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("attacker"))));

	if (!entity_attacker || !entity_attacker->GetClientClass() || !entity_attacker->IsPlayer())
		return;

	if (entity->m_iTeamNum() == ctx.m_local()->m_iTeamNum() || entity_attacker != ctx.m_local()) {
		if (entity == ctx.m_local())
		{
			player_info attacker_info;
			if (!csgo.m_engine()->GetPlayerInfo(entity_attacker->entindex(), &attacker_info))
				return;

			//hurt indicator + log
			ctx.local_damage[entity->entindex() - 1] = { game_event->GetInt(sxor("dmg_health")), csgo.m_globals()->realtime };

			/*if (ctx.m_settings.misc_notification.at(1)) {
				const auto name = std::string(attacker_info.name);
				_events.emplace_back(std::string(sxor("Hurt by ") + name.substr(0, 24) + sxor(" in the ") + hitgroup_to_name(game_event->GetInt(sxor("hitgroup"))) + sxor(" for ") + std::to_string(game_event->GetInt(sxor("dmg_health"))) + sxor(" (") + std::to_string(game_event->GetInt(sxor("health"))) + sxor(" health remaining)")));
			}*/
		}

		return;
	}

	player_info player_info;
	if (!csgo.m_engine()->GetPlayerInfo(entity->entindex(), &player_info))
		return;

	feature::resolver->hurt_listener(game_event);

	//damage_indicators.emplace_back(Vector::Zero, csgo.m_globals()->tickcount, game_event->GetInt(sxor("dmg_health")), true);

	if (ctx.m_settings.miscellaneous_log_damage_dealt) {
		const auto name = std::string(player_info.name);
		_events.emplace_back(std::string(sxor("Hit ") + name.substr(0, 24) + sxor(" in the ") + hitgroup_to_name(game_event->GetInt(sxor("hitgroup"))) + sxor(" for ") + std::to_string(game_event->GetInt(sxor("dmg_health"))) + sxor(" (") + std::to_string(game_event->GetInt(sxor("health"))) + sxor(" health remaining)")));
	}
	if (ctx.m_settings.player_esp_hit_marker_sound)
	{
		csgo.m_surface()->PlaySound_(sxor("buttons\\arena_switch_press_02.wav"));
	}

	ctx.hurt_time = csgo.m_globals()->realtime + 0.9f;
}

float last_bullet_impact_back = 0;

void game_events::BulletImpactListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr || !ctx.latest_weapon_data || !m_weapon())
		return;

	if (!csgo.m_engine()->IsConnected())
		return;

	const auto& entity = csgo.m_entity_list()->GetClientEntity(csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid"))));

	if (!entity)
		return;

	if (entity->IsDormant())
		return;

	Vector position(game_event->GetInt(sxor("x")), game_event->GetInt(sxor("y")), game_event->GetInt(sxor("z")));

	const auto islocal = entity == ctx.m_local();

	if (!islocal)
		return;

	//if (ctx.m_settings.effects_bullet_tracers)
	//	bullet_tracers.emplace_back(ctx.m_eye_position, position, csgo.m_globals()->curtime, ctx.flt2color(ctx.m_settings.effects_bullet_tracers_color), true);

	if (ctx.m_settings.effects_bullet_impact)
		csgo.m_debug_overlay()->AddBoxOverlay(position, Vector(-2.f, -2.f, -2.f), Vector(2.f, 2.f, 2.f), Vector(0.f, 0.f, 0.f), 0, 0, 255, 127, 4.f);

	feature::resolver->listener(game_event);
}

void game_events::PlayerDeathListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	const auto& entity = csgo.m_entity_list()->GetClientEntity(csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid"))));

	if (!entity || !entity->GetClientClass() || entity->GetClientClass()->m_ClassID != class_ids::CCSPlayer)
		return;

	feature::legitbot->kill_delay(game_event);

	if (entity == ctx.m_local())
	{
		ctx.auto_peek_spot.clear();
		ctx.m_settings.anti_aim_autopeek_key.toggled = false;
		ctx.m_settings.anti_aim_slowwalk_key.toggled = false;
		ctx.shots_fired.fill(0);
		if (ctx.m_settings.anti_aim_slowwalk_key.key > 0)
			ctx.pressed_keys[ctx.m_settings.anti_aim_slowwalk_key.key] = false;
		ctx.m_local()->m_bIsScoped() = false;
	}

	const auto& attacker = csgo.m_entity_list()->GetClientEntity(csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("attacker"))));
}

void game_events::RoundEndListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	ctx.m_corrections_data.clear();
	feature::lagcomp->reset();

	for (auto i = 0; i++; i < feature::sound_parser->m_cSoundPlayers.size()) {

		feature::sound_parser->m_cSoundPlayers[i].reset();
	}

	ctx.bomb_planting = false;
	ctx.plant_time = 0;
	ctx.bomb_site = 0;
	//damage_indicators.clear();

	world_hitmarker.clear();
}

void game_events::RoundStartListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	//if (ctx.m_settings.misc_autobuy_enabled)
	ctx.buy_weapons = true;

	ctx.shots_fired.fill(0);
	ctx.shots_total.fill(0);

	//if (!bullet_tracers.empty())
	//	bullet_tracers.clear();

	//ctx.fired_shot.clear();
	csgo.m_debug_overlay()->ClearAllOverlays();

	//memset(feature::visuals->dormant_alpha, 0, sizeof(int) * 128);
	//feature::visuals->dormant_alpha.fill(0.f);

	ctx.original_tickbase = 0;
	ctx.exploit_tickbase_shift = 0;
	ctx.m_corrections_data.clear();
	ctx.ticks_allowed = 0;

	ctx.auto_peek_spot.clear();
	ctx.m_settings.anti_aim_autopeek_key.toggled = false;
	ctx.m_settings.anti_aim_slowwalk_key.toggled = false;

	const auto shit = csgo.m_engine_cvars()->FindVar(sxor("mp_freezetime"));

	if (shit)
		feature::anti_aim->enable_delay = csgo.m_globals()->realtime + shit->GetFloat();

	ctx.bomb_planting = false;
	ctx.plant_time = 0;
	//damage_indicators.clear();
	world_hitmarker.clear();
}

void game_events::PurchaseListener::FireGameEvent(IGameEvent* game_event)
{
	if (!game_event || ctx.m_local() == nullptr)
		return;

	//const auto &entity = csgo.m_entity_list()->GetClientEntity(csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid"))));

	const auto weapon = game_event->GetString(sxor("weapon"));

	const int idx = csgo.m_engine()->GetPlayerForUserID(game_event->GetInt(sxor("userid")));
	const int team = game_event->GetInt(sxor("team"));

	player_info info;


	if (ctx.m_settings.miscellaneous_log_weapon_purchases && csgo.m_engine()->GetPlayerInfo(idx, &info) && (!ctx.m_local() || team != ctx.m_local()->m_iTeamNum()))
		_events.emplace_back(std::string(std::string{ info.name }.substr(0, 24) + sxor(" bought ") + weapon));
}