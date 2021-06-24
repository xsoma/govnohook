#include <props/entity.hpp>
#include <props/displacement.hpp>
#include <props/prop_manager.hpp>

void IHandleEntity::SetRefEHandle( const CBaseHandle& handle )
{
	using Fn = void ( __thiscall* )( void*, const CBaseHandle& );
	return Memory::VCall<Fn>( this, Index::IHandleEntity::SetRefEHandle )( this, handle );
}

const CBaseHandle& IHandleEntity::GetRefEHandle()
{
	using Fn = const CBaseHandle& ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IHandleEntity::GetRefEHandle )( this );
}

ICollideable* IClientUnknown::GetCollideable()
{
	static auto m_Collision = Engine::PropManager::Instance()->GetOffset("DT_BaseEntity", "m_Collision");

	return (ICollideable*)(uintptr_t(this) + m_Collision);
}

IClientNetworkable* IClientUnknown::GetClientNetworkable()
{
	//using Fn = IClientNetworkable* ( __thiscall* )( void* );
	//return Memory::VCall<Fn>( this, Index::IClientUnknown::GetClientNetworkable )( this );
	return (IClientNetworkable*)(uintptr_t(this) + 0x8);
}

IClientRenderable* IClientUnknown::GetClientRenderable()
{
	/*if (Source::m_pClientState->m_iDeltaTick == -1 || Source::m_pCvar->FindVar("mat_norendering")->GetInt())
		return nullptr;*/
		//using Fn = IClientRenderable* ( __thiscall* )( void* );
		//return Memory::VCall<Fn>( this, Index::IClientUnknown::GetClientRenderable )( this );
	return (IClientRenderable*)(uintptr_t(this) + 0x4);
}

IClientEntity* IClientUnknown::GetIClientEntity()
{
	using Fn = IClientEntity* ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientUnknown::GetIClientEntity )( this );
}

C_BaseEntity* IClientUnknown::GetBaseEntity()
{
	using Fn = C_BaseEntity* ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientUnknown::GetBaseEntity )( this );
}

Vector& ICollideable::OBBMins()
{
	using Fn = Vector& ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::ICollideable::OBBMins )( this );
}

Vector& ICollideable::OBBMaxs()
{
	using Fn = Vector& ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::ICollideable::OBBMaxs )( this );
}

solid_type ICollideable::GetSolidType()
{
	using Fn = solid_type(__thiscall*)(void*);
	return Memory::VCall<Fn>(this, Index::ICollideable::SolidType)(this);
}

//ClientClass* IClientNetworkable::GetClientClass()
//{
//	using Fn = ClientClass* ( __thiscall* )( void* );
//	return Memory::VCall<Fn>( this, Index::IClientNetworkable::GetClientClass )( this );
//}

bool IClientNetworkable::IsDormant()
{
	using Fn = bool ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::IClientNetworkable::IsDormant )( this );
}

int IClientNetworkable::entindex()
{
	using Fn = int(__thiscall*)(void*);

	if (this && *(void**)this != nullptr)
		return Memory::VCall<Fn>(this, Index::IClientNetworkable::entindex)(this);
	else
		return 0;
}

Vector& IClientEntity::OBBMins()
{
	return GetCollideable()->OBBMins();
}

Vector& IClientEntity::OBBMaxs()
{
	return GetCollideable()->OBBMaxs();
}

ClientClass* IClientEntity::GetClientClass()
{
	return GetClientNetworkable()->GetClientClass();
}

bool IClientEntity::IsDormant()
{
	if (this == nullptr || *(void**)this == nullptr)
		return true;

	//auto networkable = GetClientNetworkable();
	return *(bool*)(uintptr_t(this) + 0xED);//networkable->IsDormant();
}

int IClientEntity::entindex()
{
	//auto networkable = GetClientNetworkable();
	//return networkable->entindex();
	if (!this)
		return 0;

	return *(int*)(uintptr_t(this) + 0x64);
}

const model_t* IClientEntity::GetModel()
{
	//if (!GetClientRenderable())
	//	return nullptr;
	return GetClientRenderable()->GetModel();
}

void IClientEntity::GetWorldSpaceCenter(Vector& wSpaceCenter) {
	void* cRender = (void*)(this + 0x4);
	typedef void(__thiscall * fn)(void*, Vector&, Vector&);
	Vector va, vb;
	Memory::VCall<fn>(cRender, 17)(cRender, va, vb); // GetRenderBounds : 17
	wSpaceCenter.z += (va.z + vb.z) * 0.5f;
}

