#include <ragebot/rage_aimbot.hpp>
#include "source.hpp"
#include <props/entity.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <hooks/hooked.hpp>
#include <sdk/math/math.hpp>
#include <props/displacement.hpp>
#include <ragebot/lag_comp.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <ragebot/resolver.hpp>
#include <visuals/visuals.hpp>
#include <menu/menu/menu.hpp>
#include <misc/movement.hpp>
#include <ragebot/prediction.hpp>
#include <ragebot/autowall.hpp>
#include <misc/misc.hpp>
#include "usercmd.hpp"
//#include "thread/threading.h"
//#include "thread/shared_mutex.h"

//#include <thread>


static constexpr auto total_seeds = 255;
static constexpr auto autowall_traces = 48;

void c_aimbot::get_hitbox_data(C_Hitbox* rtn, C_BasePlayer* ent, int ihitbox, matrix3x4_t* matrix)
{
	if (ihitbox < 0 || ihitbox > 19) return;

	if (!ent) return;

	const model_t* const model = ent->GetModel();

	if (!model)
		return;

	studiohdr_t* const pStudioHdr = csgo.m_model_info()->GetStudioModel(model);

	if (!pStudioHdr)
		return;

	mstudiobbox_t* const hitbox = pStudioHdr->pHitbox(ihitbox, ent->m_nHitboxSet());

	if (!hitbox)
		return;

	const auto is_capsule = hitbox->radius != -1.f;

	Vector min, max;
	if (is_capsule) {
		Math::VectorTransform(hitbox->bbmin, matrix[hitbox->bone], min);
		Math::VectorTransform(hitbox->bbmax, matrix[hitbox->bone], max);
	}
	else
	{
		min = Math::VectorRotate(hitbox->bbmin, hitbox->rotation);
		max = Math::VectorRotate(hitbox->bbmax, hitbox->rotation);
		Math::VectorTransform(min, matrix[hitbox->bone], min);
		Math::VectorTransform(max, matrix[hitbox->bone], max);
	}

	rtn->hitboxID = ihitbox;
	rtn->isOBB = !is_capsule;
	rtn->radius = hitbox->radius;
	rtn->mins = min;
	rtn->maxs = max;
	rtn->hitgroup = hitbox->group;
	rtn->hitbox = hitbox;
	Math::VectorITransform(ctx.m_eye_position, matrix[hitbox->bone], rtn->start_scaled);
	rtn->bone = hitbox->bone;
}

Vector c_aimbot::get_hitbox(C_BasePlayer* ent, int ihitbox, matrix3x4_t mat[])
{
	if (ihitbox < 0 || ihitbox > 19) return Vector::Zero;

	if (!ent) return Vector::Zero;

	if (!ent->GetClientRenderable())
		return Vector::Zero;

	const model_t* const model = ent->GetModel();

	if (!model)
		return Vector::Zero;

	studiohdr_t* const pStudioHdr = csgo.m_model_info()->GetStudioModel(model);

	if (!pStudioHdr)
		return Vector::Zero;

	mstudiobbox_t* const hitbox = pStudioHdr->pHitbox(ihitbox, ent->m_nHitboxSet());

	if (!hitbox)
		return Vector::Zero;

	if (hitbox->bone > 128 || hitbox->bone < 0)
		return Vector::Zero;

	Vector min, max;
	Vector top_point;
	constexpr float rotation = 0.70710678f;
	Math::VectorTransform(hitbox->bbmin, mat[hitbox->bone], min);
	Math::VectorTransform(hitbox->bbmin, mat[hitbox->bone], max);
	//Math::VectorTransform((hitbox->bbmax + hitbox->bbmin) * 0.5f, mat[hitbox->bone], top_point);

	auto center = (min + max) / 2.f;

	return center;
}

int low_count = 0;
int medium_count = 0;

void c_aimbot::build_seed_table()
{
	//static DWORD get_shotgun_spread = Memory::Scan("client.dll", "55 8B EC 83 EC 10 56 8B 75 08 8D");
	//static auto weapon_accuracy_shotgun_spread_patterns = csgo.m_engine_cvars()->FindVar("weapon_accuracy_shotgun_spread_patterns");

	if (seeds_filled >= total_seeds)
		return;

	seeds_filled = 0;

	for (auto i = 0; i < total_seeds; i++) {

		//if (precomputed_seeds.size() >= total_seeds)
		//	break;
		if (seeds_filled >= 128)
			break;

		RandomSeed(seeds[i]);

		float a = RandomFloat(0.0f, 6.2831855f);
		float c = RandomFloat(0.0f, 1.0f);
		float b = RandomFloat(0.0f, 6.2831855f);

		/*if (c < 0.3f)
			++low_count;
		else if (c < 0.55f && c > 0.3f)
			++medium_count;

		if (low_count > int(total_seeds / 3) && c < 0.3f) {
			if (i % 10 != 0)
				c += RandomFloat(0.15f, Math::clamp(1.f - c, 0.f, 1.f));
			else
				continue;
		}

		if (medium_count > int(total_seeds / 3) && c < 0.55f && c > 0.3f) {
			if (i % 10 != 0)
				c += RandomFloat(0.15f, Math::clamp(1.f - c, 0.f, 1.f));
			else
				continue;
		}*/

		/*int id = m_weapon()->m_iItemDefinitionIndex();
		auto recoil_index = m_weapon()->m_flRecoilIndex();

		if (id == 28 && recoil_index < 3.0f)
		{
			for (int i = 3; i > recoil_index; i--)
				c *= c;

			c = 1.0f - c;
		}

		if (m_weapon()->IsShotgun() && weapon_accuracy_shotgun_spread_patterns->GetInt())
		{
			auto _get_shotgun_spread = reinterpret_cast<void(__stdcall*)(int, int, unsigned int, float*, float*)>(get_shotgun_spread);

			if (_get_shotgun_spread)
				_get_shotgun_spread(m_weapon()->m_iItemDefinitionIndex(), 0, i + m_weapon()->GetCSWeaponData()->max_speed * recoil_index, &c, &a);
		}*/

		precomputed_seeds[seeds_filled++] = std::tuple<float, float, float, float, float>(c,
			sin(a), cos(b), sin(b), cos(a));
	}

	//r8_seeds_filled = 0;

	//for (auto i = 0; i < total_seeds; i++) {

	//	//if (precomputed_seeds.size() >= total_seeds)
	//	//	break;
	//	if (r8_seeds_filled >= 128)
	//		break;

	//	RandomSeed(seeds[i]);

	//	float a = RandomFloat(0.0f, 6.2831855f);
	//	float c = RandomFloat(0.0f, 1.0f);
	//	float b = RandomFloat(0.0f, 6.2831855f);

	//	precomputed_r8_seeds[r8_seeds_filled++] = std::tuple<float, float, float, float, float>(c,
	//		sin(a), cos(b), sin(b), cos(a));
	//}
}

int mini_low_count = 0;

void c_aimbot::build_mini_hc_table()
{
	for (auto i = 0; i < 64; i++)
	{
		RandomSeed(i);///*seeds[*/(i * 255) - (min(1,i) / 2)/*]*/);

		float a = RandomFloat(0.0f, 6.2831855f);
		float c = RandomFloat(0.0f, 1.0f);
		float b = RandomFloat(0.0f, 6.2831855f);

		/*if (c <= 0.5f) {
			if (mini_low_count >= (total_seeds / 6))
				continue;
			else
				mini_low_count++;
		}*/

		precomputed_mini_seeds[i] = std::tuple<float, float, float, float, float>(c,
			sin(a), cos(b), sin(b), cos(a));
	}
}

