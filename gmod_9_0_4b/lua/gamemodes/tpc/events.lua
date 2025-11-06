


	function eventPlayerKilled ( killed, attacker, weapon )

		_PlayerAddDeath( killed, 1 )
			
		
		if (IsPlayer( attacker ) and attacker~=killed) then
		
			_PlayerAddScore( attacker, 1 );
			GiveMoney( attacker, 45 );
		
		end

	end

	
	function onShowTeam ( userid )

		_PlayerOption( userid, "onChooseTeam", 99999 );
		DrawTeamChoose( userid );

	end
	
	
	function onChooseTeam( playerid, num, seconds )

		HideTeamMenu( playerid );


		if (num == 1) then

			if ( _PlayerInfo( playerid, "team" ) == TEAM_YELLOW ) then return; end;

			_PlayerChangeTeam( playerid, TEAM_YELLOW );
			_EntSpawn( playerid );
			return;

		end

		if (num == 2) then

			if ( _PlayerInfo( playerid, "team" ) == TEAM_RED ) then return; end;

			_PlayerChangeTeam( playerid, TEAM_RED );
			_EntSpawn( playerid );

			return;
			
		end

		-- anything else is auto choose team
		if ( _TeamNumPlayers( TEAN_RED ) > _TeamNumPlayers( TEAM_YELLOW ) ) then
			onChooseTeam( playerid, 1, 0 );
		else
			onChooseTeam( playerid, 2, 0 );
		end


	end



	function eventPlayerSpawn ( userid )

		-- If they're a spectator show them the team choice menu
		if ( _PlayerInfo( userid, "team" ) == TEAM_SPECTATOR ) then
			onShowTeam( userid );
			return;
		end

		_PlayerSetDrawTeamCircle( userid, true );
		
		DrawCash( userid );

	end
	
	function onShowHelp ( userid )

		

		_GModText_Start( "Default" );
		 _GModText_SetPos( -1, 0.3 );
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 10, 0.2, 1 );
		 _GModText_SetText( "Tactical Police Cops\n\nKill the other team to get money. Spend money by pressing F3 or F4. Change teams by pressing F2" );
		_GModText_Send( userid, 0 );



	end


	function onShowSpare2 ( userid )
		
		if ( _PlayerInfo( userid, "team" ) == TEAM_SPECTATOR ) then return; end;
		
		HideTeamMenu( userid );
		_PlayerOption( userid, "onItemSelect", 10 );
		DrawPowerUps( userid );
		
	end
	
	function onItemSelect ( ply, num, seconds )
		
		HideBuyMenu( ply );
		
		if ( num == 1 and CanBuy( ply, 100) ) then
			
			_PlayerSetHealth( ply, 100 );
			TakeCash( ply, 100 );		
			
		end
		
		if ( num == 2 and CanBuy( ply, 200) ) then
			
			_PlayerSetArmor( ply, 100 );
			TakeCash( ply, 200 );		
			
		end
		
		if ( num == 3 and CanBuy( ply, 350) ) then
			
			_PlayerSetArmor( ply, 250 );
			TakeCash( ply, 350 );		
			
		end
		
	end
	
	function onShowSpare1 ( userid )
		
		if ( _PlayerInfo( userid, "team" ) == TEAM_SPECTATOR ) then return; end;
		
		HideTeamMenu( userid );
		_PlayerOption( userid, "onWeaponSelect", 10 );
		DrawWeaponMenu( userid );
		
	end
	
	
	function onWeaponSelect ( ply, num, seconds )
		
		HideBuyMenu( ply );
		
		if ( num == 1 and CanBuy( ply, 10) ) then
			
			_PlayerGiveSWEP( ply, "weapons/tpc/tpc_usp.lua" );
			TakeCash( ply, 10 );		
			
		end
	
		if ( num == 2 and CanBuy( ply, 100) ) then
			
			_PlayerGiveSWEP( ply, "weapons/tpc/tpc_deagle.lua" );
			TakeCash( ply, 100 );		
			
		end	
		
		if ( num == 3 and CanBuy( ply, 250) ) then
			
			_PlayerGiveSWEP( ply, "weapons/tpc/tpc_shotgun.lua" );
			TakeCash( ply, 270 );		
			
		end	
		
		if ( num == 4 and CanBuy( ply, 270) ) then
			
			_PlayerGiveSWEP( ply, "weapons/tpc/tpc_mac10.lua" );
			TakeCash( ply, 270 );		
			
		end	
		
		if ( num == 5 and CanBuy( ply, 300) ) then
			
			_PlayerGiveSWEP( ply, "weapons/tpc/tpc_mp5.lua" );
			TakeCash( ply, 300 );		
			
		end	
		
		if ( num == 6 and CanBuy( ply, 300) ) then
			
			_PlayerGiveSWEP( ply, "weapons/tpc/tpc_p90.lua" );
			TakeCash( ply, 300 );		
			
		end	
		
		if ( num == 7 and CanBuy( ply, 450) ) then
			
			_PlayerGiveSWEP( ply, "weapons/tpc/tpc_awp.lua" );
			TakeCash( ply, 450 );		
			
		end	
		
	end
	
