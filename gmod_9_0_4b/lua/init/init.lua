

-- !! DO NOT edit this script. !!

-- Any scripts dropped into the init/ folder are automatically run on startup.

-- This is the only script called when the game starts up
-- It should be used to initialize and call other scripts



-- Do not use the default LUA method to call other scripts
--  because it will be trying to open the files from the 
--  folder with hl2.exe in it.
-- Instead use the function _OpenScript( <scriptname> )


_OpenScript( "includes/defines.lua" );
_OpenScript( "includes/concommands.lua" );
_OpenScript( "includes/backcompat.lua" );
_OpenScript( "includes/vector3.lua" );
_OpenScript( "includes/luathink.lua" );
_OpenScript( "includes/player.lua" );
_OpenScript( "includes/misc.lua" );
_OpenScript( "includes/events.lua" );
_OpenScript( "includes/timers.lua" );
_OpenScript( "includes/eventhook.lua" );

-- Open the default game script incase one isn't run later.
_OpenScript( "gamemodes/default/init.lua" );