bool c_aimbot::mini_hit_chance(Vector vhitbox, C_BasePlayer* ent, int hitbox, int& hc)
{
	//if (precomputed_mini_seeds.size() > 100)// {
	//	precomputed_mini_seeds.resize(99);
	//	return false;
	//}

	build_mini_hc_table();

	if (!ctx.latest_weapon_data)
		return false;

	C_Hitbox ht;
	get_hitbox_data(&ht, ent, hitbox, ent->m_CachedBoneData().Base());

	auto traces_hit = 0;
	//auto awalls_hit = 0;

	auto const ang = Math::CalcAngle(ctx.m_eye_position, vhitbox);

	//Vector right, up;
	//Math::AngleVectors(, forward, right, up);

	Vector forward, right, up;
	Math::AngleVectors(ang, &forward, &right, &up); // maybe add an option to not account for punch.

	m_weapon()->UpdateAccuracyPenalty();
	const float weap_inaccuracy = m_weapon()->GetInaccuracy();

	if (int(weap_inaccuracy * 1000.f) == 0) {
		hc = 100;
		return true;
	}

	const auto weap_spread = m_weapon()->GetSpread();

	// performance optimization.
	if ((ctx.m_eye_position - ent->m_vecOrigin()).Length() >= (ctx.latest_weapon_data->range * 1.01f))
		return false;

	// setup calculation parameters.
	//const auto round_acc = [](const float accuracy) { return roundf(accuracy * 1000.f) / 1000.f; };
	//const auto sniper = weapon->m_iItemDefinitionIndex() == WEAPON_AWP || weapon->m_iItemDefinitionIndex() == WEAPON_G3SG1
	//	|| weapon->m_iItemDefinitionIndex() == WEAPON_SCAR20 || weapon->m_iItemDefinitionIndex() == WEAPON_SSG08;
	//const auto crouched = ctx.m_local()->m_fFlags() & FL_DUCKING;

	//if (weapon->m_iItemDefinitionIndex() == 64)
	//	return weap_inaccuracy < (crouched ? .0020f : .0055f);

	// no need for hitchance, if we can't increase it anyway.
	/*if (crouched)
	{
		if (round_acc(weap_inaccuracy) <= round_acc(sniper ? weapon->GetCSWeaponData()->flInaccuracyCrouchAlt : weapon->GetCSWeaponData()->flInaccuracyCrouch))
			return true;
	}
	else if (ctx.m_local()->m_vecVelocity().Length2D() < 1.f)
	{
		if (round_acc(weap_inaccuracy) <= round_acc(sniper ? weapon->GetCSWeaponData()->flInaccuracyStandAlt : weapon->GetCSWeaponData()->flInaccuracyStand))
			return true;
	}*/

	if (precomputed_mini_seeds.empty())
		return false;

	static std::tuple<float, float, float, float, float>* seed;
	static float c, spread_val, inaccuracy_val;
	static Vector v_spread, dir;

	Ray_t ray;

	for (int i = 0; i < 64; i++)
	{
		// get seed.
		seed = (&precomputed_mini_seeds[i]);

		//// calculate spread.
		//inaccuracy = std::get<0>(*seed) * weap_inaccuracy;
		//spread_x = std::get<2>(*seed) * inaccuracy;
		//spread_y = std::get<1>(*seed) * inaccuracy;
		//total_spread = (forward + right * spread_x + up * spread_y);

		//// calculate angle with spread applied.
		//Math::VectorAngles(total_spread, spread_angle);

		//// calculate end point of trace.
		////Math::AngleVectors(spread_angle, &end);
		//Math::AngleVectors(spread_angle, &direction);
		//end = eye_position + direction * 8092.f;

		c = std::get<0>(*seed);

		spread_val = c * weap_spread;
		inaccuracy_val = c * weap_inaccuracy;

		v_spread.Set((std::get<2>(*seed) * spread_val) + (std::get<4>(*seed) * inaccuracy_val), (std::get<3>(*seed) * spread_val) + (std::get<1>(*seed) * inaccuracy_val), 0);

		//dir.x = forward.x + (right.x * v_spread.x) + (up.x * v_spread.y);
		//dir.y = forward.y + (right.y * v_spread.x) + (up.y * v_spread.y);
		//dir.z = forward.z + (right.z * v_spread.x) + (up.z * v_spread.y);

		//Vector spread_view;
		//Math::VectorAngles(dir, spread_view);
		////spread_view.Normalize();
		//Math::AngleVectors(spread_view, &end);

		dir.Set(forward.x + (v_spread.x * right.x) + (v_spread.y * up.x),
			forward.y + (v_spread.x * right.y) + (v_spread.y * up.y),
			forward.z + (v_spread.x * right.z) + (v_spread.y * up.z));
		dir.NormalizeInPlace();
		//proper
		auto const end = ctx.m_eye_position + (dir * 8192.f);

		trace_t tr;
		ray.Init(ctx.m_eye_position, end);

		tr.fraction = 1.0;
		tr.startsolid = false;

		//csgo.m_engine_trace()->ClipRayToEntity(ray, CONTENTS_HITBOX, entity, &tr);
		auto intersect = Math::ClipRayToHitbox(ray, ht.hitbox, ent->m_CachedBoneData().Base()[ht.bone], tr) >= 0;

		//bool intersect;

		//if (ht.isOBB) {
		//	Vector delta;
		//	Math::VectorIRotate((dir * 8192.f), ent->m_CachedBoneData().Base()[ht.bone], delta);

		//	intersect = Math::IntersectBB(ht.start_scaled, delta, ht.mins, ht.maxs);
		//}
		//else
		//{
		//	intersect = Math::Intersect(ctx.m_eye_position, end, ht.mins, ht.maxs, ht.radius);

		//	//if (intersect && i % 2 == 0)
		//	//	intersect = feature::autowall->CanHit(eye_position, end, ctx.m_local(), ent, hitbox) > 1;
		//}

		/*Color color = Color::Red();

		switch (hitbox)
		{
		case 0:
			color = Color::Red();
			break;
		case 1:
			color = Color::Blue();
			break;
		case 2:
			color = Color::Grey();
			break;
		case 3:
			color = Color::Green();
			break;
		case 4:
			color = Color::Purple();
			break;
		case 5:
			color = Color::White();
			break;
		case 6:
			color = Color(255,130, 0);
			break;
		case 7:
			color = Color(100, 255, 0);
			break;
		}

		csgo.m_debug_overlay()->AddLineOverlay(eye_position, end, color.r(), color.g(), color.b(), true, csgo.m_globals()->interval_per_tick * 2.f);*/

		if (intersect)
			++traces_hit;
	}

	hc = /*int(float(float(*/traces_hit * 1.5625f;//) * 1.5625f));

	return true;
}

int c_aimbot::hitbox2hitgroup(C_BasePlayer* m_player, int ihitbox)
{
	if (ihitbox < 0 || ihitbox > 19) return 0;

	if (!m_player) return 0;

	const model_t* const model = m_player->GetModel();

	if (!model)
		return 0;

	studiohdr_t* const pStudioHdr = csgo.m_model_info()->GetStudioModel(model);

	if (!pStudioHdr)
		return 0;

	mstudiobbox_t* const hitbox = pStudioHdr->pHitbox(ihitbox, m_player->m_nHitboxSet());

	if (!hitbox)
		return 0;

	return hitbox->group;

	/*switch (ihitbox)
	{
	case HITBOX_HEAD:
	case HITBOX_NECK:
		return HITGROUP_HEAD;
	case HITBOX_UPPER_CHEST:
	case HITBOX_CHEST:
	case HITBOX_THORAX:
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_RIGHT_UPPER_ARM:
		return HITGROUP_CHEST;
	case HITBOX_PELVIS:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_THIGH:
	case HITBOX_BODY:
		return HITGROUP_STOMACH;
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_FOOT:
		return HITGROUP_LEFTLEG;
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_FOOT:
		return HITGROUP_RIGHTLEG;
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_HAND:
		return HITGROUP_LEFTARM;
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_RIGHT_HAND:
		return HITGROUP_RIGHTARM;
	default:
		return HITGROUP_STOMACH;
	}*/
}

int c_aimbot::safe_point(C_BasePlayer* entity, Vector eye_pos, Vector aim_point, int hitboxIdx, C_Tickrecord* record)
{
	resolver_records* resolve_info = &feature::resolver->player_records[entity->entindex() - 1];
	c_player_records* log = &feature::lagcomp->records[entity->entindex() - 1];

	if (!record || !record->data_filled || !record->valid)
		return 0;

	const auto is_colliding = [entity, hitboxIdx](Vector start, Vector end, C_Hitbox* hbox_data, matrix3x4_t* mx) -> bool
	{
		if (hbox_data->isOBB)
		{
			auto dir = end - start;
			dir.NormalizeInPlace();
			Vector delta;
			Math::VectorIRotate((dir * 8192.f), mx[hbox_data->bone], delta);

			return Math::IntersectBB(hbox_data->start_scaled, delta, hbox_data->mins, hbox_data->maxs);
		}
		else
		{
			if (Math::Intersect(ctx.m_eye_position, end, hbox_data->mins, hbox_data->maxs, hbox_data->radius))
				return true;
		}

		return false;
	};

	auto forward = aim_point - eye_pos;
	auto end = eye_pos + (forward * 8192.f);

	C_Hitbox box1; get_hitbox_data(&box1, entity, hitboxIdx, record->leftmatrixes);
	C_Hitbox box2; get_hitbox_data(&box2, entity, hitboxIdx, record->rightmatrixes);
	C_Hitbox box3; get_hitbox_data(&box3, entity, hitboxIdx, record->matrixes);

	int hits = 0;

	if (is_colliding(eye_pos, end, &box1, record->leftmatrixes)) ++hits;

	if (is_colliding(eye_pos, end, &box2, record->rightmatrixes)) ++hits;

	if (is_colliding(eye_pos, end, &box3, record->matrixes)) ++hits;

	return hits;
}

