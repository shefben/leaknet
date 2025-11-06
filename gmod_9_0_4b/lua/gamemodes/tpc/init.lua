

_OpenScript( "gamemodes/tpc/gamerules.lua" );
_OpenScript( "gamemodes/tpc/events.lua" );
_OpenScript( "gamemodes/tpc/gui.lua" );
_OpenScript( "gamemodes/tpc/controls.lua" );


TEAMA_MODEL		= "models/player/male_08.mdl";
TEAMB_MODEL		= "models/player/male_02.mdl";


PlayerInfo = {}


for i=1, _MaxPlayers() do

	PlayerInfo[i] = {};
	PlayerInfo[i].Money = 0;

end


_EntPrecacheModel( TEAMA_MODEL );
_EntPrecacheModel( TEAMB_MODEL );
