#pragma once

#include <props/entity.hpp>

enum CSWeaponType
{
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_EQUIPMENT,
	WEAPONTYPE_STACKABLEITEM,
	WEAPONTYPE_UNKNOWN
};

enum ItemDefinitionIndex
{
	WEAPON_NONE,
	WEAPON_DEAGLE = 1,
	WEAPON_ELITE = 2,
	WEAPON_FIVESEVEN = 3,
	WEAPON_GLOCK = 4,
	WEAPON_AK47 = 7,
	WEAPON_AUG = 8,
	WEAPON_AWP = 9,
	WEAPON_FAMAS = 10,
	WEAPON_G3SG1 = 11,
	WEAPON_GALILAR = 13,
	WEAPON_M249 = 14,
	WEAPON_M4A1 = 16,
	WEAPON_MAC10 = 17,
	WEAPON_P90 = 19,
	WEAPON_MP5SD = 23,
	WEAPON_UMP45 = 24,
	WEAPON_XM1014 = 25,
	WEAPON_BIZON = 26,
	WEAPON_MAG7 = 27,
	WEAPON_NEGEV = 28,
	WEAPON_SAWEDOFF = 29,
	WEAPON_TEC9 = 30,
	WEAPON_TASER = 31,
	WEAPON_HKP2000 = 32,
	WEAPON_MP7 = 33,
	WEAPON_MP9 = 34,
	WEAPON_NOVA = 35,
	WEAPON_P250 = 36,
	WEAPON_SHIELD = 37,
	WEAPON_SCAR20 = 38,
	WEAPON_SG556 = 39,
	WEAPON_SSG08 = 40,
	WEAPON_KNIFEGG = 41,
	WEAPON_KNIFE = 42,
	WEAPON_FLASHBANG = 43,
	WEAPON_HEGRENADE = 44,
	WEAPON_SMOKEGRENADE = 45,
	WEAPON_MOLOTOV = 46,
	WEAPON_DECOY = 47,
	WEAPON_INCGRENADE = 48,
	WEAPON_C4 = 49,
	WEAPON_HEALTHSHOT = 57,
	WEAPON_KNIFE_T = 59,
	WEAPON_M4A1_SILENCER = 60,
	WEAPON_USP_SILENCER = 61,
	WEAPON_CZ75A = 63,
	WEAPON_REVOLVER = 64,
	WEAPON_TAGRENADE = 68,
	WEAPON_FISTS = 69,
	WEAPON_BREACHCHARGE = 70,
	WEAPON_TABLET = 72,
	WEAPON_MELEE = 74,
	WEAPON_AXE = 75,
	WEAPON_HAMMER = 76,
	WEAPON_SPANNER = 78,
	WEAPON_KNIFE_GHOST = 80,
	WEAPON_FIREBOMB = 81,
	WEAPON_DIVERSION = 82,
	WEAPON_FRAG_GRENADE = 83,
	WEAPON_SNOWBALL = 84,
	WEAPON_BUMPMINE = 85,
	WEAPON_BAYONET = 500,
	WEAPON_KNIFE_CSS = 503,
	WEAPON_KNIFE_FLIP = 505,
	WEAPON_KNIFE_GUT = 506,
	WEAPON_KNIFE_KARAMBIT = 507,
	WEAPON_KNIFE_M9_BAYONET = 508,
	WEAPON_KNIFE_TACTICAL = 509,
	WEAPON_KNIFE_FALCHION = 512,
	WEAPON_KNIFE_SURVIVAL_BOWIE = 514,
	WEAPON_KNIFE_BUTTERFLY = 515,
	WEAPON_KNIFE_PUSH = 516,
	WEAPON_KNIFE_CORD = 517,
	WEAPON_KNIFE_CANIS = 518,
	WEAPON_KNIFE_URSUS = 519,
	WEAPON_KNIFE_GYPSY_JACKKNIFE = 520,
	WEAPON_KNIFE_OUTDOOR = 521,
	WEAPON_KNIFE_STILETTO = 522,
	WEAPON_KNIFE_WIDOWMAKER = 523,
	WEAPON_KNIFE_SKELETON = 525,
	WEAPON_KNIFE_BAYONET = 500,
	GLOVE_STUDDED_BLOODHOUND = 5027,
	GLOVE_T_SIDE = 5028,
	GLOVE_CT_SIDE = 5029,
	GLOVE_SPORTY = 5030,
	GLOVE_SLICK = 5031,
	GLOVE_LEATHER_WRAP = 5032,
	GLOVE_MOTORCYCLE = 5033,
	GLOVE_SPECIALIST = 5034,
	GLOVE_HYDRA = 5035,
	GLOVE_BROKENFRAG = 4725,
	AGENT_GROUND_REBEL = 5105,
	AGENT_Osiris = 5106,
	AGENT_Prof_Shahmat = 5107,
	AGENT_The_Elite_Mr_Muhlik = 5108,
	AGENT_Soldier_Phoenix = 5205,
	AGENT_Enforcer_Phoenix = 5206,
	AGENT_Slingshot = 5207,
	AGENT_Street_Soldier = 5208,
	AGENT_Operator_FBI = 5305,
	AGENT_Markus_Delrow = 5306,
	AGENT_Michael_Syfers = 5307,
	AGENT_Special_Agent_Ava = 5308,
	AGENT_3rd_Commando_Company = 5400,
	AGENT_Seal_Team_6_Soldier = 5401,
	AGENT_BuckShot = 5402,
	AGENT_Two_Times_McCoy_USAF = 5403,
	AGENT_Lt_Commander_Ricksaw = 5404,
	AGENT_Blueberries_Buckshot = 4619,
	AGENT_Two_Times_McCoy_TACP = 4680,
	AGENT_Dragomir = 5500,
	AGENT_Maximus = 5501,
	AGENT_Rezan_The_Ready = 5502,
	AGENT_Blackwolf = 5503,
	AGENT_The_Doctor_Romanov = 5504,
	AGENT_Rezan_the_Redshirt = 4718,
	AGENT_Dragomir_Sabre = 5505,
	AGENT_B_Squadron_Officer = 5601,
	AGENT_Cmdr_Mae_Dead_Cold_Jamison = 4711,
	AGENT_1st_Lieutenant_Farlow = 4712,
	AGENT_John_Van_Healen_Kask = 4713,
	AGENT_Bio_Haz_Specialist = 4714,
	AGENT_Sergeant_Bombson = 4715,
	AGENT_Chem_Haz_Specialist = 4716,
	AGENT_Sir_Bloody_Miami_Darryl = 4726,
	AGENT_Sir_Bloody_Silent_Darryl = 4733,
	AGENT_Sir_Bloody_Skullhead_Darryl = 4734,
	AGENT_Sir_Bloody_Darryl_Royale = 4735,
	AGENT_Sir_Bloody_Loudmouth_Darryl = 4736,
	AGENT_SafeCracker_Voltzmann = 4727,
	AGENT_Little_Kev = 4728,
	AGENT_Number_K = 4732,
	AGENT_Getaway_Sally = 4730,

};