bool c_aimbot::safe_side_point(C_BasePlayer* entity, Vector eye_pos, Vector aim_point, int hitboxIdx, C_Tickrecord* record)
{
	resolver_records* resolve_info = &feature::resolver->player_records[entity->entindex() - 1];
	c_player_records* log = &feature::lagcomp->records[entity->entindex() - 1];

	if (!record || !record->data_filled || !record->valid)
		return false;
	
	const auto angle = Math::CalcAngle(eye_pos, aim_point);
	Vector forward;
	Math::AngleVectors(angle, &forward);
	auto const end(eye_pos + forward * 8192.f)/*(eye_pos.DistanceSquared(aim_point) * 1.01f)*/;

	C_Hitbox box1; get_hitbox_data(&box1, entity, hitboxIdx, record->leftmatrixes);
	C_Hitbox box2; get_hitbox_data(&box2, entity, hitboxIdx, record->rightmatrixes);
	C_Hitbox box3; get_hitbox_data(&box3, entity, hitboxIdx, record->matrixes);

	//C_Tickrecord rec;
	//cheat::features::lagcomp.store_record_data(entity, &rec);

	auto hits = 0;

	//if (pizdets ? Math::IntersectBB(eye_pos, end, box1.mins, box1.maxs) : Math::Intersect(eye_pos, end, box1.mins, box1.maxs, box1.radius))//if (is_colliding(eye_pos, end_point, &resolve_info->leftrec, &rec))
	//	++hits;
	//if (pizdets ? Math::IntersectBB(eye_pos, end, box2.mins, box2.maxs) : Math::Intersect(eye_pos, end, box2.mins, box2.maxs, box2.radius))//if (is_colliding(eye_pos, end_point, &resolve_info->rightrec, &rec))
	//	++hits;
	//if (pizdets ? Math::IntersectBB(eye_pos, end, box3.mins, box3.maxs) : Math::Intersect(eye_pos, end, box3.mins, box3.maxs, box3.radius))//if (is_colliding(eye_pos, end_point, &resolve_info->norec, &rec))
	//	++hits;

	//bool ok = false;
	if (box2.isOBB)
	{
		Vector delta1;
		Math::VectorIRotate((forward * 8192.f), record->leftmatrixes[box1.bone], delta1);

		Vector delta2;
		Math::VectorIRotate((forward * 8192.f), record->rightmatrixes[box2.bone], delta2);

		Vector delta3;
		Math::VectorIRotate((forward * 8192.f), record->matrixes[box3.bone], delta3);

		if (Math::IntersectBB(box1.start_scaled, delta1, box1.mins, box1.maxs))
			++hits;
		if (Math::IntersectBB(box2.start_scaled, delta2, box2.mins, box2.maxs))
			++hits;
		if (Math::IntersectBB(box3.start_scaled, delta3, box3.mins, box3.maxs))
			++hits;
	}
	else
	{
		if (Math::Intersect(eye_pos, end, box1.mins, box1.maxs, box1.radius))
			++hits;
		if (Math::Intersect(eye_pos, end, box2.mins, box2.maxs, box2.radius))
			++hits;
		if (Math::Intersect(eye_pos, end, box3.mins, box3.maxs, box3.radius))
			++hits;
	}

	//if (ok)
	//	++hits;

	return (hits >= 2);
}

bool c_aimbot::hit_chance(QAngle angle, Vector point, C_BasePlayer* ent, float chance, int hitbox, float damage, float* hc)
{
	//static float last_innacc = 0.f;

	if (chance < 1.f)
		return true;

	if (ctx.latest_weapon_data == nullptr || !m_weapon())
		return false;

	build_seed_table();

	C_Hitbox ht;
	get_hitbox_data(&ht, ent, hitbox, ent->m_CachedBoneData().Base());

	int traces_hit = 0;
	int awalls_hit = 0;
	int awall_traces_done = 0;

	static Vector forward, right, up;

	// performance optimization.
	if ((ctx.m_eye_position - ent->m_vecOrigin()).Length() > (ctx.latest_weapon_data->range * 1.02f))
		return false;

	Math::AngleVectors(angle, &forward, &right, &up);

	const float& weap_inaccuracy = Engine::Prediction::Instance()->GetInaccuracy();
	const auto weap_spread = Engine::Prediction::Instance()->GetSpread();

	auto is_special_weapon = m_weapon()->m_iItemDefinitionIndex() == 9
		|| m_weapon()->m_iItemDefinitionIndex() == 11
		|| m_weapon()->m_iItemDefinitionIndex() == 38
		|| m_weapon()->m_iItemDefinitionIndex() == 40;


	if (precomputed_seeds.empty())
		return false;

	static std::tuple<float, float, float, float, float>* seed;
	static float c, spread_val, inaccuracy_val;
	static Vector v_spread, dir, end;
	Ray_t ray;
	float average_spread = 0;

	int total = int(100.f * (chance / 100.f));

	//std::deque<Vector> hit_rays;
	for (auto i = 0; i < total_seeds; i++)
	{
		// get seed.
		seed = &precomputed_seeds[i];

		c = std::get<0>(*seed);

		spread_val = c * weap_spread;
		inaccuracy_val = c * weap_inaccuracy;

		v_spread = Vector((std::get<2>(*seed) * spread_val) + (std::get<4>(*seed) * inaccuracy_val), (std::get<3>(*seed) * spread_val) + (std::get<1>(*seed) * inaccuracy_val), 0);
		dir.x = forward.x + (v_spread.x * right.x) + (v_spread.y * up.x);
		dir.y = forward.y + (v_spread.x * right.y) + (v_spread.y * up.y);
		dir.z = forward.z + (v_spread.x * right.z) + (v_spread.y * up.z);

		dir.NormalizeInPlace();
		end = ctx.m_eye_position + (dir * 8192.f);

		bool intersect = false;

		if (ht.isOBB)
		{
			Vector delta;
			Math::VectorIRotate((dir * 8192.f), ent->m_CachedBoneData().Base()[ht.bone], delta);

			intersect = Math::IntersectBB(ht.start_scaled, delta, ht.mins, ht.maxs);
		}
		else
		{
			intersect = Math::Intersect(ctx.m_eye_position, end, ht.mins, ht.maxs, ht.radius);

		}

		if (intersect)
		{
			++traces_hit;
		}

		float final_chance = (float(traces_hit) / 100.f) * 100.f;

		if (final_chance >= chance)
		{
			hc[0] = final_chance;
			return true;
		}

		if ((100 - i + traces_hit) < total)
			return false;
	}

	/*if (chance <= final_chance) {
		hc[0] = final_chance;
		hc[1] = trace_chance;
		return true;
	}*/

	return false;
}
void c_aimbot::visualize_hitboxes(C_BasePlayer* entity, matrix3x4_t* mx, Color color, float time)
{
	const model_t* model = entity->GetModel();

	if (!model)
		return;

	const studiohdr_t* studioHdr = csgo.m_model_info()->GetStudioModel(model);

	if (!studioHdr)
		return;

	const mstudiohitboxset_t* set = studioHdr->pHitboxSet(entity->m_nHitboxSet());

	if (!set)
		return;

	for (int i = 0; i < set->numhitboxes; i++)
	{
		mstudiobbox_t* hitbox = set->pHitbox(i);

		if (!hitbox)
			continue;

		Vector min, max/*, center*/;
		Math::VectorTransform(hitbox->bbmin, mx[hitbox->bone], min);
		Math::VectorTransform(hitbox->bbmax, mx[hitbox->bone], max);

		if (hitbox->radius != -1)
			csgo.m_debug_overlay()->AddCapsuleOverlay(min, max, hitbox->radius, 255, 255, 255, 10, time, 0, 1);
	}
}
void c_aimbot::autostop(CUserCmd* cmd, C_WeaponCSBaseGun* local_weapon)
{
	static auto accel = csgo.m_engine_cvars()->FindVar(sxor("sv_accelerate"));
	//static float last_time_stopped = csgo.m_globals()->realtime;

	static bool was_onground = ctx.m_local()->m_fFlags() & FL_ONGROUND;

	//ctx.did_stop_before = false;

	/*if (csgo.m_client_state()->m_iChockedCommands > 0 && ctx.do_autostop && abs(last_time_stopped - csgo.m_globals()->realtime) > 0.5f)
	{
		for (auto i = 0; i < max(1,(int)feature::usercmd->command_numbers.size()); i++)
		{
			auto ucmd = feature::usercmd->command_numbers[i];
			auto cmd = csgo.m_input()->GetUserCmd(ucmd.command_number);
			auto vcmd = csgo.m_input()->GetVerifiedUserCmd(ucmd.command_number);

			auto v73 = cmd->sidemove;
			auto v74 = cmd->forwardmove;

			auto sqr = sqrtf((v73 * v73) + (v74 * v74));

			auto v81 = !ctx.m_local()->m_bIsScoped();
			auto v90 = 0.f;

			if (v81)
				v90 = m_weapon()->GetCSWeaponData()->max_speed;
			else
				v90 = m_weapon()->GetCSWeaponData()->max_speed_alt;

			const auto chocked_ticks = fabsf(ctx.last_sent_tick - csgo.m_globals()->tickcount);
			auto v234 = v90 * 0.32f;

			if (v234 <= (sqr + 1.0f))
			{
				auto v110 = (v73 * v73) + (v74 * v74);
				if (((cmd->upmove * cmd->upmove) + v110) > 0.0f)
				{
					cmd->buttons |= 0x20000u;
					if (v234 <= 0.1f)
					{
						v73 = cmd->sidemove * v234;
						v74 = cmd->forwardmove * v234;
					}
					else
					{
						auto v111 = sqrtf(v110);
						v74 = (cmd->forwardmove / v111) * v234;
						v73 = (cmd->sidemove / v111) * v234;
					}
				}
			}

			auto v112 = fminf(450.f, v74);

			if (v112 <= -450.0f)
				cmd->forwardmove = -450.0f;
			else
				cmd->forwardmove = v112;

			auto v113 = fminf(450.f, v73);

			if (v113 <= -450.0f)
				cmd->sidemove = -450.0f;
			else
				cmd->sidemove = v113;

			vcmd->m_cmd = *cmd;
			vcmd->m_crc = cmd->GetChecksum();
		}

		*(int*)((DWORD)csgo.m_prediction() + 0xC) = -1;
		*(int*)((DWORD)csgo.m_prediction() + 0x1C) = 0;
	}*/

	if ((!local_weapon->can_shoot() || m_weapon()->m_iItemDefinitionIndex() == 64)) {
		ctx.did_stop_before = false;
		ctx.do_autostop = false;
		return;
	}

	Engine::Prediction::Instance()->m_autostop_velocity_to_validate = 0.f;

	if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_quick_stop && local_weapon && local_weapon->m_iItemDefinitionIndex() != WEAPON_TASER && ctx.m_local()->m_fFlags() & FL_ONGROUND && was_onground && ctx.latest_weapon_data/* && !(cmd->buttons & IN_JUMP)*/)
	{
		auto v10 = cmd->buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD | IN_JUMP | IN_SPEED);
		cmd->buttons = v10;

		const auto chocked_ticks = (cmd->command_number % 3) == 0 ? (14 - csgo.m_client_state()->m_iChockedCommands) : ((14 - csgo.m_client_state()->m_iChockedCommands) / 2);
		const auto max_speed = (local_weapon->GetMaxSpeed() * 0.33f) - 1.f - (float(chocked_ticks) * (m_weapon()->m_iItemDefinitionIndex() == WEAPON_AWP ? 1.35f : 1.65f) * +ctx.m_local()->m_iShotsFired()/* * 1.2f*/);//, (0.32f - float(0.005f * chocked_ticks)));

		auto velocity = ctx.m_local()->m_vecVelocity();
		velocity.z = 0;
		auto current_speed = ((velocity.x * velocity.x) + (velocity.y * velocity.y));
		current_speed = sqrtf(current_speed);

		const auto cmd_speed = sqrtf((cmd->sidemove * cmd->sidemove) + (cmd->forwardmove * cmd->forwardmove));

		//if (feature::anti_aim->animation_speed <= 6.f && !ctx.did_stop_before) //accurate enough not to stop.
		//	return;

		auto new_sidemove = cmd->sidemove;
		auto new_forwardmove = cmd->forwardmove;

		if (current_speed >= 28.f) {
			if (current_speed <= max_speed)
			{
				if (current_speed > 0.0f)
				{
					//cmd->buttons |= IN_SPEED;
					if (current_speed <= 0.1f)
					{
						new_sidemove = cmd->sidemove * fminf(current_speed, max_speed);
						new_forwardmove = cmd->forwardmove * fminf(current_speed, max_speed);
					}
					else
					{
						new_forwardmove = (cmd->forwardmove / cmd_speed) * fminf(current_speed, max_speed);
						new_sidemove = (cmd->sidemove / cmd_speed) * fminf(current_speed, max_speed);
					}
				}
			}
			else
			{
				QAngle angle;
				Math::VectorAngles(velocity, angle);

				// fix direction by factoring in where we are looking.
				angle.y = ctx.cmd_original_angles.y - angle.y;

				// convert corrected angle back to a direction.
				Vector direction;
				Math::AngleVectors(angle, &direction);

				if (current_speed > 5.f) {
					auto stop = direction * -current_speed;

					new_forwardmove = stop.x;
					new_sidemove = stop.y;
				}
				else
				{
					new_forwardmove = 0;
					new_sidemove = 0;
				}
			}
		}

		if (ctx.m_local()->m_bDucking()
			|| ctx.m_local()->m_fFlags() & FL_DUCKING) {
			new_forwardmove = new_forwardmove / (((ctx.m_local()->m_flDuckAmount() * 0.34f) + 1.0f) - ctx.m_local()->m_flDuckAmount());
			new_sidemove = new_sidemove / (((ctx.m_local()->m_flDuckAmount() * 0.34f) + 1.0f) - ctx.m_local()->m_flDuckAmount());
		}

		cmd->sidemove = Math::clamp(new_sidemove, -450.f, 450.f);
		cmd->forwardmove = Math::clamp(new_forwardmove, -450.f, 450.f);

		ctx.did_stop_before = true;
		ctx.last_autostop_tick = cmd->command_number;
		ctx.do_autostop = false;
	}

	was_onground = (ctx.m_local()->m_fFlags() & FL_ONGROUND);
}

