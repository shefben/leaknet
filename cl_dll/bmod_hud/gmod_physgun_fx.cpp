// Physgun visual effect precache and ConVar bindings for the 2003 client.
#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "convar.h"

// Beam color controls (matching GMod physgun color cvars)
ConVar physgun_r( "physgun_r", "0", FCVAR_ARCHIVE, "Physgun beam red component" );
ConVar physgun_g( "physgun_g", "0", FCVAR_ARCHIVE, "Physgun beam green component" );
ConVar physgun_b( "physgun_b", "255", FCVAR_ARCHIVE, "Physgun beam blue component" );

// Materials used by the physgun beam/halo/glow stack
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectPhysgunFX )
	CLIENTEFFECT_MATERIAL( "sprites/physbeam" )
	CLIENTEFFECT_MATERIAL( "sprites/physgbeam" )
	CLIENTEFFECT_MATERIAL( "sprites/physgbeamB" )
	CLIENTEFFECT_MATERIAL( "sprites/orangecore1" )
	CLIENTEFFECT_MATERIAL( "sprites/orangecore2" )
	CLIENTEFFECT_MATERIAL( "sprites/orangeflare1" )
	CLIENTEFFECT_MATERIAL( "sprites/glow04_noz" )
	CLIENTEFFECT_MATERIAL( "sprites/orangelight1_noz" )
	CLIENTEFFECT_MATERIAL( "sprites/orangelight1" )
	CLIENTEFFECT_MATERIAL( "sprites/physcannon_bluelight2" )
	CLIENTEFFECT_MATERIAL( "sprites/blueglow1" )
CLIENTEFFECT_REGISTER_END();
