#pragma once

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
typedef struct netpacket_s
{
	netadr_t		from;		// sender IP //size 10 //0
	int				source;		// received source //10
	double			received;	// received time //18

	netadr_t		to;			// receiver IP //size 10 //26
	int				ssource;	// send source //36
	double			sendtime;	// send time //40

	unsigned char* data;		// pointer to raw packet data //48
	bf_read			message;	// easy bitbuf data access //52 //size 36
	int				size;		// size in bytes //88
	int				wiresize;   // size in bytes before decompression //92
	bool			stream;		// was send as stream
	struct netpacket_s* pNext;	// for internal use, should be NULL in public
} netpacket_t;

typedef struct netpacket_s netpacket_t;

#define UDP_HEADER_SIZE 28


class Sequence {
public:
	float m_time;
	int   m_state;
	int   m_seq;

public:
	__forceinline Sequence() : m_time{ }, m_state{ }, m_seq{ } {};
	__forceinline Sequence(float time, int state, int seq) : m_time{ time }, m_state{ state }, m_seq{ seq } {};
};

class c_net_channel
{
public:
	std::deque< Sequence > m_sequences;
	virtual void FlowNewPacket(INetChannel* chan, int flow, int seqnr, int acknr, int nChoked, int nDropped, int nSize);
	virtual void FlowUpdate(INetChannel* chan, int flow, int addbytes);
	virtual void UpdateIncomingSequences();
	virtual void AddLatencyToNetchan(INetChannel* netchan, float Latency);
};