std::string hitbox_to_string(int h)
{
	switch (h)
	{
	case 0:
		return "head";
		break;
	case 1:
		return "neck";
		break;
	case HITBOX_RIGHT_FOOT:
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_THIGH:
		return "right leg";
		break;
	case HITBOX_LEFT_FOOT:
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_THIGH:
		return "left leg";
		break;
	case HITBOX_RIGHT_HAND:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_FOREARM:
		return "right hand";
		break;
	case HITBOX_LEFT_HAND:
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_UPPER_ARM:
		return "left hand";
		break;
	case HITBOX_CHEST:
		return "lower chest";
	case HITBOX_UPPER_CHEST:
		return "upper chest";
		break;
	default:
		return "body";
		break;
	}
}
Vector get_bone(int bone, matrix3x4_t mx[])
{
	return Vector(mx[bone][0][3], mx[bone][1][3], mx[bone][2][3]);
}
TargetListing_t::TargetListing_t(C_BasePlayer* ent)
{
	entity = ent;

	hp = min(100, entity->m_iHealth());
	idx = entity->entindex();
}
void c_aimbot::OnRoundStart(C_BasePlayer* player) {
	m_target = nullptr;
	m_hitboxes.clear();
	// IMPORTANT: DO NOT CLEAR LAST HIT SHIT.
}

void c_aimbot::SetupHitboxes(C_BasePlayer* ent, C_Tickrecord* record, bool history) {

	// reset hitboxes.
	m_hitboxes.clear();

	if (!record)
		return;

	bool is_distance_invalid = ctx.m_local()->m_vecOrigin().Distance(ent->m_vecOrigin()) / 12 >= 30;


	if (m_weapon()->m_iItemDefinitionIndex() == WEAPON_TASER) {
		// hitboxes for the zeus.
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::NORMAL });
		return;
	}
	if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(2))
	{
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::LETHAL });
	}

	if (ctx.exploit_allowed && ctx.has_exploit_toggled && ctx.main_exploit >= 2 && !ctx.fakeducking && m_weapon()->WeaponGroup() == 0)
	{
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::LETHAL2 });
	}

	if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_prefer_safe_point)
	{
		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(0))
		{
			m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::SAFETY });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(1) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_CHEST, HitscanMode::SAFETY });
			m_hitboxes.push_back({ HITBOX_UPPER_CHEST, HitscanMode::SAFETY });

		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(2))
		{
			m_hitboxes.push_back({ HITBOX_THORAX, HitscanMode::SAFETY });
			m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::SAFETY });
			m_hitboxes.push_back({ HITBOX_PELVIS, HitscanMode::SAFETY });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(3) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_UPPER_ARM, HitscanMode::SAFETY });
			m_hitboxes.push_back({ HITBOX_RIGHT_UPPER_ARM, HitscanMode::SAFETY });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(4) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_THIGH, HitscanMode::SAFETY });
			m_hitboxes.push_back({ HITBOX_RIGHT_THIGH, HitscanMode::SAFETY });
			m_hitboxes.push_back({ HITBOX_LEFT_CALF,  HitscanMode::SAFETY });
			m_hitboxes.push_back({ HITBOX_RIGHT_CALF,  HitscanMode::SAFETY });
		}
		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(5) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_FOOT,  HitscanMode::SAFETY });
			m_hitboxes.push_back({ HITBOX_RIGHT_FOOT,  HitscanMode::SAFETY });
		}

		return;
	}


	if (m_weapon()->WeaponGroup() == 1)
	{
		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(0))
		{
			m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::SAFETY });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(1) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_CHEST, HitscanMode::SEMI_SAFETY });
			m_hitboxes.push_back({ HITBOX_UPPER_CHEST, HitscanMode::SEMI_SAFETY });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(2))
		{
			m_hitboxes.push_back({ HITBOX_THORAX, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_PELVIS, HitscanMode::NORMAL });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(3) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_UPPER_ARM, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_RIGHT_UPPER_ARM, HitscanMode::NORMAL });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(4) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_THIGH, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SAFETY : HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_RIGHT_THIGH, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SAFETY : HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_LEFT_CALF, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SAFETY : HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_RIGHT_CALF,  ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SAFETY : HitscanMode::NORMAL });
		}
		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(5) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_FOOT, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SAFETY : HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_RIGHT_FOOT, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SAFETY : HitscanMode::NORMAL });
		}

		return;
	}
	else
	{
		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(0))
		{
			m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::NORMAL });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(1) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_CHEST, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_UPPER_CHEST, HitscanMode::NORMAL });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(2))
		{
			m_hitboxes.push_back({ HITBOX_THORAX, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_PELVIS, HitscanMode::NORMAL });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(3) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_UPPER_ARM, HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_RIGHT_UPPER_ARM, HitscanMode::NORMAL });
		}

		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(4) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_THIGH,ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SEMI_SAFETY : HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_RIGHT_THIGH, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SEMI_SAFETY : HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_LEFT_CALF, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SEMI_SAFETY : HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_RIGHT_CALF,  ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SEMI_SAFETY : HitscanMode::NORMAL });
		}
		if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_target_hitbox.at(5) && !is_distance_invalid)
		{
			m_hitboxes.push_back({ HITBOX_LEFT_FOOT, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SEMI_SAFETY : HitscanMode::NORMAL });
			m_hitboxes.push_back({ HITBOX_RIGHT_FOOT, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_on_limbs ? HitscanMode::SEMI_SAFETY : HitscanMode::NORMAL });
		}
	}
}

