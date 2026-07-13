//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: BarrysMod Tool System - Shared definitions
//          Constants and enums shared between client and server
//
//=============================================================================

#ifndef WEAPON_TOOL_SHARED_H
#define WEAPON_TOOL_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Tool mode definitions - MUST match dlls/bmod_dll/weapon_tool.h's ToolMode_t
// exactly. These are the authentic GMod 9.0.4b gm_toolmode IDs (verified via
// IDA against the real server.dll/client.dll and the shipped menu_main/*.txt
// content files) - the server is authoritative and networks m_nToolMode as a
// plain int, so this copy exists only for client-side code that can't include
// the server-only dlls/bmod_dll/weapon_tool.h (which pulls in CBaseCombatWeapon,
// player.h, etc). Do not renumber independently of that file.
//-----------------------------------------------------------------------------
enum ToolMode_t
{
	TOOL_NONE		= -1,	// sentinel only - falls back to TOOL_WELD

	TOOL_ROPE		= 0,
	TOOL_ELASTIC	= 1,
	TOOL_WELD		= 2,
	TOOL_BALLSOCKET	= 3,
	TOOL_PULLEY		= 4,
	TOOL_EASYWELD	= 5,
	TOOL_EASYBALL	= 6,
	TOOL_AXIS		= 7,
	TOOL_SLIDER		= 8,
	TOOL_NAILGUN	= 9,
	TOOL_FACEPOSER	= 10,
	TOOL_EYESPOSER	= 11,
	TOOL_REMOVER	= 12,
	TOOL_IGNITE		= 13,
	TOOL_PAINT		= 14,
	TOOL_DUPLICATE	= 15,
	TOOL_COLOUR		= 16,
	TOOL_MAGNETISE	= 17,
	TOOL_NOCOLLIDE	= 18,
	TOOL_DYNAMITE	= 19,
	TOOL_MATERIAL	= 20,
	TOOL_RTCAMERA	= 21,
	TOOL_THRUSTER	= 23,
	TOOL_PHYSPROPS	= 24,
	TOOL_STATUE		= 25,
	TOOL_BALLOON	= 26,
	TOOL_EMITTER	= 27,
	TOOL_SPRITE		= 28,
	TOOL_WHEEL		= 29,

	TOOL_GUN		= 30,
	TOOL_CAMERA		= 31,
	TOOL_NPCSPAWN	= 32,
	TOOL_INFLATOR	= 33,

	TOOL_MAX		= 40
};

//-----------------------------------------------------------------------------
// Tool trace distance constants
//-----------------------------------------------------------------------------
#define TOOL_TRACE_DISTANCE	1024.0f
#define TOOL_MIN_DISTANCE	32.0f
#define TOOL_MAX_DISTANCE	8192.0f

//-----------------------------------------------------------------------------
// Tool network flags
//-----------------------------------------------------------------------------
#define TOOL_FLAG_NONE		0x00
#define TOOL_FLAG_ACTIVE	0x01
#define TOOL_FLAG_GHOST		0x02
#define TOOL_FLAG_FROZEN	0x04

#endif // WEAPON_TOOL_SHARED_H