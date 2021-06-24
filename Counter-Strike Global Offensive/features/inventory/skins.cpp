#include "skins.h"
#include <props/weapon.hpp>
#include <codecvt>
#include <cassert>
#include <locale>
#include "inventory/inventorychanger.h"
#include "inventory/items.h"
#include <Psapi.h>
#include <hooks/hooked.hpp>
Skins g_skins{ };;
CreateClientClassFn GetWearableCreateFn()
{
	auto clazz = csgo.m_client()->GetAllClasses();
	// Please, if you gonna paste it into a cheat use classids here. I use names because they
	// won't change in the foreseeable future and i dont need high speed, but chances are
	// you already have classids, so use them instead, they are faster.
	while (strcmp(clazz->m_pNetworkName, "CEconWearable"))
		clazz = clazz->m_pNext;
	return clazz->m_pCreateFn;
}
const char* models_to_change[] = {
("models/player/custom_player/legacy/tm_phoenix.mdl"),
("models/player/custom_player/legacy/ctm_sas.mdl"),
("models/player/custom_player/legacy/tm_balkan_variantj.mdl"),
("models/player/custom_player/legacy/tm_balkan_variantg.mdl"),
("models/player/custom_player/legacy/tm_balkan_varianti.mdl"),
("models/player/custom_player/legacy/tm_balkan_variantf.mdl"),
("models/player/custom_player/legacy/ctm_st6_varianti.mdl"),
("models/player/custom_player/legacy/ctm_st6_variantm.mdl"),
("models/player/custom_player/legacy/ctm_st6_variantg.mdl"),
("models/player/custom_player/legacy/ctm_st6_variante.mdl"),
("models/player/custom_player/legacy/ctm_st6_variantk.mdl"),
("models/player/custom_player/legacy/tm_balkan_varianth.mdl"),
("models/player/custom_player/legacy/ctm_fbi_varianth.mdl"),
("models/player/custom_player/legacy/ctm_fbi_variantg.mdl"),
("models/player/custom_player/legacy/ctm_fbi_variantf.mdl"),
("models/player/custom_player/legacy/tm_phoenix_variantg.mdl"),
("models/player/custom_player/legacy/tm_phoenix_variantf.mdl"),
("models/player/custom_player/legacy/tm_phoenix_varianth.mdl"),
("models/player/custom_player/legacy/tm_leet_variantf.mdl"),
("models/player/custom_player/legacy/tm_leet_varianti.mdl"),
("models/player/custom_player/legacy/tm_leet_varianth.mdl"),
("models/player/custom_player/legacy/tm_leet_variantg.mdl"),
("models/player/custom_player/legacy/ctm_fbi_variantb.mdl"),
("models/player/custom_player/legacy/ctm_sas_variantf.mdl"),
("models/player/custom_player/legacy/tm_anarchist.mdl"),
("models/player/custom_player/legacy/tm_anarchist_varianta.mdl"),
("models/player/custom_player/legacy/tm_anarchist_variantb.mdl"),
("models/player/custom_player/legacy/tm_anarchist_variantc.mdl"),
("models/player/custom_player/legacy/tm_anarchist_variantd.mdl"),
("models/player/custom_player/legacy/tm_pirate.mdl"),
("models/player/custom_player/legacy/tm_pirate_varianta.mdl"),
("models/player/custom_player/legacy/tm_pirate_variantb.mdl"),
("models/player/custom_player/legacy/tm_pirate_variantc.mdl"),
("models/player/custom_player/legacy/tm_pirate_variantd.mdl"),
("models/player/custom_player/legacy/tm_professional.mdl"),
("models/player/custom_player/legacy/tm_professional_var1.mdl"),
("models/player/custom_player/legacy/tm_professional_var2.mdl"),
("models/player/custom_player/legacy/tm_professional_var3.mdl"),
("models/player/custom_player/legacy/tm_professional_var4.mdl"),
("models/player/custom_player/legacy/tm_separatist.mdl"),
("models/player/custom_player/legacy/tm_separatist_varianta.mdl"),
("models/player/custom_player/legacy/tm_separatist_variantb.mdl"),
("models/player/custom_player/legacy/tm_separatist_variantc.mdl"),
("models/player/custom_player/legacy/tm_separatist_variantd.mdl"),
("models/player/custom_player/legacy/ctm_gign.mdl"),
("models/player/custom_player/legacy/ctm_gign_varianta.mdl"),
("models/player/custom_player/legacy/ctm_gign_variantb.mdl"),
("models/player/custom_player/legacy/ctm_gign_variantc.mdl"),
("models/player/custom_player/legacy/ctm_gign_variantd.mdl"),
("models/player/custom_player/legacy/ctm_gsg9.mdl"),
("models/player/custom_player/legacy/ctm_gsg9_varianta.mdl"),
("models/player/custom_player/legacy/ctm_gsg9_variantb.mdl"),
("models/player/custom_player/legacy/ctm_gsg9_variantc.mdl"),
("models/player/custom_player/legacy/ctm_gsg9_variantd.mdl"),
("models/player/custom_player/legacy/ctm_idf.mdl"),
("models/player/custom_player/legacy/ctm_idf_variantb.mdl"),
("models/player/custom_player/legacy/ctm_idf_variantc.mdl"),
("models/player/custom_player/legacy/ctm_idf_variantd.mdl"),
("models/player/custom_player/legacy/ctm_idf_variante.mdl"),
("models/player/custom_player/legacy/ctm_idf_variantf.mdl"),
("models/player/custom_player/legacy/ctm_swat.mdl"),
("models/player/custom_player/legacy/ctm_swat_varianta.mdl"),
("models/player/custom_player/legacy/ctm_swat_variantb.mdl"),
("models/player/custom_player/legacy/ctm_swat_variantc.mdl"),
("models/player/custom_player/legacy/ctm_swat_variantd.mdl"),
("models/player/custom_player/legacy/ctm_sas_varianta.mdl"),
("models/player/custom_player/legacy/ctm_sas_variantb.mdl"),
("models/player/custom_player/legacy/ctm_sas_variantc.mdl"),
("models/player/custom_player/legacy/ctm_sas_variantd.mdl"),
("models/player/custom_player/legacy/ctm_st6.mdl"),
("models/player/custom_player/legacy/ctm_st6_varianta.mdl"),
("models/player/custom_player/legacy/ctm_st6_variantb.mdl"),
("models/player/custom_player/legacy/ctm_st6_variantc.mdl"),
("models/player/custom_player/legacy/ctm_st6_variantd.mdl"),
("models/player/custom_player/legacy/tm_balkan_variante.mdl"),
("models/player/custom_player/legacy/tm_balkan_varianta.mdl"),
("models/player/custom_player/legacy/tm_balkan_variantb.mdl"),
("models/player/custom_player/legacy/tm_balkan_variantc.mdl"),
("models/player/custom_player/legacy/tm_balkan_variantd.mdl"),
("models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl"),
("models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl"),
("models/player/custom_player/legacy/tm_jumpsuit_variantc.mdl"),
("models/player/custom_player/legacy/tm_phoenix_heavy.mdl"),
("models/player/custom_player/legacy/ctm_heavy.mdl"),
("models/player/custom_player/legacy/tm_leet_varianta.mdl"),
("models/player/custom_player/legacy/tm_leet_variantb.mdl"),
("models/player/custom_player/legacy/tm_leet_variantc.mdl"),
("models/player/custom_player/legacy/tm_leet_variantd.mdl"),
("models/player/custom_player/legacy/tm_leet_variante.mdl"),
("models/player/custom_player/legacy/tm_phoenix.mdl"),
("models/player/custom_player/legacy/tm_phoenix_varianta.mdl"),
("models/player/custom_player/legacy/tm_phoenix_variantb.mdl"),
("models/player/custom_player/legacy/tm_phoenix_variantc.mdl"),
("models/player/custom_player/legacy/tm_phoenix_variantd.mdl"),
("models/player/custom_player/legacy/ctm_fbi.mdl"),
("models/player/custom_player/legacy/ctm_fbi_varianta.mdl"),
("models/player/custom_player/legacy/ctm_fbi_variantc.mdl"),
("models/player/custom_player/legacy/ctm_fbi_variantd.mdl"),
("models/player/custom_player/legacy/ctm_fbi_variante.mdl"),
("models/player/custom_player/legacy/ctm_sas.mdl")
};