void c_aimbot::init() {
	// clear old targets.
	m_targets[0].entity = nullptr;
	m_targets_count = 0;

	m_target = nullptr;
	m_aim = Vector{};
	m_angle = QAngle{};
	m_damage = 0.f;
	//ctx.do_autostop = false;

	m_baim_key = false;
	m_damage_key = false;
	m_safety_key = false;

	m_best_dist = FLT_MAX;
	m_best_fov = 180.f + 1.f;
	m_best_damage = 0.f;
	m_best_hp = 100 + 1;
	m_best_lag = FLT_MAX;
	m_best_height = FLT_MAX;

	if (ctx.last_rage_matrix != nullptr)
		ctx.last_rage_matrix->Clear();
}

void c_aimbot::StripAttack(CUserCmd* cmd)
{
	if (m_weapon()->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		return;
	else
		cmd->buttons &= ~IN_ATTACK;
}

bool c_aimbot::think(CUserCmd* cmd, bool* send_packet) {
	// do all startup routines.
	init();

	m_damage_key = ctx.get_key_press(ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_min_damage_override);

	if (m_damage_key)
		ctx.active_keybinds[6].mode = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_min_damage_override.mode + 1;

	m_safety_key = ctx.get_key_press(ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_key);

	if (m_safety_key)
		ctx.active_keybinds[2].mode = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_force_safe_point_key.mode + 1;

	// we have no aimbot enabled.
	if (!ctx.m_settings.aimbot_enabled)
		return false;

	for (int i{ 1 }; i <= 64; ++i) {
		auto player = csgo.m_entity_list()->GetClientEntity(i);
		auto r_log = &feature::resolver->player_records[i - 1];

		if (!player ||
			player->IsDormant() ||
			!player->IsPlayer() ||
			player->m_iHealth() <= 0 ||
			player->m_iTeamNum() == ctx.m_local()->m_iTeamNum() ||
			player->m_bGunGameImmunity()) {

			continue;
		}

		auto data = &feature::lagcomp->records[i - 1];

		if (!data || data->player != player || data->records_count < 1) {
			continue;
		}


		if (m_weapon()->is_knife() && m_weapon()->m_iItemDefinitionIndex() != WEAPON_TASER && ctx.m_settings.miscellaneous_knifebot) {
			knife(send_packet, player, cmd);
		}

		if (ctx.shots_fired[i - 1] < 1)
			r_log->last_shot_missed = false;

		// store player as potential target this tick.
		m_targets[m_targets_count++] = player;
	}

	// sanity.
	if (!m_weapon() || cmd->weaponselect > 0) {
		ctx.m_last_shot_index = -1;
		return false;
	}

	if (abs(cmd->command_number - ctx.m_ragebot_shot_nr) > 40)
		ctx.m_last_shot_index = -1;


	//
		// no grenades or bomb.
	if (!m_weapon()->IsGun() || ctx.air_stuck) {
		ctx.m_last_shot_index = -1;
		return false;
	}

	// no point in aimbotting if we cannot fire this tick.
	if ((ctx.fakeducking) && feature::anti_aim->did_shot_in_chocked_cycle)
		return false;

	if (!m_weapon()->can_shoot())
	{
		if (m_weapon()->m_reloadState() != 0) {
			return false;
		}

		ctx.shot_angles.clear();
	}

	// no grenades or bomb.
	if (!m_weapon()->IsGun() || ctx.air_stuck) {
		ctx.m_last_shot_index = -1;
		return false;
	}

	// no point in aimbotting if we cannot fire this tick.
	if ((ctx.fakeducking) && feature::anti_aim->did_shot_in_chocked_cycle)
		return false;

	if (!m_weapon()->can_shoot())
	{
		if (m_weapon()->m_reloadState() != 0) {
			return false;
		}

		ctx.shot_angles.clear();
	}

	// setup bones for all valid targets.

	ctx.do_autostop = true;

	// scan available targets... if we even have any.
	find(cmd);

	if (ctx.m_settings.fake_lag_shooting && feature::anti_aim->did_shot_in_chocked_cycle) {
		cmd->buttons &= ~IN_ATTACK;
		ctx.do_autostop = false;
	}

	apply(cmd, send_packet);

	if (!m_target || m_damage == 0)
		ctx.do_autostop = false;

	return m_target && m_damage > 0 && cmd->buttons & IN_ATTACK;
}

void c_aimbot::find(CUserCmd* cmd) {
	struct BestTarget_t { C_BasePlayer* player; Vector pos; float damage; int hitbox; C_Tickrecord* best_record; };

	Vector       tmp_pos;
	float        tmp_damage;
	int				tmp_hitbox;
	BestTarget_t best;
	best.player = nullptr;
	best.damage = -1.f;
	best.pos = Vector{};
	best.hitbox = -1;
	best.best_record = nullptr;

	BestTarget_t last_record;
	last_record.player = nullptr;
	last_record.damage = -1.f;
	last_record.pos = Vector{};
	last_record.hitbox = -1;
	last_record.best_record = nullptr;

	BestTarget_t history_record;
	history_record.player = nullptr;
	history_record.damage = -1.f;
	history_record.pos = Vector{};
	history_record.hitbox = -1;
	history_record.best_record = nullptr;

	int players_iterated = 0;

	if (m_targets[0].entity == nullptr || m_targets_count < 1)
		return;

	//if (g_cl.m_weapon_id == ZEUS && !g_menu.main.aimbot.zeusbot.get())
	//	return;

	// iterate all targets.
	for (auto i = 0; i < m_targets_count; i++) {
		auto& target = m_targets[i];
		auto t = m_targets[i].entity;

		if (!t ||
			t->IsDormant() ||
			!t->IsPlayer() ||
			t->m_iHealth() <= 0 ||
			t->m_iTeamNum() == ctx.m_local()->m_iTeamNum() ||
			t->m_bGunGameImmunity()
			) continue;

		auto data = &feature::lagcomp->records[target.idx - 1];
		auto rdata = &feature::resolver->player_records[target.idx - 1];

		if (!data || data->records_count < 1 || data->player != t)
			continue;

		data->best_record = nullptr;

		C_Tickrecord* last = feature::resolver->find_first_available(t, data, false);

		if (!last || !last->data_filled || last->dormant)
			continue;

		SetupHitboxes(t, last, false);

		if (m_hitboxes.empty())
			continue;

		players_iterated++;

		if (GetBestAimPosition(t, tmp_pos, tmp_damage, tmp_hitbox, last, players_iterated) && SelectTarget(t, last, tmp_pos, tmp_damage)) {
			// if we made it so far, set shit.
			/*best.player = t;
			best.pos = tmp_pos;
			best.damage = tmp_damage;
			data->best_record = last;
			best.hitbox = tmp_hitbox;*/
			last_record.player = t;
			last_record.pos = tmp_pos;
			last_record.damage = tmp_damage;
			last_record.best_record = last;
			last_record.hitbox = tmp_hitbox;
			//break;
		}

		if (Engine::Prediction::Instance()->m_flFrameTime > csgo.m_globals()->interval_per_tick && last && last->data_filled && !last->breaking_lc || !m_weapon()->can_shoot())
		{
			best.player = last_record.player;
			best.pos = last_record.pos;
			best.damage = last_record.damage;
			best.best_record = last;
			best.hitbox = last_record.hitbox;
			continue;
		}

		C_Tickrecord* old = feature::resolver->find_first_available(t, data, true);

		if (!old || !old->data_filled || old->dormant || old->breaking_lc)
		{
			best.player = last_record.player;
			best.pos = last_record.pos;
			best.damage = last_record.damage;
			best.best_record = last;
			best.hitbox = last_record.hitbox;
			continue;
		}

		SetupHitboxes(t, old, false);

		if (m_hitboxes.empty())
		{
			best.player = last_record.player;
			best.pos = last_record.pos;
			best.damage = last_record.damage;
			best.best_record = last;
			best.hitbox = last_record.hitbox;
			continue;
		}

		if (GetBestAimPosition(t, tmp_pos, tmp_damage, tmp_hitbox, old, players_iterated) && SelectTarget(t, old, tmp_pos, tmp_damage)) {
			// if we made it so far, set shit.
			history_record.player = t;
			history_record.pos = tmp_pos;
			history_record.damage = tmp_damage;
			history_record.best_record = old;
			//data->best_record = old;
			history_record.hitbox = tmp_hitbox;
			break;
		}
	}
	if (history_record.damage >= last_record.damage)
	{
		best.player = history_record.player;
		best.pos = history_record.pos;
		best.damage = history_record.damage;
		best.hitbox = history_record.hitbox;
		best.best_record = history_record.best_record;
	}
	else
	{
		best.player = last_record.player;
		best.pos = last_record.pos;
		best.damage = last_record.damage;
		best.hitbox = last_record.hitbox;
		best.best_record = last_record.best_record;
	}
	// verify our target and set needed data.
	if (best.player && ctx.m_local()) {
		auto data = &feature::lagcomp->records[best.player->entindex() - 1];
		auto r_data = &feature::resolver->player_records[best.player->entindex() - 1];

		// calculate aim angle.
		Math::VectorAngles(best.pos - ctx.m_eye_position, m_angle);

		// set member vars.
		m_target = best.player;
		m_aim = best.pos;
		m_damage = best.damage;
		m_hitbox = best.hitbox;
		data->best_record = best.best_record;
		//m_record = best.record;

		//csgo.m_debug_overlay()->AddBoxOverlay(m_aim, Vector(-2, -2, -2), Vector(2, 2, 2), Vector(0, 0, 0), 255, 0, 0, 255, 2.f * csgo.m_globals()->frametime);

		if (m_damage > 0) {
			// write data, needed for traces / etc.
			data->best_record->apply(m_target, false);

			const auto cur_mul = float(min(100, m_target->m_iHealth())) / m_damage;
			float ndmg = 1;

			auto velocity = Engine::Prediction::Instance()->GetVelocity();

			//ctx.last_aim_state = 1;

			if (auto animstate = ctx.m_local()->get_animation_state(); animstate != nullptr && animstate->m_player && m_weapon()->can_shoot()) {
				if (ctx.m_local()->should_fix_modify_eye_pos()) {

					const auto oldposeparam = *(float*)(uintptr_t(ctx.m_local()) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48));
					auto eye_pitch = Math::normalize_angle(m_angle.x + ctx.m_local()->m_viewPunchAngle().x);

					auto angles = QAngle(0.f, ctx.m_local()->get_animation_state()->m_abs_yaw, 0);
					ctx.m_local()->set_abs_angles(angles);

					if (eye_pitch > 180.f)
						eye_pitch = eye_pitch - 360.f;

					eye_pitch = Math::clamp(eye_pitch, -89, 89);
					*(float*)(uintptr_t(ctx.m_local()) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48)) = Math::clamp((eye_pitch + 89.f) / 176.f, 0.0f, 1.0f);

					ctx.m_local()->force_bone_rebuild();

					const auto absorg = ctx.m_local()->get_abs_origin();
					ctx.m_local()->set_abs_origin(ctx.m_local()->m_vecOrigin());
					ctx.m_local()->SetupBonesEx(0x100);
					ctx.m_local()->set_abs_origin(absorg);

					ctx.m_local()->force_bone_cache();

					ctx.m_eye_position = ctx.m_local()->GetEyePosition(); //call weapon_shootpos
					*(float*)(uintptr_t(ctx.m_local()) + (Engine::Displacement::DT_CSPlayer::m_flPoseParameter + 48)) = oldposeparam;

					Math::VectorAngles(best.pos - ctx.m_eye_position, m_angle);
					//m_angle = Math::CalcAngle(ctx.m_eye_position, m_aim);
					//}
				}
			}

			//if (cur_mul >= 1.0f &&
			//	velocity.Length2D() > 0
			//	&& cmd->buttons & (IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD) && !(cmd->buttons & IN_DUCK))
			//{
			//	velocity.z = 0.f;
			//	auto pred = ctx.m_eye_position + (velocity * 0.2f);

			//	Ray_t ray;
			//	ray.Init(ctx.m_eye_position, pred, ctx.m_local()->OBBMins(), ctx.m_local()->OBBMaxs());

			//	CTraceFilter filter;
			//	filter.pSkip = ctx.m_local();
			//	trace_t tr;

			//	csgo.m_engine_trace()->TraceRay(ray, MASK_PLAYERSOLID, &filter, &tr);

			//	if (tr.DidHit())
			//	{
			//		const auto frac = (tr.fraction * 0.2f) * 0.95f;
			//		pred = ctx.m_eye_position + (velocity * frac);
			//	}

			//	ray.Init(ctx.m_eye_position, ctx.m_eye_position - Vector(0, 0, 500.f), Vector(-2, -2, -2), Vector(2, 2, 2));
			//	csgo.m_engine_trace()->TraceRay(ray, MASK_PLAYERSOLID, &filter, &tr);

			//	pred.z = ctx.m_eye_position.z;

			//	if (tr.DidHit())
			//	{
			//		//const auto frac = (tr.fraction * 0.2f) * 0.94999999f;
			//		//pred.z = tr.endpos.z * frac;
			//		//pred.z += ctx.m_eye_position.z - ctx.m_local()->m_vecOrigin().z;
			//		pred.z = tr.endpos.z + (ctx.m_eye_position.z - ctx.m_local()->m_vecOrigin().z);
			//	}

			//	ctx.force_low_quality_autowalling = true;
			//	ndmg = feature::autowall->CanHit(pred, m_aim, ctx.m_local(), m_target, m_hitbox);

			//	ctx.force_low_quality_autowalling = false;
			//	//csgo.m_debug_overlay()->AddBoxOverlay(pred, Vector(-2, -2, -2), Vector(2, 2, 2), Vector(0, 0, 0), 255, 0, 0, 255, csgo.m_globals()->frametime * 2.f);
			//}

			const auto new_mul = float(min(100, m_target->m_iHealth())) / ndmg;

			//if (cur_mul <= new_mul || cur_mul <= 1.0f || ctx.exploit_allowed && ctx.main_exploit >= 2 && cur_mul < 2.f)
			//	ctx.last_aim_state = 2;

			//csgo.m_debug_overlay()->AddBoxOverlay(ctx.m_eye_position + (best.pos - ctx.m_eye_position).Normalized() * 8192.f, Vector(-2, -2, -2), Vector(2, 2, 2), Vector(0, 0, 0), 255, 0, 0, 255, 2.f);

			m_best_hc[0] = 0;
			m_best_hc[1] = 0;

			bool hc_result = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_hit_chance > 0.f && hit_chance(m_angle, m_aim, m_target, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_hit_chance, best.hitbox, 1, m_best_hc);

			ctx.do_autostop = true;
			ctx.shot_angles = m_angle;

			auto is_zoomable_weapon = (m_weapon()->m_iItemDefinitionIndex() == WEAPON_SSG08
				|| m_weapon()->m_iItemDefinitionIndex() == WEAPON_AWP
				|| m_weapon()->m_iItemDefinitionIndex() == WEAPON_SCAR20
				|| m_weapon()->m_iItemDefinitionIndex() == WEAPON_G3SG1);

			auto sniper = (m_weapon()->m_iItemDefinitionIndex() == WEAPON_SSG08 || m_weapon()->m_iItemDefinitionIndex() == WEAPON_AWP);

			if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_automatic_scope
				&& is_zoomable_weapon
				&& !ctx.m_local()->m_bIsScoped()
				&& !ctx.m_local()->m_bResumeZoom())
			{
				cmd->buttons |= IN_ATTACK2;
				cmd->buttons &= ~IN_ATTACK;
				return;
			}


			if (hc_result)
			{
				if (!m_weapon()->m_iClip1())
				{
					if (m_weapon()->m_iPrimaryReserveAmmoCount() > 0)
						cmd->buttons = cmd->buttons & ~1 | 0x2000;
					else
						ctx.do_autostop = false;

					return;
				}

				ctx.last_aim_state = 3;

				if (ctx.m_last_shot_time + 0.5f > csgo.m_globals()->curtime && ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_delay_shot)
					return;

				if (m_weapon()->can_shoot() && ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_automatic_fire) {
					cmd->buttons |= IN_ATTACK;
				}
			}
		}
	}
	else
		ctx.shot_angles.clear();
}

