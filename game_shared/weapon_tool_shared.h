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
// Tool mode definitions - matching Garry's Mod values discovered in IDA
//-----------------------------------------------------------------------------
enum ToolMode_t
{
	TOOL_NONE = 0,
	TOOL_GUN = 1,			// Gun tool
	TOOL_PHYSGUN = 2,		// Physics gun (should use weapon_physcannon instead)
	TOOL_CAMERA = 3,		// Camera tool
	TOOL_NPC = 4,			// NPC spawning tool
	TOOL_NEXTBOT = 5,		// NextBot tool
	TOOL_MATERIAL = 6,		// Material tool
	TOOL_ROPE = 7,			// Rope tool
	TOOL_PULLEY = 8,		// Pulley tool
	TOOL_WHEEL = 9,			// Wheel tool
	TOOL_THRUSTER = 10,		// Thruster tool
	TOOL_EMITTER = 11,		// Emitter tool
	TOOL_WELDER = 12,		// Welder tool
	TOOL_HYDRAULIC = 13,	// Hydraulic tool
	TOOL_BALLSOCKET = 14,	// Ball socket tool
	TOOL_SLIDER = 15,		// Slider tool
	TOOL_DUPLICATOR = 16,	// Duplicator tool
	TOOL_REMOVER = 17,		// Remover tool
	TOOL_NOCOLLIDE = 18,	// No collide tool
	TOOL_PAINT = 19,		// Paint tool

	TOOL_MAX = 20			// Maximum tool count
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