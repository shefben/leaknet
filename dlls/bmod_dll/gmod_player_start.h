//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GMod player spawn point entity header
//
//=============================================================================//

#ifndef GMOD_PLAYER_START_H
#define GMOD_PLAYER_START_H
#pragma once

class CBaseEntity;
class CBasePlayer;

//-----------------------------------------------------------------------------
// Helper function to find a valid gmod_player_start for a player
// Searches for gmod_player_start entities that match the player's team
// Returns NULL if no valid spawn point found
//-----------------------------------------------------------------------------
CBaseEntity* FindGModPlayerStart(CBasePlayer* pPlayer);

#endif // GMOD_PLAYER_START_H
