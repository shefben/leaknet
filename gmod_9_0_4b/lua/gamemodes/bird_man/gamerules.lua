

	function gamerulesThink ()

		if (_CurTime() >= roundEndTime) then
		
			if (roundEndFunc ~= nil) then
			
				roundEndFunc()
			
			end
		
		end
		
	end 

	function PickDefaultSpawnTeam(userid)		
		_PlayerChangeTeam(userid, TEAM_SPECTATOR);
		return true;
	end

	function GiveDefaultItems( playerid )
		_PlayerGiveItem( playerid, "weapon_physcannon" )
		_PlayerGiveItem( playerid, "weapon_physgun" )
		_PlayerGiveItem( playerid, "weapon_tool" )
	end
	
	
	function ShowWall()
		if (wallEnabled == 1) then
			return
		end
		
		local wall = _EntGetByName("Wall")
		if (wall ~= 0) then
			_EntFire(wall, "Toggle", 0, 0)
		end
		wallEnabled = 1
	end
	
	function HideWall()
		if (wallEnabled == 0) then
			return
		end
		
		local wall = _EntGetByName("Wall")
		if (wall ~= 0) then
			_EntFire(wall, "Toggle", 0, 0)
		end
		wallEnabled = 0
	end
	
	function timeUpdater()
		local timeDiff = roundEndTime - _CurTime()
		if (timeDiff < 0) then
			timeDiff = 0
		end
		local time = "Time left: " .. ToMinutesSeconds(timeDiff)
		
		_GModRect_Start( "gmod/white" );
		_GModRect_SetPos( 0.035, 0.72, 0.18, 0.06 );
		_GModRect_SetColor( 0, 0, 0, 150 );
		_GModRect_SetTime( 99999, 0, 0 );
		_GModRect_SetDelay( 0 );
		_GModRect_Send( 0, 2 );

		_GModText_Start( "Default" );
		_GModText_SetColor( 255, 0, 0, 255 );
		_GModText_SetTime( 2, 0, 0 );
		_GModText_SetPos( 0.05, 0.74 )
		_GModText_SetText( time );
		_GModText_Send( 0, 12 );
	end
	
	function BuildMode()
		_GModText_Start( "Default" );
		_GModText_SetColor( 255, 0, 0, 255 );
		_GModText_SetTime( ROUND_TIME, 0, 0 );
		_GModText_SetPos( -1, 0.2 )
		_GModText_SetText( "Build!" );
		_GModText_Send( 0, 11 );

		roundStartTime = _CurTime()
		roundEndTime = _CurTime() + ROUND_TIME
		roundEndFunc = StartFlyTestMode
		local rtChanged = roundType ~= RT_BUILD
		roundType = RT_BUILD
		
		ShowWall()
		
		if (rtChanged) then
			PlayerSpawnAll()
		end
	end
	
	function restartRound()
		if (roundType == RT_BUILD) then			BuildMode()
		elseif (roundType == RT_FLYTEST) then		StartFlyTestMode()
		elseif (roundType == RT_DEATHMATCH) then	BeginDeathMatchMode()
		end
		
	end

	function StartFlyTestMode()
		_GModText_Start( "Default" );
		_GModText_SetColor( 255, 0, 0, 255 );
		_GModText_SetTime( ROUND_TIME, 0, 0 );
		_GModText_SetPos( -1, 0.2 )
		_GModText_SetText( "Fly my monkies, FLYYYYYY!" );
		_GModText_Send( 0, 11 );

		roundStartTime = _CurTime()
		roundEndTime = _CurTime() + ROUND_TIME
		roundEndFunc = BuildMode
		roundType = RT_FLYTEST
		
		HideWall()
	end
	
	function EndDeathMatchMode()
	
		_ServerCommand("gm_sv_playerdamage 0\n")
		_ServerCommand("gm_sv_pvpdamage 0\n")
		_ServerCommand("gm_sv_teamdamage 0\n")
		_ServerCommand("gm_sv_setrules\n")
	
		for i=1, _MaxPlayers() do
			_PlayerSilentKill(i)
		end
	
		BuildMode()
	end

	function BeginDeathMatchMode()
		_GModText_Start( "Default" );
		_GModText_SetColor( 255, 0, 0, 255 );
		_GModText_SetTime( ROUND_TIME, 0, 0 );
		_GModText_SetPos( -1, 0.2 )
		_GModText_SetText( "Kill each other!!!" );
		_GModText_Send( 0, 11 );
		
		roundStartTime = _CurTime()
		roundEndTime = _CurTime() + ROUND_TIME
		roundEndFunc = EndDeathMatchMode
		local rtChanged = roundType ~= RT_DEATHMATCH
		roundType = RT_DEATHMATCH
		
		if (rtChanged) then
			_ServerCommand("gm_sv_playerdamage 1\n")
			_ServerCommand("gm_sv_pvpdamage 1\n")
			_ServerCommand("gm_sv_teamdamage 1\n")
			_ServerCommand("gm_sv_setrules\n")

			for i=1, _MaxPlayers() do
				_PlayerRemoveAllWeapons( i )
				_PlayerGiveItem( i, "weapon_shotgun" )
				_PlayerGiveItem( i, "weapon_smg1" )
				_PlayerGiveAmmo( i, 255, "Buckshot", false )	
				_PlayerGiveAmmo( i, 255, "SMG1", false )
			end
		end
		
		HideWall()
	end
	
	function SetNextRoundDeathMatch()
		roundEndFunc = BeginDeathMatchMode
	end
	
	function canPlayerHaveItem( playerid, itemname )
	
		if ((	itemname == "weapon_physcannon" or 
			itemname == "weapon_physgun" or
			itemname == "weapon_Tool") and roundType == RT_DEATHMATCH) then
			
			return false
		
		elseif ((itemname ~= "weapon_physcannon" and 
			itemname ~= "weapon_physgun" and
			itemname ~= "weapon_Tool") and roundType ~= RT_DEATHMATCH) then
			
			return false
			
		end

		return true

	end

	function gamerulesStartMap ()

		bEndGame = false;

		fIntermissionEnd = 0;

		PlayerFreezeAll( false );

		-- Set the default team names

		_TeamSetName( TEAM_BLUE, "Blue Team" );
		_TeamSetName( TEAM_GREEN, "Green Team" );
		_TeamSetName( TEAM_YELLOW, "Yellow Team" );
		_TeamSetName( TEAM_RED, "Red Team" );
		
		timeUpdater = AddTimer(0.5, 0, timeUpdater)
		
		BuildMode()

	end

	function PlayerSpawnChooseModel ( playerid )	

		if ( _PlayerInfo( playerid, "model" ) == DEFAULT_PLAYER_MODEL ) then

			if ( _PlayerPreferredModel( playerid ) == "" ) then

				_PlayerSetModel( playerid, _PlayerGetRandomAllowedModel() )

			else

				_PlayerSetModel( playerid, _PlayerPreferredModel( playerid ) )

			end

		end

	end