bool c_aimbot::get_points(C_BasePlayer* player, int ihitbox, std::vector<scan_point>& points, matrix3x4_t mx[])
{
	points.clear();

	if (!player || !ctx.latest_weapon_data)
		return false;

	const model_t* model = player->GetModel();

	if (!model)
		return false;

	studiohdr_t* pStudioHdr = csgo.m_model_info()->GetStudioModel(model);

	if (!pStudioHdr)
		return false;

	mstudiobbox_t* hitbox = pStudioHdr->pHitbox(ihitbox, player->m_nHitboxSet());

	if (!hitbox)
		return false;

	static float POINT_SCALE = 0.f;//(r.has_been_resolved ? ctx.m_settings.aimbot_resolved_pointscale : ctx.m_settings.aimbot_pointscale) * 0.01f;

	switch (ihitbox)
	{
	case HITBOX_HEAD:
	case HITBOX_NECK:
		POINT_SCALE = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point.at(0) ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_scale : 0.f;
		break;
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
		POINT_SCALE = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point.at(1) ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_scale : 0.f;
		break;
	case HITBOX_PELVIS:
		POINT_SCALE = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point.at(2) ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_scale : 0.f;
		break;
	case HITBOX_BODY:
	case HITBOX_THORAX:
		POINT_SCALE = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point.at(2) ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_scale : 0.f;
		break;
	case HITBOX_RIGHT_HAND:
	case HITBOX_LEFT_HAND:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_FOREARM:
		POINT_SCALE = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point.at(3) ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_scale : 0.f;
		break;
	case HITBOX_RIGHT_THIGH:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_LEFT_CALF:
		POINT_SCALE = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point.at(4) ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_scale : 0.f;
		break;
	case HITBOX_RIGHT_FOOT:
	case HITBOX_LEFT_FOOT:
		POINT_SCALE = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point.at(5) ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_scale : 0.f;
		break;
	default:
		break;
	}

	POINT_SCALE *= 0.01f;


	auto center = (hitbox->bbmin + hitbox->bbmax) * 0.5f;

	//if (force_pointscale > 0.f)
	//	POINT_SCALE = force_pointscale * 0.01f;

	// these indexes represent boxes.
	if (hitbox->radius == -1.f) {
		// convert rotation angle to a matrix.
		matrix3x4_t rot_matrix;
		Math::AngleMatrix(hitbox->rotation, rot_matrix);

		// apply the rotation to the entity input space (local).
		matrix3x4_t matrix;
		Math::ConcatTransforms(mx[hitbox->bone], rot_matrix, matrix);

		// extract origin from matrix.
		const Vector origin = matrix.GetOrigin();

		// compute raw center point.
		Vector center = (hitbox->bbmin + hitbox->bbmax) / 2.f;

		// the feet hiboxes have a side, heel and the toe.
		if (ihitbox == HITBOX_RIGHT_FOOT || ihitbox == HITBOX_LEFT_FOOT) {


			auto side = (hitbox->bbmin.z - center.z) * 0.875f;

			if (ihitbox == HITBOX_LEFT_FOOT)
				side = -side;

			points.emplace_back(scan_point(Vector(center.x, center.y, center.z + side), ihitbox, true));

			auto min = (hitbox->bbmin.x - center.x) * 0.875f;
			auto max = (hitbox->bbmax.x - center.x) * 0.875f;

			points.emplace_back(scan_point(Vector(center.x + min, center.y, center.z), ihitbox, false));
			points.emplace_back(scan_point(Vector(center.x + max, center.y, center.z), ihitbox, false));

			const float d2 = (hitbox->bbmin.x - center.x) * POINT_SCALE;
			const float d3 = (hitbox->bbmax.x - center.x) * POINT_SCALE;
		}

		// nothing to do here we are done.
		if (points.empty())
			return false;
	}
	else
	{
		if (POINT_SCALE <= 0.0f) //-V648
		{
			Math::VectorTransform(center, mx[hitbox->bone], center);
			points.emplace_back(scan_point(center, ihitbox, true));

			return true;
		}
		bool can_only_centre = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_multi_point_strange == 0;


		auto final_radius = hitbox->radius * POINT_SCALE;
		if (ihitbox == HITBOX_HEAD)
		{
			if (can_only_centre)
			{
				points.emplace_back(scan_point(center, ihitbox, true));
			}
			else
			{
				points.emplace_back(scan_point(center, ihitbox, true));
				points.emplace_back(scan_point(Vector(hitbox->bbmax.x + 0.70710678f * final_radius, hitbox->bbmax.y - 0.70710678f * final_radius, hitbox->bbmax.z), ihitbox, false));

				points.emplace_back(scan_point(Vector(hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmax.z + final_radius), ihitbox, false));
				points.emplace_back(scan_point(Vector(hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmax.z - final_radius), ihitbox, false));

				points.emplace_back(scan_point(Vector(hitbox->bbmax.x, hitbox->bbmax.y - final_radius, hitbox->bbmax.z), ihitbox, false));
			}
		}
		if ((ihitbox == HITBOX_CHEST || ihitbox == HITBOX_UPPER_CHEST))
		{
			if (can_only_centre)
			{
				points.emplace_back(scan_point(center, ihitbox, true));
			}
			else
			{
				points.emplace_back(scan_point(center, ihitbox, true));
				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + final_radius), ihitbox, false));
				points.emplace_back(scan_point(Vector(center.x, center.y, center.z - final_radius), ihitbox, false));


				points.emplace_back(scan_point(Vector(center.x, center.y + final_radius, center.z), ihitbox, false));
				points.emplace_back(scan_point(Vector(center.x, center.y - final_radius, center.z), ihitbox, false));
			}
		}
		if (ihitbox == HITBOX_THORAX)
		{
			if (can_only_centre)
			{
				points.emplace_back(scan_point(center, ihitbox, true));
			}
			else
			{
				points.emplace_back(scan_point(center, ihitbox, true));
				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + final_radius), ihitbox, false));
				points.emplace_back(scan_point(Vector(center.x, center.y, center.z - final_radius), ihitbox, false));


				points.emplace_back(scan_point(Vector(center.x, center.y + final_radius, center.z), ihitbox, false));
				points.emplace_back(scan_point(Vector(center.x, center.y - final_radius, center.z), ihitbox, false));
			}
		}
		if (ihitbox == HITBOX_PELVIS)
		{
			if (can_only_centre)
			{
				points.emplace_back(scan_point(center, ihitbox, true));

			}
			else
			{
				points.emplace_back(scan_point(center, ihitbox, true));

				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + final_radius), ihitbox, false));
				points.emplace_back(scan_point(Vector(center.x, center.y, center.z - final_radius), ihitbox, false));


				points.emplace_back(scan_point(Vector(center.x, center.y + final_radius, center.z), ihitbox, false));
				points.emplace_back(scan_point(Vector(center.x, center.y - final_radius, center.z), ihitbox, false));
			}
		}
		if ((ihitbox == HITBOX_RIGHT_UPPER_ARM || ihitbox == HITBOX_LEFT_UPPER_ARM))
		{
			if (can_only_centre)
			{
				points.emplace_back(scan_point(center, ihitbox, true));
			}
			else
			{
				points.emplace_back(scan_point(center, ihitbox, true));
				points.emplace_back(scan_point(Vector(hitbox->bbmax.x + final_radius, center.y, center.z), ihitbox, false));
			}
		}
		if ((ihitbox == HITBOX_RIGHT_CALF || ihitbox == HITBOX_LEFT_CALF))
		{
			if (can_only_centre)
			{
				points.emplace_back(scan_point(center, ihitbox, true));
			}
			else
			{
				points.emplace_back(scan_point(center, ihitbox, true));
				points.emplace_back(scan_point(Vector(hitbox->bbmax.x - final_radius, hitbox->bbmax.y, hitbox->bbmax.z), ihitbox, false));
			}
		}
		if ((ihitbox == HITBOX_RIGHT_FOOT || ihitbox == HITBOX_LEFT_FOOT))
		{
			auto side = (hitbox->bbmin.z - center.z) * 0.875f;

			if (ihitbox == HITBOX_LEFT_FOOT)
				side = -side;

			auto min = (hitbox->bbmin.x - center.x) * 0.875f;
			auto max = (hitbox->bbmax.x - center.x) * 0.875f;

			if (can_only_centre)
			{
				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + side), ihitbox, true));
			}
			else
			{
				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + side), ihitbox, true));

				points.emplace_back(scan_point(Vector(center.x + min, center.y, center.z), ihitbox, false));
				points.emplace_back(scan_point(Vector(center.x + max, center.y, center.z), ihitbox, false));
			}
		}
	}

	for (auto& point : points)
		Math::VectorTransform(point.point, mx[hitbox->bone], point.point);

	return true;
}