class C_BaseCombatWeapon : public C_BaseEntity
{
public:
	float& m_flNextPrimaryAttack();
	float& m_flNextSecondaryAttack();
	CBaseHandle& m_hOwner();
	int& m_iClip1();
	int& m_iPrimaryReserveAmmoCount();
	short& m_iItemDefinitionIndex();
	int& m_iItemIDHigh();
	float& m_flMaxspeed();
	float& m_flThrowStrength();
	C_BasePlayer* m_hThrower();
	int& m_nFallbackPaintKit();
	int& m_nFallbackStatTrak();
	bool IsGrenade();
	bool IsShotgun();
	int WeaponGroup();
	bool IsGun();
	std::string get_icon();
	bool is_default_knife();
	void post_data_update(int updateType);
	void on_data_changed(int updateType);
	void pre_data_update(int updateType);
	void SetDestroyedOnRecreateEntities();
	void set_abs_origin(Vector origin);
	void Release();
	bool is_knife();
	float& m_flFallbackWear();
	float& m_nFallbackSeed();

	int& m_nSequence();
	float& m_flCycle();
	void invalidate_anims(int m);
	float& m_flAccuracyPenalty();
	int& m_iAccountID();
	int& m_OriginalOwnerXuidLow();
	int& m_OriginalOwnerXuidHigh();
	int& m_iEntityQuality();

	int& m_nModelIndex();
	int& m_iViewModelIndex();
	int& m_iWorldModelIndex();
	char* m_szCustomName();
	CBaseHandle& m_hWeaponWorldModel();
	void set_model_index(int index);

	__forceinline weapon_info* GetWpnData()
	{
		return Memory::VCall< weapon_info* (__thiscall*)(void*) >(this, 460)(this);
	}
};

class C_WeaponCSBaseGun : public C_BaseCombatWeapon
{
public:
	float& m_flRecoilIndex();

	int& m_zoomLevel();

	float GetSpread();
	float GetInaccuracy();
	float GetMaxSpeed();
	weapon_info* GetCSWeaponData();
	void UpdateAccuracyPenalty();

	bool is_weapon();

	CSWeaponType GetWeaponType();

	bool can_fire_bullet();

	Vector CalculateSpread(int seed, float inaccuracy, float spread, bool revolver2 = false);
	weapon_info* GetWpnData();
	bool IsFireTime();
	int& m_weaponMode();
	int& m_reloadState();
	bool& m_reload();
	bool can_shoot();
	bool can_exploit(int tickbase_shift);
	bool can_cock();
	//int& m_iShotsFired();
	float& m_flLastShotTime();
	float& m_fAccuracyPenalty();
	bool IsSniper();
	float& m_flPostponeFireReadyTime();
	int& m_Activity();
	bool IsSecondaryFireTime();
	bool& m_bPinPulled();
	float& m_fThrowTime();
	bool IsBeingThrowed();
};