bool IClientEntity::is_breakable()
{
	static auto is_breakable_fn = reinterpret_cast<bool(__thiscall*)(IClientEntity*)>(Engine::Displacement::Signatures[c_signatures::BREAKABLE]);

	if (!this || !GetCollideable() || !GetClientClass())
		return false;

	auto client_class = GetClientClass();

	if (this->entindex() > 0) {
		if (client_class)
		{
			auto v3 = (int)client_class->m_pNetworkName;
			if (*(DWORD*)v3 == 0x65724243)
			{
				if (*(DWORD*)(v3 + 7) == 0x53656C62)
					return 1;
			}
			if (*(DWORD*)v3 == 0x73614243)
			{
				if (*(DWORD*)(v3 + 7) == 0x79746974)
					return 1;
			}
			if (client_class
				&& *reinterpret_cast<std::uint32_t*>(client_class->m_pNetworkName) == 'erBC'
				&& *reinterpret_cast<std::uint32_t*>(client_class->m_pNetworkName + 7u) == 'Selb')
				return true;

			return is_breakable_fn(this);
		}

		return is_breakable_fn(this);
	}
	return 0;
}

bool IClientEntity::SetupBones( matrix3x4a_t* pBoneToWorld, int nMaxBones, int boneMask, float currentTime )
{
	auto renderable = GetClientRenderable();
	return renderable->SetupBones( pBoneToWorld, nMaxBones, boneMask, currentTime );
}

bool C_BaseEntity::IsPlayer()
{
	using Fn = bool ( __thiscall* )( void* );
	return Memory::VCall<Fn>( this, Index::C_BaseEntity::IsPlayer )( this );
}

unsigned char& C_BaseEntity::m_MoveType()
{
	return *(unsigned char* )(uintptr_t(this) + Engine::Displacement::C_BaseEntity::m_MoveType );
}

unsigned char& C_BaseEntity::m_MoveCollide()
{
	return *(unsigned char* )(uintptr_t(this) + Engine::Displacement::C_BaseEntity::m_MoveType + 1 );
}

matrix3x4_t& C_BaseEntity::m_rgflCoordinateFrame()
{
	return *( matrix3x4_t* )(uintptr_t(this) + Engine::Displacement::C_BaseEntity::m_rgflCoordinateFrame );
}

int& C_BaseEntity::m_iTeamNum()
{
	return *( int* )(uintptr_t(this) + Engine::Displacement::DT_BaseEntity::m_iTeamNum );
}

Vector& C_BaseEntity::m_vecOrigin()
{
	return *( Vector* )(uintptr_t(this) + Engine::Displacement::DT_BaseEntity::m_vecOrigin );
}

float& C_BaseEntity::get_creation_time()
{
	return *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(this) + 0x29F4);
}

void C_BaseEntity::SetPredictionRandomSeed( CUserCmd* cmd )
{
	*( int* )(Engine::Displacement::Signatures[c_signatures::PREDRANDOMSEED]) = cmd ? cmd->random_seed : -1;
}

void C_BaseEntity::SetPredictionPlayer( C_BasePlayer* player )
{
	*( C_BasePlayer** )(Engine::Displacement::Signatures[c_signatures::PREDPLAYER]) = player;
}

CBaseHandle& C_BaseCombatCharacter::m_hActiveWeapon()
{
	return *( CBaseHandle* )(uintptr_t(this) + Engine::Displacement::DT_BaseCombatCharacter::m_hActiveWeapon );
}

int IClientEntity::GetPropInt(std::string& table, std::string& var)
{
	static auto offset = Engine::PropManager::Instance()->GetOffset(table.c_str(), var.c_str());
	int val = *(int*)(uintptr_t(this) + (int)offset);
	return val;
}
float IClientEntity::GetPropFloat(std::string& table, std::string& var)
{
	static auto offset = Engine::PropManager::Instance()->GetOffset(table.c_str(), var.c_str());
	float val = *(float*)(uintptr_t(this) + (int)offset);
	return val;
}
bool IClientEntity::GetPropBool(std::string& table, std::string& var)
{
	static auto offset = Engine::PropManager::Instance()->GetOffset(table.c_str(), var.c_str());
	bool val = *(bool*)(uintptr_t(this) + (int)offset);
	return val;
}
std::string IClientEntity::GetPropString(std::string& table, std::string& var)
{
	static auto offset = Engine::PropManager::Instance()->GetOffset(table.c_str(), var.c_str());
	char* val = (char*)(uintptr_t(this) + (int)offset);
	return std::string(val);
}

void IClientEntity::SetPropInt(std::string& table, std::string& var, int val)
{
	*reinterpret_cast<int*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(table.c_str(), var.c_str())) = val;
}
void IClientEntity::SetPropFloat(std::string& table, std::string& var, float val)
{
	*reinterpret_cast<float*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(table.c_str(), var.c_str())) = val;
}
void IClientEntity::SetPropBool(std::string& table, std::string& var, bool val)
{
	*reinterpret_cast<float*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(table.c_str(), var.c_str())) = val;
}
void IClientEntity::SetPropString(std::string& table, std::string& var, std::string val)
{
	*reinterpret_cast<std::string*>(uintptr_t(this) + (int)Engine::PropManager::Instance()->GetOffset(table.c_str(), var.c_str())) = val;
}