#define INRANGE(x, a, b) (x >= a && x <= b)  //-V1003
#define GETBITS(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0)) //-V1003
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))

uint64_t FindSignature(const char* szModule, const char* szSignature)
{
	MODULEINFO modInfo;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(szModule), &modInfo, sizeof(MODULEINFO));

	uintptr_t startAddress = (DWORD)modInfo.lpBaseOfDll; //-V101 //-V220
	uintptr_t endAddress = startAddress + modInfo.SizeOfImage;

	const char* pat = szSignature;
	uintptr_t firstMatch = 0;

	for (auto pCur = startAddress; pCur < endAddress; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GETBYTE(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;
			else
				pat += 2;
		}
		else
		{
			pat = szSignature;
			firstMatch = 0;
		}
	}

	return 0;
}
static auto fnEquip
= reinterpret_cast<int(__thiscall*)(void*, void*)>(
	FindSignature("client.dll", "55 8B EC 83 EC 10 53 8B 5D 08 57 8B F9")
	);

static auto fnInitializeAttributes
= reinterpret_cast<int(__thiscall*)(void*)>(
	FindSignature("client.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 56 8B F1 8B 86")
	);
static auto zis_knife(const int i) -> bool
{
	return (i >= 500 && i < 550) || i == 59 || i == 42;
}
static auto zis_glove(const int i) -> bool
{
	return (i >= 5027 && i <= 5035 || i == 4725); //hehe boys
}

