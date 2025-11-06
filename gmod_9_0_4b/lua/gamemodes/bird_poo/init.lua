
VERSION_TEXT		= "Version 2.0"

BT_SEAGULL 		= 0
BT_CROW 		= 1
BT_PIGEON 		= 2

TEAM_BIRDS		= TEAM_BLUE
TEAM_HUMANS		= TEAM_YELLOW

WALK_SPEED		= 100
FAST_WALK_SPEED		= 150
NORMAL_SPEED		= 300
FAST_SPEED		= 500

MAX_RUN			= 50

MAX_HUD_POOP		= 50

DRAW_POOP_LINE		= 0

_OpenScript( "includes/Events.lua" )

_OpenScript( "gamemodes/bird_poo/playerdata.lua" );
_OpenScript( "gamemodes/bird_poo/gamerules.lua" );
_OpenScript( "gamemodes/bird_poo/events.lua" );
_OpenScript( "gamemodes/bird_poo/gui.lua" );
_OpenScript( "gamemodes/bird_poo/controls.lua" );
_OpenScript( "gamemodes/bird_poo/cmds.lua" );

_EntPrecacheModel("models/crow.mdl")
_EntPrecacheModel("models/pigeon.mdl")
_EntPrecacheModel("models/seagull.mdl")
_EntPrecacheModel("models/player/male_02.mdl")
_EntPrecacheModel("models/player/male_03.mdl")

_Msg("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n")
_Msg("     gm_birdpoo - " .. VERSION_TEXT .. " - By Andy Vincent\n")
_Msg("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n")

