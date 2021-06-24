#include "modify_packet.h"
CPacketModify* modify_packet;
CUserCmd* __thiscall CPacketModify::GetModifablePacket(CPacketManager* manage)
{
    CUserCmd* result; // eax

    result = &manage->m_aCommands[88 * (manage->m_nSequence % 150) + 4];
    if (!result || !result->command_number)
        result = 0;

    return result;
}

void __thiscall CPacketModify::Update(BYTE* gap, int* pint, int m_nSequence)
{
    auto v3 = gap;
    auto v4 = *gap;
    auto v6 = (*(gap + 1) - v4) >> 2;
    if (v6 == 0x3FFFFFFF)
        return;

    auto v7 = v6 + 1;
    auto v19 = v6 + 1;
    auto v8 = (*(gap + 2) - v4) >> 2;
    auto v9 = ((v3[2] - v4) >> 2) >> 1;
    auto v10 = 0;
    auto v20 = v10;
    if (v8 <= 0x3FFFFFFF - v9)
    {
        v10 = v9 + v8;
        if (v9 + v8 < v7)
            v10 = v7;
        v20 = v10;
    }
    else
    {
        v10 = v6 + 1;
        v20 = v7;
    }
    auto v11 = 4 * v10;
    auto v13 = 0;
    auto v5 = 0;
    auto v21 = 0;
    int v12 = 0;
    auto v22 = 0;
    v13 = 0;
LABEL_15:
    v22 = v13;
LABEL_16:
    auto v24 = 0;
    auto v14 = v13 + 4 * v5;
    v21 = v14;
    v14 = m_nSequence;
    auto v15 = v3[1];
    auto v16 = *v3;

    std::memmove(&v13, &v16, sizeof(v15));

    v24 = -1;
    auto v17 = *v3;
    if (*v3)
    {
        if (((v3[2] - v17) & 0xFFFFFFFC) < 0x1000)
        {
        LABEL_23:
            goto LABEL_24;
        }
        if ((v17 - (v17 - 4) - 4) <= 0x1F)
        {
            v17 = (v17 - 4);
            goto LABEL_23;
        }
    }
LABEL_24:
    *v3 = v13;
    v3[1] = v13 + 4 * v19;
    v3[2] = v11 + v13;
}

bool __thiscall CPacketModify::Init(CPacketManager* m_PacketMgr, int m_nSequence)
{
    if (!m_PacketMgr)
        return false;

    m_PacketMgr->m_nLastSequence = m_PacketMgr->m_nSequence;
    m_PacketMgr->m_nSequence = m_nSequence;
    if (!ctx.m_local())
        return false;

    auto iCmdSlot = m_nSequence % 150;
    auto m_pCmd = csgo.m_input()->GetUserCmd(m_nSequence);
    auto m_pVerifiedCmd = csgo.m_input()->GetVerifiedUserCmd(m_nSequence);

    auto g_iTickcount = m_pCmd->tick_count;
    auto g_nCommand = m_pCmd->command_number;
    int iCommand = 88 * (m_PacketMgr->m_nSequence % 150);
    *&m_PacketMgr->m_aCommands->forwardmove = m_pCmd->forwardmove;
    *&m_PacketMgr->m_aCommands->sidemove = m_pCmd->sidemove;
    *&m_PacketMgr->m_aCommands->upmove = m_pCmd->upmove;
    *&m_PacketMgr->m_aCommands->tick_count = m_pCmd->tick_count;
    if (m_PacketMgr->m_bResetModifablePacket)
    {
        auto v13 = GetModifablePacket(m_PacketMgr);
        v13->tick_count = 0.0;
        v13->forwardmove = 0.0;
        v13->sidemove = 0.0;
        v13->upmove = 0;
        *&v13->buttons = 0;
        m_PacketMgr->m_bResetModifablePacket = 0;
    }
    auto v16 = m_PacketMgr->pint33A4;
    m_nSequence = (m_pCmd->command_number % 150 + 4);
    auto m_bValueToReturn = 1;
    if (v16 == m_PacketMgr->pint33A4)
    {
        Update(&m_PacketMgr->gap339D[3], v16, m_nSequence);
        m_bValueToReturn = 1;
    }
    else
    {
        m_bValueToReturn = 1;
        ++m_PacketMgr->pint33A4;
    }
    return m_bValueToReturn;
}