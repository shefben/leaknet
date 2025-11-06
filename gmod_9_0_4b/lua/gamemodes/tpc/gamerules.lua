



	function gamerulesThink ()

		

		DoControls();

		

	end 

	



	function GiveDefaultItems( playerid ) 
		
		if ( _PlayerInfo( playerid, "team" ) == TEAM_SPECTATOR )  then return; end;
		
		_PlayerGiveSWEP( playerid, "weapons/tpc/tpc_usp.lua" );
		
	end

	

	

	function gamerulesStartMap ()

	
		PlayerFreezeAll( false )

		_TeamSetName( TEAM_RED, "Team Freedom" )
		_TeamSetName( TEAM_YELLOW, "Team Liberation" )

		-- Precache the models 

			

	end



	function PlayerSpawnChooseModel ( playerid )	

		if ( _PlayerInfo( playerid, "team" ) == TEAM_YELLOW )  then
			
			_PlayerSetModel( playerid, TEAMA_MODEL )

		else
		
			_PlayerSetModel( playerid, TEAMB_MODEL )
			
		end
				
	end




	function PickDefaultSpawnTeam( userid )

		_PlayerChangeTeam( userid, TEAM_SPECTATOR );
		return true; 

	end

	

	function eventPlayerActive ( name, userid, steamid )

		DrawIntro( userid );
		
	end


	function GiveMoney( player, amount )
		
		PlayerInfo[player].Money = PlayerInfo[player].Money + amount;
		DrawCash( player );
		
	end

	function CanBuy( player, amount )
		
		if ( PlayerInfo[player].Money < amount ) then return false; end
		return true;		
		
	end
	
	
	function TakeCash( player, amount )
		
		PlayerInfo[player].Money = PlayerInfo[player].Money - amount;
		DrawCash( player );
		
	end