enum class EStickerAttributeType
{
	Index,
	Wear,
	Scale,
	Rotation
};

enum DataUpdateType_t
{
	DATA_UPDATE_CREATED = 0,
	//	DATA_UPDATE_ENTERED_PVS,
	DATA_UPDATE_DATATABLE_CHANGED
	//	DATA_UPDATE_LEFT_PVS,
	//DATA_UPDATE_DESTROYED,
};
static uint16_t s_iwoff = 0;

static void* o_uint_fn;

static unsigned int __fastcall hooked_uint_fn(void* thisptr, void*, int slot, EStickerAttributeType attribute, unsigned fl)
{
	C_WeaponCSBaseGun* item = reinterpret_cast<C_WeaponCSBaseGun*>(uintptr_t(thisptr) - s_iwoff);

	for (auto& wweap : g_InventorySkins)
	{
		if ((!wweap.second.in_use_ct && !wweap.second.in_use_t) && item->m_iItemDefinitionIndex() != WEAPON_KNIFE && item->m_iItemDefinitionIndex() != WEAPON_KNIFE_T)
			continue;

		if (wweap.second.wId != item->m_iItemDefinitionIndex())
			continue;

		if (!wweap.second.in_use_t && ctx.m_local()->m_iTeamNum() == 2)
			continue;

		if (!wweap.second.in_use_ct && ctx.m_local()->m_iTeamNum() == 3)
			continue;


		if (attribute == EStickerAttributeType::Index)
		{
			switch (slot)
			{
			case 0:
				return wweap.second.sicker[0];
				break;
			case 1:
				return wweap.second.sicker[1];
				break;
			case 2:
				return wweap.second.sicker[2];
				break;
			case 3:
				return wweap.second.sicker[3];
				break;
			default:
				break;
			}
		}
	}
	return reinterpret_cast<decltype(hooked_uint_fn)*>(o_uint_fn)(thisptr, nullptr, slot, attribute, fl);
}

void ApplyStickerHooks(C_WeaponCSBaseGun* item)
{
	if (!s_iwoff)
		s_iwoff = 0x00002DC0 + 0xC; //m_Item

	void**& iw_vt = *reinterpret_cast<void***>(uintptr_t(item) + s_iwoff);

	static void** iw_hook_vt = nullptr;

	if (!iw_hook_vt)
	{
		size_t len = 0;
		for (; iw_vt[len]; ++len);
		iw_hook_vt = new void* [len];

		memcpy(iw_hook_vt, iw_vt, len * sizeof(void*));

		o_uint_fn = iw_hook_vt[5];
		iw_hook_vt[5] = &hooked_uint_fn;
	}

	iw_vt = iw_hook_vt;
}

