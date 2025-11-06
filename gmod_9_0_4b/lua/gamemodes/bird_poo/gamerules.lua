
function gamerulesThink ()

	DoControls();

end 

function GiveDefaultItems( playerid ) 
	if (_PlayerInfo(playerid, "team") == TEAM_HUMANS) then

		_PlayerGiveItem( playerid, "weapon_stunstick" )
		_PlayerGiveItem( playerid, "weapon_pistol" )
		_PlayerGiveItem( playerid, "weapon_357" )
		_PlayerGiveItem( playerid, "weapon_crossbow" )

		_PlayerGiveAmmo( playerid, 255, "Pistol", false )
		_PlayerGiveAmmo( playerid, 255, "357", false )
		_PlayerGiveAmmo( playerid, 255, "XBowBolt", false )

	end
end

function gamerulesStartMap ()
	_TeamSetName( TEAM_BIRDS, "The Birds" );
	_TeamSetName( TEAM_HUMANS, "The People" );

	PlayerFreezeAll( false )	
end

function PlayerSpawnChooseModel ( playerid )	
	if (_PlayerInfo(playerid, "team") == TEAM_HUMANS) then

		local iRand = math.random( 2, 3 )

		_PlayerSetModel( playerid, "models/player/male_0"..iRand..".mdl" )

	end
end

function PickDefaultSpawnTeam( userid )
	_PlayerChangeTeam( userid, TEAM_SPECTATOR );
	return true; 
end



