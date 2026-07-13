//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: Per-tool-mode free function declarations dispatched by
//          CWeaponTool::OnToolUse/OnToolTrace/OnToolThink (weapon_tool.cpp).
//
//          Each tool_*.cpp implements the 3 functions for its ToolMode_t
//          value(s). CWeaponTool is composed with these, not subclassed -
//          there is exactly one entity class ("weapon_tool") for every mode,
//          so per-mode behavior has to be a switch, not virtual dispatch.
//
//=============================================================================

#ifndef TOOL_DISPATCH_H
#define TOOL_DISPATCH_H
#ifdef _WIN32
#pragma once
#endif

class CWeaponTool;
class CBaseEntity;
class CGameTrace;
typedef CGameTrace trace_t;

// Constraint family: Rope(0) Elastic(1) Weld(2) Ballsocket(3) Pulley(4)
// EasyWeld(5) EasyBall(6) Axis(7) Slider(8) NailGun(9)
void Tool_Constraint_OnUse( CWeaponTool *pTool, int nMode, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Constraint_OnTrace( CWeaponTool *pTool, int nMode, trace_t &tr, bool bPrimary );
void Tool_Constraint_OnThink( CWeaponTool *pTool, int nMode );

// FacePoser(10) EyesPoser(11)
void Tool_Poser_OnUse( CWeaponTool *pTool, int nMode, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Poser_OnTrace( CWeaponTool *pTool, int nMode, trace_t &tr, bool bPrimary );
void Tool_Poser_OnThink( CWeaponTool *pTool, int nMode );

// Remover(12)
void Tool_Remover_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Remover_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_Remover_OnThink( CWeaponTool *pTool );

// Ignite(13) NoCollide(18) Dynamite(19) Magnetise(17) Statue(25) - simple
// single/pending-click property toggles on props.
void Tool_Simple_OnUse( CWeaponTool *pTool, int nMode, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Simple_OnTrace( CWeaponTool *pTool, int nMode, trace_t &tr, bool bPrimary );
void Tool_Simple_OnThink( CWeaponTool *pTool, int nMode );

// Paint(14)
void Tool_Paint_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Paint_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_Paint_OnThink( CWeaponTool *pTool );

// Duplicate(15)
void Tool_Duplicator_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Duplicator_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_Duplicator_OnThink( CWeaponTool *pTool );

// Colour(16)
void Tool_Color_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Color_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_Color_OnThink( CWeaponTool *pTool );

// Material(20)
void Tool_Material_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Material_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_Material_OnThink( CWeaponTool *pTool );

// RTCamera(21) Thruster(23) PhysProps(24) Balloon(26) Emitter(27) Sprite(28) Wheel(29)
// - attach a decorator entity/behavior to the targeted prop.
void Tool_Attach_OnUse( CWeaponTool *pTool, int nMode, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Attach_OnTrace( CWeaponTool *pTool, int nMode, trace_t &tr, bool bPrimary );
void Tool_Attach_OnThink( CWeaponTool *pTool, int nMode );

// Gun(30) Camera(31) NPCSpawn(32) - auxiliary, not menu-reachable via gm_toolmode yet
void Tool_Gun_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Gun_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_Gun_OnThink( CWeaponTool *pTool );

void Tool_Camera_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Camera_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_Camera_OnThink( CWeaponTool *pTool );

void Tool_NPC_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_NPC_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_NPC_OnThink( CWeaponTool *pTool );

// Inflator(33) - resize tool, no authentic gm_toolmode slot
void Tool_Inflator_OnUse( CWeaponTool *pTool, CBaseEntity *pEntity, trace_t &tr, bool bPrimary );
void Tool_Inflator_OnTrace( CWeaponTool *pTool, trace_t &tr, bool bPrimary );
void Tool_Inflator_OnThink( CWeaponTool *pTool );

#endif // TOOL_DISPATCH_H
