#include "net_channel.h"

void c_net_channel::FlowNewPacket(INetChannel* chan, int flow, int seqnr, int acknr, int nChoked, int nDropped, int nSize)
{
	//netflow_t * m_DataFlow = (netflow_t*)((DWORD)chan + 0x5C0);
	netflow_t* pflow = &chan->m_DataFlow[flow];

	// if frame_number != ( current + 1 ) mark frames between as invalid

	netframe_t_firstpart* pfirstpart = NULL;
	netframe_t_secondpart* psecondpart = NULL;

	if (seqnr > pflow->currentindex)
	{
		int framecount = 0;
		for (int i = pflow->currentindex + 1; i <= seqnr && framecount < NET_FRAMES_BACKUP; i++)
		{
			int nBackTrack = seqnr - i;

			pfirstpart = &pflow->frames[i & NET_FRAMES_MASK];
			pfirstpart->time = chan->m_fClearTime;	// now
			pfirstpart->valid = false;
			pfirstpart->size = 0;
			pfirstpart->latency = -1.0f; // not acknowledged yet
			pfirstpart->choked = 0; // not acknowledged yet

			psecondpart = &pflow->frames2[i & NET_FRAMES_MASK];
			psecondpart->avg_latency = ctx.avglatency[FLOW_OUTGOING];
			psecondpart->dropped = 0;
			psecondpart->m_flInterpolationAmount = 0.0f;
			memset(&psecondpart->msggroups, 0, sizeof(psecondpart->msggroups));

			if (nBackTrack < (nChoked + nDropped))
			{
				if (nBackTrack < nChoked)
					pfirstpart->choked = 1;
				else
					psecondpart->dropped = 1;
			}
			framecount++;
		}

		psecondpart->dropped = nDropped;
		pfirstpart->choked = nChoked;
		pfirstpart->size = nSize;
		pfirstpart->valid = true;
		psecondpart->avg_latency = ctx.avglatency[FLOW_OUTGOING];
		psecondpart->m_flInterpolationAmount = chan->m_flInterpolationAmount;
	}
	else
	{
		//Assert(demoplayer->IsPlayingBack() || seqnr > pflow->currentindex);
	}

	pflow->totalpackets++;
	pflow->currentindex = seqnr;
	pflow->currentframe = pfirstpart;

	// updated ping for acknowledged packet

	int aflow = (flow == FLOW_OUTGOING) ? FLOW_INCOMING : FLOW_OUTGOING;

	if (acknr <= (chan->m_DataFlow[aflow].currentindex - NET_FRAMES_BACKUP))
		return;	// acknowledged packet isn't in backup buffer anymore

	netframe_t_firstpart* aframe = &chan->m_DataFlow[aflow].frames[acknr & NET_FRAMES_MASK];

	if (aframe->valid && aframe->latency == -1.0f)
	{
		// update ping for acknowledged packet, if not already acknowledged before

		aframe->latency = chan->m_fClearTime - aframe->time;

		if (aframe->latency < 0.0f)
			aframe->latency = 0.0f;
	}

}

void c_net_channel::FlowUpdate(INetChannel* chan, int flow, int addbytes)
{
	//netflow_t * m_DataFlow = (netflow_t*)((DWORD)chan + 0x5C0);
	netflow_t* pflow = &chan->m_DataFlow[flow];
	pflow->totalbytes += addbytes;

	if (pflow->nextcompute > chan->m_fClearTime)
		return;

	pflow->nextcompute = chan->m_fClearTime + FLOW_INTERVAL;

	int		totalvalid = 0;
	int		totalinvalid = 0;
	int		totalbytes = 0;
	float	totallatency = 0.0f;
	int		totallatencycount = 0;
	int		totalchoked = 0;

	float   starttime = FLT_MAX;
	float	endtime = 0.0f;

	//netframe_t_firstpart *pprev = &pflow->frames[NET_FRAMES_BACKUP - 1];

	for (int i = 0; i < NET_FRAMES_BACKUP; i++)
	{
		// Most recent message then backward from there
		netframe_t_firstpart* pcurr = &pflow->frames[i];

		if (pcurr->valid)
		{
			if (pcurr->time < starttime)
				starttime = pcurr->time;

			if (pcurr->time > endtime)
				endtime = pcurr->time;

			totalvalid++;
			totalchoked += pcurr->choked;
			totalbytes += pcurr->size;

			if (pcurr->latency > -1.0f)
			{
				totallatency += pcurr->latency;
				totallatencycount++;
			}
		}
		else
		{
			totalinvalid++;
		}

		//pprev = pcurr;
	}

	float totaltime = endtime - starttime;

	if (totaltime > 0.0f)
	{
		pflow->avgbytespersec *= FLOW_AVG;
		pflow->avgbytespersec += (1.0f - FLOW_AVG) * ((float)totalbytes / totaltime);

		pflow->avgpacketspersec *= FLOW_AVG;
		pflow->avgpacketspersec += (1.0f - FLOW_AVG) * ((float)totalvalid / totaltime);
	}

	int totalPackets = totalvalid + totalinvalid;

	if (totalPackets > 0)
	{
		pflow->avgloss *= FLOW_AVG;
		pflow->avgloss += (1.0f - FLOW_AVG) * ((float)(totalinvalid - totalchoked) / totalPackets);

		if (pflow->avgloss < 0)
			pflow->avgloss = 0;

		pflow->avgchoke *= FLOW_AVG;
		pflow->avgchoke += (1.0f - FLOW_AVG) * ((float)totalchoked / totalPackets);
	}
	else
	{
		pflow->avgloss = 0.0f;
	}

	if (totallatencycount > 0)
	{
		float newping = totallatency / totallatencycount;
		pflow->latency = newping;
		pflow->avglatency *= FLOW_AVG;
		pflow->avglatency += (1.0f - FLOW_AVG) * newping;
	}
}

void c_net_channel::UpdateIncomingSequences()
{
	if (!csgo.m_client_state()->m_ptrNetChannel)
		return;

	if (m_sequences.empty() || csgo.m_client_state()->m_ptrNetChannel->in_sequence_nr > m_sequences.front().m_seq) {
		// store new stuff.
		m_sequences.emplace_front(csgo.m_globals()->realtime, csgo.m_client_state()->m_ptrNetChannel->in_reliable_state, csgo.m_client_state()->m_ptrNetChannel->in_sequence_nr);
	}

	// do not save too many of these.
	while (m_sequences.size() > 2048)
		m_sequences.pop_back();
}

void c_net_channel::AddLatencyToNetchan(INetChannel* netchan, float Latency)
{
	for (auto& seq : m_sequences)
	{
		if (csgo.m_globals()->realtime - seq.m_time >= Latency)
		{
			netchan->in_reliable_state = seq.m_state;
			netchan->in_sequence_nr = seq.m_seq;
			break;
		}
	}
}