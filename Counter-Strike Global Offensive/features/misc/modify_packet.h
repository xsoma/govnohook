#pragma once

#include "source.hpp"
#include <props/entity.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <hooks/hooked.hpp>
#include <sdk/math/math.hpp>
#include <props/displacement.hpp>
#include <algorithm>
#include <iostream>
#include "usercmd.hpp"


class CPacketModify
{
public:
	CUserCmd* __thiscall GetModifablePacket(CPacketManager* manage);
	void __thiscall Update(BYTE* gap, int* pint, int m_nSequence);
	bool __thiscall Init(CPacketManager* m_PacketMgr, int m_nSequence);
private:

};

extern CPacketModify* modify_packet;