bool c_aimbot::GetBestAimPosition(C_BasePlayer* player, Vector& aim, float& damage, int& hitbox, C_Tickrecord* record, int players_iterated) {
	bool                  done, pen;
	float                 dmg, pendmg;
	HitscanData_t         scan;
	scan.m_hitchance = 0;
	//std::vector< Vector > points;
	//std::vector<scan_point> points;
	std::vector< Vector > points;
	//memset(&scan.m_safepoint[0], false, sizeof(bool) * 19);

	// get player hp.
	int hp = min(100, player->m_iHealth());

	scan.m_damage = 0;

	auto data = &feature::lagcomp->records[player->entindex() - 1];
	auto rdata = &feature::resolver->player_records[player->entindex() - 1];

	if (m_weapon()->m_iItemDefinitionIndex() == WEAPON_TASER) {
		dmg = pendmg = 110;
		pen = false;
	}
	else {

		dmg = min(hp > 99 ? hp + 10 : hp + 1, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_damage < 101 ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_damage : hp + ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_damage - 100);

		pendmg = min(hp > 99 ? hp + 10 : hp + 1, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_autowall_damage < 101 ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_autowall_damage : hp + ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_autowall_damage - 100);

		if (m_damage_key)
			pendmg = dmg = min(ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_override_damage < 101 ? ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_override_damage : hp + ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_override_damage - 100, hp > 99 ? hp + 10 : hp);

		pen = ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_automatic_penetration;
	}
	record->apply(player, false);
	data->is_restored = false;

	bool had_any_dmg = false;

	// find hitboxes.
	for (const auto& it : m_hitboxes) {
		done = false;

		bool retard;

		if (it.m_index == HITBOX_HEAD) {

			// nope we did not hit head..
			if (m_baim_key)
				continue;
		}

		const model_t* const model = player->GetModel();

		if (!model)
			continue;

		studiohdr_t* const pStudioHdr = csgo.m_model_info()->GetStudioModel(model);

		if (!pStudioHdr)
			continue;

		mstudiobbox_t* const hitbox = pStudioHdr->pHitbox(it.m_index, player->m_nHitboxSet());

		if (!hitbox)
			continue;

		C_Hitbox ht;

		get_hitbox_data(&ht, player, it.m_index, record->resolved_matrix);

		if (ht.radius < m_weapon()->GetSpread() * 1000.f)
			continue;

		// setup points on hitbox.
		if (!player->get_multipoints(it.m_index, points, record->resolved_matrix, retard))
			continue;

		int points_this_hitbox = 0;

		scan.m_safepoint[it.m_index] = false;

		// iterate points on hitbox.
		for (auto& point : points) {
			int safepoints = safe_point(player, ctx.m_eye_position, point, it.m_index, record);

			if (safepoints == 0)
				continue;

			if (safepoints < 3 && m_safety_key)
				continue;

			float wall_dmg = pendmg;
			float just_dmg = dmg;

			if (m_damage_key)
				just_dmg = wall_dmg = min(hp, ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_minimum_damage / 5);

			bool was_viable;

			float best_damage_per_hitbox = 0;
			float distance_to_center = 0;
			penetration::PenetrationInput_t in;

			in.m_damage = dmg;
			in.m_damage_pen = pendmg;
			in.m_can_pen = pen;
			in.m_target = player;
			in.m_from = ctx.m_local();
			in.m_pos = point;

			penetration::PenetrationOutput_t out;
			// we can hit p!

			if (penetration::run(&in, &out)) {
				/*if (float m_damage = feature::autowall->CanHit(ctx.m_eye_position, point, ctx.m_local(), player, it.m_index, &was_viable); m_damage > 0)
				{*/
				if (!was_viable)
				{
					if (!pen || out.m_damage < wall_dmg)
						continue;
				}
				else
				{
					if (out.m_damage < just_dmg)
						continue;
				}

				bool good_for_baim = false;

				if (it.m_mode == HitscanMode::PREFER)
					good_for_baim = true;

				else if (it.m_mode == HitscanMode::SAFETY && safepoints < 3)
					continue;
				else if (it.m_mode == HitscanMode::SAFETY_BETTER && safepoints < 2)
					continue;

				else if (it.m_mode == HitscanMode::LETHAL2 && out.m_damage * 2.f > player->m_iHealth())
				{
					good_for_baim = true;
				}
				else if (it.m_mode == HitscanMode::LETHAL && out.m_damage > player->m_iHealth())
				{
					good_for_baim = true;
				}

				if (it.m_mode == HitscanMode::NORMAL)
					good_for_baim = true;

				if (good_for_baim) {
					done = true;
				}
				if (out.m_damage > best_damage_per_hitbox || out.m_damage > scan.m_damage)
				{
					scan.m_damage = out.m_damage;
					scan.m_pos = point;
					scan.m_hitbox = it.m_index;
					scan.m_safepoint[it.m_index] = safepoints > 2;
					best_damage_per_hitbox = out.m_damage;
				}
				//}
			}
		}

		// ghetto break out of outer loop.
		if (done)
			break;
	}

	// we found something that we can damage.
	// set out vars.
	if (scan.m_damage > 0.f) {
		aim = scan.m_pos;
		damage = scan.m_damage;
		hitbox = scan.m_hitbox;
		if (scan.m_damage > 5.f && ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_quick_stop_options.at(0))
			ctx.do_autostop = true;
		return true;
	}
	else
		rdata->damage_ticks = 0;

	return false;
}

