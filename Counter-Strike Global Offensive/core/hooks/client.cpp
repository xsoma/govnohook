#include <hooks/hooked.hpp>
#include <props/displacement.hpp>
#include <props/player.hpp>
#include <props/weapon.hpp>
#include <ragebot/prediction.hpp>
#include <misc/movement.hpp>
#include <ragebot/rage_aimbot.hpp>
#include <antiaim/anti_aimbot.hpp>
#include <ragebot/lag_comp.hpp>
#include <intrin.h>
#include <menu/menu/menu.hpp>
#include "usercmd.hpp"
#include <visuals/visuals.hpp>
#include <props/prop_manager.hpp>
#include <visuals/sound_parser.hpp>
#include <features/netchannel/net_channel.h>

struct twoints
{
	int first; int second;
};

std::vector<twoints> g_states;

namespace Hooked
{
	bool in_ping_spike{}, flipped_state{};

	float calculate_wanted_ping(INetChannel* channel)
	{
		if (!csgo.m_engine()->GetNetChannelInfo())
			return 0.f;

		auto wanted_ping = 0.f;

		/*if (in_ping_spike)
			wanted_ping = (200.f / 1000.f);
		else */if (ctx.m_settings.settings_low_fps_warning)
			wanted_ping = (200.f / 1000.f) - ctx.lerp_time;
		else
			return 0.f;

		return max(0.f, wanted_ping - csgo.m_engine()->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) * 2.f);
	}

	void set_suitable_in_sequence(INetChannel* channel)
	{
		if (flipped_state || ctx.exploit_allowed)
		{
			flipped_state = false;
			return;
		}

		const auto spike = TIME_TO_TICKS(calculate_wanted_ping(channel));

		if (channel->in_sequence_nr > spike)
			channel->in_sequence_nr -= spike;
	}

	void flip_state(INetChannel* channel)
	{
		static auto last_reliable_state = -1;

		if (channel->in_reliable_state != last_reliable_state)
			flipped_state = true;

		last_reliable_state = channel->in_reliable_state;
	}

	bool __fastcall ProcessTempEntities(pPastaState* cl_state, void* EDX, void* msg)
	{
		using Fn = bool(__thiscall*)(void*, void*);
		static auto ofunc = vmt.m_clientstate->VCall<Fn>(36);

		auto o_ents = csgo.m_client_state()->m_nMaxClients;
		csgo.m_client_state()->m_nMaxClients = 1;
		const auto ret = ofunc(cl_state, msg);
		csgo.m_client_state()->m_nMaxClients = o_ents;

		csgo.m_engine()->FireEvents();

		return ret;
	}

	void __fastcall PacketEnd(pPastaState* cl_state, void* EDX)
	{
		using Fn = void(__thiscall*)(void*);
		static auto ofunc = vmt.m_clientstate->VCall<Fn>(6);

		if (!csgo.m_engine()->IsInGame()) {
			ctx.m_corrections_data.clear();
			ctx.command_numbers.clear();
			return ofunc(cl_state);
		}

		if (!ctx.m_local() || ctx.m_local()->IsDead()) {
			ctx.m_corrections_data.clear();
			ctx.command_numbers.clear();
			return ofunc(cl_state);
		}

		if (*reinterpret_cast<int*>(uintptr_t(cl_state) + 0x164) == *reinterpret_cast<int*>(uintptr_t(cl_state) + 0x16C)) {
			feature::sound_parser->get_active_sounds();
			auto lastCommand = (*(int(__thiscall**)(void*))(*(uintptr_t*)csgo.m_engine() + 820))(csgo.m_engine());

			if (ctx.allow_shooting > 0 && lastCommand >= ctx.allow_shooting)
				ctx.allow_shooting = 0;
		}

		ofunc(cl_state);
	}

	int __fastcall SendDatagram(void* netchan, void*, void* datagram)
	{
		const auto ofunc = vmt.m_net_channel->VCall<int(__thiscall*)(void*, void*)>(46);
		INetChannel* chan = (INetChannel*)netchan;
		if (!csgo.m_engine()->IsInGame() || !ctx.m_local())
			return ofunc(chan, datagram);

		bf_write* data = (bf_write*)datagram;
		INetChannel* client_netchan = (INetChannel*)netchan;
		static int times = 0;
		static int tickstart = 0;
		static float nexttime = 0;
		static bool isplaying = false;

		int instate = chan->in_reliable_state;
		int insequencenr = chan->in_sequence_nr;

		int out = chan->out_sequence_nr;
		int ret;
		int originalchokedpackets = chan->choked_packets;
		double originalcleartime = chan->m_fClearTime;

		auto Last_InSequenceNrSent = client_netchan->in_sequence_nr;

		ctx.in_send_datagram = true;
		ret = ofunc(netchan, data);
		ctx.in_send_datagram = false;

		auto latency_out = chan->get_latency(FLOW_OUTGOING);
		if (latency_out < 0.2f)
		{
			auto v13 = chan->in_sequence_nr - (((0.2f - latency_out) / csgo.m_globals()->interval_per_tick) + 0.5f);
			chan->in_sequence_nr = v13;

			for (auto& seq : feature::net_channel->m_sequences)
			{
				if (seq.m_seq != seq.m_state)
				{
					//std::clamp(seq.m_seq, 0, (int)v13);

					while (seq.m_seq <= v13)
					{
						seq.m_seq += 2;
						if (seq.m_seq == v13)
						{
							chan->in_reliable_state = instate;
							chan->in_sequence_nr = insequencenr;

							return ret;
						}
					}

					chan->in_reliable_state = seq.m_state;
				}
			}
		}

		int nTotalSize = (int)((chan->m_fClearTime - originalcleartime) * chan->m_Rate);

		feature::net_channel->FlowNewPacket(chan, FLOW_OUTGOING, chan->out_sequence_nr, Last_InSequenceNrSent, originalchokedpackets, 0, nTotalSize);
		feature::net_channel->FlowUpdate(chan, FLOW_OUTGOING, nTotalSize);

		chan->in_reliable_state = instate;
		chan->in_sequence_nr = insequencenr;

		return ret;
	}

	void __fastcall PacketStart(void* ecx, void* edx, int incoming_sequence, int outgoing_acknowledged)
	{
		using Fn = void(__thiscall*)(void*, int, int);
		static auto ofunc = vmt.m_clientstate->VCall<Fn>(5);

		if (!ctx.command_numbers.empty()) {
			for (auto it = ctx.command_numbers.rbegin(); it != ctx.command_numbers.rend(); ++it) {
				if (!it->is_outgoing) {
					continue;
				}

				if (it->command_nr == outgoing_acknowledged
					|| outgoing_acknowledged > it->command_nr && (!it->is_used || it->prev_command_nr == outgoing_acknowledged)) {

					it->prev_command_nr = outgoing_acknowledged;
					it->is_used = true;
					ofunc(ecx,
						incoming_sequence, it->command_nr);

					break;
				}
			}

			auto result = false;

			for (auto it = ctx.command_numbers.begin(); it != ctx.command_numbers.end(); ) {
				if (outgoing_acknowledged == it->command_nr || outgoing_acknowledged == it->prev_command_nr)
					result = true;

				if (outgoing_acknowledged > it->command_nr && outgoing_acknowledged > it->prev_command_nr) {
					it = ctx.command_numbers.erase(it);
				}
				else {
					it++;
				}
			}

			if (!result)
				ofunc(ecx, incoming_sequence, outgoing_acknowledged);
		}
		else {

			ofunc(ecx,
				incoming_sequence, outgoing_acknowledged);
		}
	}

	void __fastcall  ProcessPacketNet(INetChannel* channel, uint32_t, void* packet, bool header)
	{
		using Fn = void(__thiscall*)(void* net, void* packet, bool bHasHeader);
		const auto ofc = vmt.m_net_channel->VCall<Fn>(39);
		INetChannel* netchan = (INetChannel*)channel;
		netpacket_t* pack = (netpacket_t*)packet;

		bool ShouldGetChokedTicks = netchan->m_Name[0] == 'C' && netchan->m_Name[5] == 'T'; //CLIENT

		float original_last_received = netchan->last_received;

		if (ShouldGetChokedTicks)
		{
			feature::net_channel->FlowUpdate(netchan, FLOW_INCOMING, pack->wiresize + UDP_HEADER_SIZE);
		}

		ofc(channel, packet, header);

		if (ShouldGetChokedTicks && netchan->last_received != original_last_received)
		{
			feature::net_channel->FlowNewPacket(netchan, FLOW_INCOMING, netchan->in_sequence_nr, netchan->out_sequence_nr_ack, netchan->choked_packets, netchan->m_PacketDrop, pack->wiresize + UDP_HEADER_SIZE);
		}

		feature::net_channel->UpdateIncomingSequences();
	}
	bool __fastcall SendNetMsg(void* NetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice)
	{
		using Fn = bool(__thiscall*)(void* NetChan, INetMessage& msg, bool bForceReliable, bool bVoice);
		const auto ofc = vmt.m_net_channel->VCall<Fn>(40);
		INetChannel* pNetChan = (INetChannel*)NetChan;
		if (ctx.m_local() && ctx.m_local() != nullptr && csgo.m_engine()->IsInGame())
		{
			if (msg.GetType() == 14) // Return and don't send messsage if its FileCRCCheck
				return true;

			if (msg.GetGroup() == 9) // Fix lag when transmitting voice and fakelagging
				bVoice = true;

			return ofc(NetChan, msg, bForceReliable, bVoice);
		}

		return ofc(NetChan, msg, bForceReliable, bVoice);
	}

	void __fastcall Shutdown(INetChannel* pNetChan, void* EDX, const char* reason) {
		using Fn = void(__thiscall*)(INetChannel*, const char*);
		const auto ofc = vmt.m_net_channel->VCall<Fn>(27);

		vmt.m_net_channel.reset();
		vmt.m_net_channel = nullptr;

		return ofc(pNetChan, reason);
	}
}