bool Skins::ApplyCustomSkin(C_BasePlayer* localPlayer, C_WeaponCSBaseGun* pWeapon, short nWeaponIndex)
{
	switch (nWeaponIndex)
	{
	case WEAPON_KNIFE_BAYONET:
	case WEAPON_KNIFE_BUTTERFLY:
	case WEAPON_KNIFE_FALCHION:
	case WEAPON_KNIFE_FLIP:
	case WEAPON_KNIFE_GUT:
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
	case WEAPON_KNIFE_KARAMBIT:
	case WEAPON_KNIFE_M9_BAYONET:
	case WEAPON_KNIFE_PUSH:
	case WEAPON_KNIFE_STILETTO:
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
	case WEAPON_KNIFE_TACTICAL:
	case WEAPON_KNIFE_URSUS:
	case WEAPON_KNIFE_WIDOWMAKER:
	case WEAPON_KNIFE_SKELETON:
	case WEAPON_KNIFE_OUTDOOR:
	case WEAPON_KNIFE_CANIS:
	case WEAPON_KNIFE_CORD:
	case WEAPON_KNIFE_CSS:
	case WEAPON_KNIFE_T:
	case WEAPON_KNIFE:
		nWeaponIndex = WEAPON_KNIFE;
	}


	for (auto& wweap : g_InventorySkins)
	{
		if ((!wweap.second.in_use_ct && !wweap.second.in_use_t))
			continue;

		if (wweap.second.wId != nWeaponIndex)
			continue;

		if (!wweap.second.in_use_t && localPlayer->m_iTeamNum() == 2)
			continue;

		if (!wweap.second.in_use_ct && localPlayer->m_iTeamNum() == 3)
			continue;



		if (wweap.second.sicker[0] || wweap.second.sicker[1] || wweap.second.sicker[2] || wweap.second.sicker[3])
			ApplyStickerHooks(pWeapon);


		pWeapon->m_nFallbackPaintKit() = wweap.second.paintKit;
		pWeapon->m_iEntityQuality() = wweap.second.quality;
		pWeapon->m_nFallbackSeed() = wweap.second.seed;
		pWeapon->m_nFallbackStatTrak() = wweap.second.stattrak;
#undef max
		pWeapon->m_flFallbackWear() = std::max(wweap.second.wear, 0.00001f);

		if (wweap.second.name.length())
			strcpy(pWeapon->m_szCustomName(), wweap.second.name.c_str());
		pWeapon->m_iItemIDHigh() = -1;


	}

	for (auto& wweap : g_InventorySkins)
	{
		if ((!wweap.second.in_use_ct && !wweap.second.in_use_t))
			continue;

		if (zis_knife(wweap.second.wId) && zis_knife(nWeaponIndex))
		{

			if (!wweap.second.in_use_t && localPlayer->m_iTeamNum() == 2)
				continue;

			if (!wweap.second.in_use_ct && localPlayer->m_iTeamNum() == 3)
				continue;


			pWeapon->m_nFallbackPaintKit() = wweap.second.paintKit;
			pWeapon->m_iEntityQuality() = wweap.second.quality;
			pWeapon->m_nFallbackSeed() = wweap.second.seed;
			pWeapon->m_nFallbackStatTrak() = wweap.second.stattrak;
			pWeapon->m_flFallbackWear() = std::max(wweap.second.wear, 0.00001f);

			if (wweap.second.name.length())
				strcpy(pWeapon->m_szCustomName(), wweap.second.name.c_str());


			pWeapon->m_iItemIDHigh() = -1;

			if (wweap.second.wId != WEAPON_NONE && zis_knife(wweap.second.wId)) {
				static auto old_definition_index = pWeapon->m_iItemDefinitionIndex();

				pWeapon->m_iItemDefinitionIndex() = wweap.second.wId;
				pWeapon->m_iEntityQuality() = 3;
				const auto& replacement_item = k_weapon_info.at(pWeapon->m_iItemDefinitionIndex());
				Hooked::skinfix(csgo.m_model_info()->GetModelIndex(replacement_item.model));
				pWeapon->set_model_index(csgo.m_model_info()->GetModelIndex(replacement_item.model));

				C_BasePlayer* worldmodel = csgo.m_entity_list()->GetClientEntityFromHandle(pWeapon->m_hWeaponWorldModel());
				if (!worldmodel) return false;

				if (worldmodel) {
					worldmodel->set_model_index(csgo.m_model_info()->GetModelIndex(replacement_item.model) + 1);
				}

				//iconfix(replacement_item.icon);
				//pWeapon->pre_data_update(DATA_UPDATE_CREATED);
			}
		}
	}
	return true;
}