float GetFOV(const QAngle& view_angles, const Vector& start, const Vector& end) {
	Vector dir, fw;

	// get direction and normalize.
	dir = (end - start).Normalized();

	// get the forward direction vector of the view angles.
	Math::AngleVectors(view_angles, &fw);

	// get the angle between the view angles forward directional vector and the target location.
	return max(RAD2DEG(std::acos(fw.Dot(dir))), 0.f);
}

bool c_aimbot::SelectTarget(C_BasePlayer* player, C_Tickrecord* record, const Vector& aim, float damage) {
	float dist, fov, height;
	int   hp;

	// fov check.
	if (ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_maximum_fov > 0 && ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_maximum_fov < 180) {
		// if out of fov, retn false.
		if (GetFOV(ctx.cmd_original_angles, ctx.m_eye_position, aim) > ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_maximum_fov)
			return false;
	}

	if (ctx.m_last_shot_index == player->entindex() && damage > 0)
		return true;

	switch (ctx.m_settings.aimbot_target_selection) {

		// distance.
	case 0:
		dist = (record->origin - ctx.m_eye_position).Length();

		if (dist < m_best_dist) {
			m_best_dist = dist;
			return true;
		}

		break;
	case 1:
		dist = (record->origin - ctx.m_eye_position).Length();

		if (dist < m_best_dist) {
			m_best_dist = dist;
			return true;
		}

		break;
		// crosshair.
	case 2:
		fov = Math::GetFov(Engine::Movement::Instance()->m_qRealAngles, Math::CalcAngle(ctx.m_eye_position, aim));

		if (fov < m_best_fov) {
			m_best_fov = fov;
			return true;
		}

		break;

		// damage.
	case 3:
		if (damage > m_best_damage) {
			m_best_damage = damage;
			return true;
		}

		break;

		// lowest ping.
	case 4:
		fov = Math::GetFov(Engine::Movement::Instance()->m_qRealAngles, Math::CalcAngle(ctx.m_eye_position, aim));

		if (fov < m_best_fov) {
			m_best_fov = fov;
			return true;
		}

		break;

		// best k/d.
	case 5:
		fov = Math::GetFov(Engine::Movement::Instance()->m_qRealAngles, Math::CalcAngle(ctx.m_eye_position, aim));

		if (fov < m_best_fov) {
			m_best_fov = fov;
			return true;
		}

		break;

		// best hit chance.
	case 6:
		if (m_best_hc[0] < m_best_hit_chance) {
			m_best_hit_chance = m_best_hc[0];
			return true;
		}

		break;

	default:
		return false;
	}

	return false;
}
void c_aimbot::apply(CUserCmd* cmd, bool* send_packet)
{
	bool attack;

	// attack states.
	attack = cmd->buttons & IN_ATTACK;

	if (attack) {
		if (!m_weapon()->can_shoot())
			return;

		if (m_target)
		{
			auto v89 = csgo.m_client_state()->m_iChockedCommands;

			if (!ctx.fakeducking)
			{
				*send_packet = true;
			}

			c_player_records* log = &feature::lagcomp->records[m_target->entindex() - 1];
			auto r_log = &feature::resolver->player_records[m_target->entindex() - 1];

			// make sure to aim at un-interpolated data.
			// do this so BacktrackEntity selects the exact record.
			if (log->best_record && !log->best_record->exploit && !log->best_record->lc_exploit)
				cmd->tick_count = TIME_TO_TICKS(log->best_record->simulation_time + ctx.lerp_time);

			// set angles to target.
			cmd->viewangles = m_angle;

			ctx.shots_fired[m_target->entindex() - 1]++;

			r_log->last_shot_missed = false;
			ctx.autopeek_back = true;

			// if not silent aim, apply the viewangles.
			if (!ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_silent_aim)
				csgo.m_engine()->SetViewAngles(m_angle);

			const auto did_shot = feature::resolver->add_shot(ctx.m_eye_position, m_aim, log->best_record, feature::misc->hitbox_to_hitgroup(m_hitbox), m_damage, m_target->entindex());


			//if (did_shot && ctx.m_settings.aimbot_weapon[ctx.weapon].aimbot_log_misses_due_to_spread) {

			//	char text[255];
			//	auto new_nick = m_target->m_szNickName(); new_nick.resize(31);

			//	if (log->best_record) {
			//		sprintf_s(text, sxor("shot at %s in the %s (aimbot damage: %.0f with %.0f hitchance at %dt backtrack [side: %s | %d | delta: %.2f | eye: %.5f | positive: %.5f | negative: %.5f | tol right: %.2f | tol left: %.2f]\0"),
			//			new_nick.c_str(),
			//			hitbox_to_string(m_hitbox).c_str(),
			//			m_damage,
			//			m_best_hc[0],
			//			max(0, (csgo.m_globals()->tickcount - TIME_TO_TICKS(log->best_record->simulation_time))),
			//			side_so_string(log->best_record->resolver_index).c_str(),
			//			log->best_record->resolver_type,
			//			log->best_record->resolver_delta_multiplier,
			//			/*log->best_record->m_LayerData[0].m_flPlaybackRate * 1000.f,
			//			log->best_record->m_LayerData[1].m_flPlaybackRate * 1000.f,
			//			log->best_record->m_LayerData[2].m_flPlaybackRate * 1000.f*/
			//			log->best_record->m_LayerData[0].m_flPlaybackRate * 1000.f,
			//			log->best_record->m_LayerData[1].m_flPlaybackRate * 1000.f,
			//			log->best_record->m_LayerData[2].m_flPlaybackRate * 1000.f,
			//			log->best_record->m_LayerData[0].ToleranceLeft * 1000.f,
			//			log->best_record->m_LayerData[0].ToleranceRight * 1000.f
			//		);
			//	}
			//	_events.emplace_back(text);
			//}

			// set that we fired.
			ctx.m_ragebot_shot_nr = cmd->command_number;
			ctx.m_last_shot_index = m_target->entindex();
			ctx.m_last_shot_time = csgo.m_globals()->curtime;
		}
	}
}