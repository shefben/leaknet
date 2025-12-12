// Client-side glue for gmod-style commands/CVars that mirror the original client.dll.
#include "cbase.h"
#include "convar.h"
#include "cdll_int.h"
#include <string.h>

#ifdef BMOD_CLIENT_DLL

ConVar gm_spawncombolines(
	"gm_spawncombolines",
	"20",
	FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
	"Number of lines to display in spawn combo boxes");

ConVar gm_wepselmode(
	"gm_wepselmode",
	"1",
	FCVAR_CLIENTDLL | FCVAR_ARCHIVE,
	"Weapon selection mode (0 = legacy, 1 = condense inactive, 2 = condense all)");

ConVar gm_toolweapon(
	"gm_toolweapon",
	"1",
	FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_USERINFO,
	"Current tool weapon selection");

// Balloon client cvars (match gmod client.dll so context panels function)
ConVar gm_balloon_reverse("gm_balloon_reverse", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Make balloons pull down");
ConVar gm_balloon_power("gm_balloon_power", "300", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Balloon lift power");
ConVar gm_balloon_rope_width("gm_balloon_rope_width", "0.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Balloon rope width");
ConVar gm_balloon_rope_length("gm_balloon_rope_length", "50", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Balloon rope length");
ConVar gm_balloon_rope_forcelimit("gm_balloon_rope_forcelimit", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Balloon rope force limit");
ConVar gm_balloon_rope_rigid("gm_balloon_rope_rigid", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Use rigid rope for balloons");
ConVar gm_balloon_rope_type("gm_balloon_rope_type", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Balloon rope type");
ConVar gm_balloon_explode("gm_balloon_explode", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Balloons explode when shot");
ConVar gm_thruster_sounds("gm_thruster_sounds", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Play thruster sounds (clientside)");

// Avoid recursive forwarding when echoing commands to the server.
static bool g_bForwarding = false;

static void ForwardCommandToServer()
{
	if ( g_bForwarding )
		return;

	int argc = engine ? engine->Cmd_Argc() : 0;
	if ( argc <= 0 )
		return;

	char cmd[512];
	cmd[0] = '\0';
	for ( int i = 0; i < argc; ++i )
	{
		const char *arg = engine->Cmd_Argv( i );
		if ( !arg )
			continue;
		if ( i > 0 )
			strncat( cmd, " ", sizeof( cmd ) - strlen(cmd) - 1 );
		strncat( cmd, arg, sizeof( cmd ) - strlen(cmd) - 1 );
	}

	g_bForwarding = true;
	engine->ClientCmd_Unrestricted( cmd );
	g_bForwarding = false;
}

// Client proxies for the make commands so UI clicks forward to the server implementation.
CON_COMMAND( gmod_makeprop, "Create a prop at the crosshair" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: gmod_makeprop <modelpath>\n" );
		return;
	}
	ForwardCommandToServer(); // use Cmd_Args internally
}

CON_COMMAND( gmod_makeragdoll, "Create a ragdoll at the crosshair" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: gmod_makeragdoll <modelpath>\n" );
		return;
	}
	ForwardCommandToServer();
}

CON_COMMAND( gmod_makeeffect, "Create an effect at the crosshair" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: gmod_makeeffect <effectname>\n" );
		return;
	}
	ForwardCommandToServer();
}

// Forward gm_spawn (used by command/menu entries) straight to the server.
CON_COMMAND( gm_spawn, "Spawn an entity or model (forwards to server)" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: gm_spawn <classname or modelpath>\n" );
		return;
	}
	ForwardCommandToServer();
}

// Simple helper to spawn a model or ragdoll from the client (mirrors spawn menu behavior).
CON_COMMAND( gm_spawn_model, "Spawn a model (auto-selects prop or ragdoll)" )
{
	if ( engine->Cmd_Argc() < 2 )
	{
		Msg( "Usage: gm_spawn_model <modelpath>\n" );
		return;
	}

	const char *model = engine->Cmd_Argv(1);
	if ( !model ) return;

	// Heuristic: ragdoll if name contains ragdoll/corpse/dead (matches IsRagdollModel in server code)
	char lower[256]; Q_strncpy(lower, model, sizeof(lower));
	Q_strlower(lower);
	bool bRagdoll = ( Q_stristr(lower, "ragdoll") || Q_stristr(lower, "corpse") || Q_stristr(lower, "dead") ) ? true : false;

	if ( bRagdoll )
	{
		char cmd[512]; Q_snprintf(cmd, sizeof(cmd), "gmod_makeragdoll %s", model);
		engine->ClientCmd_Unrestricted(cmd);
	}
	else
	{
		char cmd[512]; Q_snprintf(cmd, sizeof(cmd), "gmod_makeprop %s", model);
		engine->ClientCmd_Unrestricted(cmd);
	}
}

#endif // BMOD_CLIENT_DLL