void Skins::think()
{
	if (!csgo.m_engine()->IsInGame())
		return;
	if (!ctx.m_local())
		return;
	if (ctx.m_local()->m_iHealth() <= 0)
		return;

	const int iTeam = ctx.m_local()->m_iTeamNum();

	for (auto& wweap : g_InventorySkins)
	{
		if ((!wweap.second.in_use_ct && !wweap.second.in_use_t))
		{
			if (ctx.m_local() && !ctx.m_local()->IsDead())
				ctx.original_model_index = ctx.m_local()->m_nModelIndex();
			else
				ctx.original_model_index = -1;
			continue;
		}


		if (!wweap.second.in_use_t && ctx.m_local()->m_iTeamNum() == 2)
		{
			if (ctx.m_local() && !ctx.m_local()->IsDead())
				ctx.original_model_index = ctx.m_local()->m_nModelIndex();
			else
				ctx.original_model_index = -1;
			continue;
		}

		if (!wweap.second.in_use_ct && ctx.m_local()->m_iTeamNum() == 3)
		{
			if (ctx.m_local() && !ctx.m_local()->IsDead())
				ctx.original_model_index = ctx.m_local()->m_nModelIndex();
			else
				ctx.original_model_index = -1;
			continue;
		}


		if ((wweap.second.wId >= 5100 && wweap.second.wId <= 6000) || wweap.second.wId >= 4619 && wweap.second.wId <= 4800)
		{
			const auto& replacement_item = k_weapon_info.at(wweap.second.wId);
			if (wweap.second.in_use_t && ctx.m_local()->m_iTeamNum() == 2)
			{
				ctx.m_local()->set_model_index(csgo.m_model_info()->GetModelIndex(replacement_item.model));
			}
			if (wweap.second.in_use_ct && ctx.m_local()->m_iTeamNum() == 3)
			{
				ctx.m_local()->set_model_index(csgo.m_model_info()->GetModelIndex(replacement_item.model));
			}
		}
	}
	player_info player_info;
	if (!csgo.m_engine()->GetPlayerInfo(ctx.m_local()->entindex(), &player_info))
		return;

	const auto weapons = ctx.m_local()->m_hMyWeapons();
	if (!weapons)
		return;

	const auto wearables = ctx.m_local()->m_hMyWearables();
	if (!wearables)
		return;

	if (!csgo.m_entity_list()->GetClientEntityFromHandle(wearables[0]))
	{
		static auto gloveHandle = CBaseHandle(0);
		auto glove = reinterpret_cast<C_WeaponCSBaseGun*>(csgo.m_entity_list()->GetClientEntityFromHandle(wearables[0]));

		if (!glove)
		{
			const auto ourGlove = (C_WeaponCSBaseGun*)csgo.m_entity_list()->GetClientEntityFromHandle(gloveHandle);
			if (ourGlove)
			{
				wearables[0] = gloveHandle;
				glove = ourGlove;
			}
		}

		if (ctx.m_local()->IsDead())
		{
			if (glove)
			{
				glove->SetDestroyedOnRecreateEntities();
				glove->Release();
			}
			return;
		}


		for (auto& wglove : g_InventorySkins)
		{
			if (!zis_glove(wglove.second.wId))
				continue;

			if (!wglove.second.in_use_t && ctx.m_local()->m_iTeamNum() == 2)
				continue;

			if (!wglove.second.in_use_ct && ctx.m_local()->m_iTeamNum() == 3)
				continue;


			static int whatever_fix;
			if (!glove)
			{
				static auto create_wearable_fn = GetWearableCreateFn();
				const auto entry = csgo.m_entity_list()->GetHighestEntityIndex() + 1;
				const auto serial = rand() % 0x1000;

				create_wearable_fn(entry, serial);
				glove = reinterpret_cast<C_WeaponCSBaseGun*>(csgo.m_entity_list()->GetClientEntity(entry));
				whatever_fix = entry;
				glove->set_abs_origin({ 10000.f, 10000.f, 10000.f });
				const auto wearable_handle = reinterpret_cast<CBaseHandle*>(&wearables[0]);
				*wearable_handle = entry | serial << 16;
				gloveHandle = wearables[0];

			}

			if (glove)
			{
				glove->m_iItemDefinitionIndex() = wglove.second.wId;
				glove->m_iItemIDHigh() = -1;
				glove->m_iEntityQuality() = 4;
				glove->m_iAccountID() = player_info.steamID64;
				glove->m_nFallbackSeed() = wglove.second.seed;
				glove->m_nFallbackStatTrak() = -1;
				glove->m_flFallbackWear() = wglove.second.wear;
				glove->m_nFallbackPaintKit() = wglove.second.paintKit;
				const auto& replacement_item = k_weapon_info.at(glove->m_iItemDefinitionIndex());
				glove->set_model_index(csgo.m_model_info()->GetModelIndex(replacement_item.model) + 2 /*enjoy nopaste things*/);

				fnEquip(glove, ctx.m_local()); //follow entity, owner etc.
				ctx.m_local()->m_nBody() = 1; //remove default arms in 3th person mode

				g_pIClientLeafSystem->CreateRenderableHandle(glove); //render our glove in 3th person mode

				glove->pre_data_update(DataUpdateType_t::DATA_UPDATE_CREATED);
			}
		}
	}

	for (auto i = 0; weapons[i] != 0xFFFFFFFF; i++)
	{
		auto weapon = reinterpret_cast<C_WeaponCSBaseGun*>(csgo.m_entity_list()->GetClientEntityFromHandle(weapons[i]));
		if (!weapon)
			continue;

		const auto weaponIndex = weapon->m_iItemDefinitionIndex();
		ApplyCustomSkin(ctx.m_local(), weapon, weaponIndex);
		weapon->m_iAccountID() = player_info.steamID64;

	}
}