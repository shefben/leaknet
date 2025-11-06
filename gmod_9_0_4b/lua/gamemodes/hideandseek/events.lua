--##############################
--## Hide and Seek
--## fayte, gdhughes@gmail.com
--##############################

function gamerulesStartMap( )

	_TeamSetName( TEAM_HIDERS, "Hiders" );
	_TeamSetName( TEAM_SEEKERS, "Seekers" );
	_TeamSetName( TEAM_SPECTATOR, "Spectators" );

end

function gamerulesThink( )

	if ( _TeamScore( TEAM_HIDERS ) > 49 ) then

		_PrintMessageAll( 3, "Time is up - the Hiders won that round!" );

		_TeamSetScore( TEAM_HIDERS, 0 );
		_TeamSetScore( TEAM_SEEKERS, 0 );

		HSIsFound = {}
		HSNumFound = 0
		HSRoundNum = HSRoundNum + 1 

		for userid=0, _MaxPlayers() do
			_PlayerChangeTeam( userid, TEAM_SPECTATOR );
			_EntSpawn( userid );
		end

	elseif ( ( _TeamNumPlayers( TEAM_HIDERS ) < 1) and ( HSNumFound > 0 ) ) then

		_PrintMessageAll( 3, "All Hiders have been found. A new round begins in a few seconds." );

		if ( _TeamScore( TEAM_HIDERS ) > _TeamScore( TEAM_SEEKERS ) ) then
			_PrintMessageAll( 3, "The Hiders won that round with " .. _TeamScore( TEAM_HIDERS ) .. " points! " );
		elseif ( _TeamScore( TEAM_HIDERS ) < _TeamScore( TEAM_SEEKERS ) ) then
			_PrintMessageAll( 3, "The Seekers won that round with " .. _TeamScore( TEAM_SEEKERS ) .. " points!" );		
		else
			_PrintMessageAll( 3, "Both teams tied with " .. _TeamScore( TEAM_HIDERS ) .. " points!" );		
		end

		_TeamSetScore( TEAM_HIDERS, 0 );
		_TeamSetScore( TEAM_SEEKERS, 0 );

		HSIsFound = {}
		HSNumFound = 0
		HSRoundNum = HSRoundNum + 1 

		for userid=0, _MaxPlayers() do
			_PlayerChangeTeam( userid, TEAM_SPECTATOR );
			_EntSpawn( userid );
		end

	end

	for userid=0, _MaxPlayers() do
		if ( _PlayerInfo( userid, "connected" ) and ( ( _PlayerInfo( userid, "team") == TEAM_HIDERS ) or ( _PlayerInfo( userid, "team") == TEAM_SEEKERS ) ) and ( _PlayerInfo( userid, "alive") ) and ( _GetCurrentMap() == "gm_hideandseek" ) ) then
			local vecpos = _PlayerGetShootPos( userid );
			if ( vecpos.z < -500 ) then
				_PrintMessage( userid, 3, "Sorry, can't let you down there, it's too cold!");
				_PlayerSilentKill( userid, 1, true );
			end
		end
	end 
	
end

function eventPlayerSay( userid, text, team )

	if ( _PlayerInfo( userid, "team" ) == TEAM_SPECTATOR ) then
		for listid=1, _MaxPlayers() do
			if ( _PlayerInfo( listid, "team" ) == TEAM_SPECTATOR ) then
				_PrintMessage( listid, 3, "*SPEC* " .. text );
			end
		end
		return ""
	end

	return text

end

function eventPlayerSpawn( userid )

	if ( HSIsFound[userid] ) then

		if ( _TeamNumPlayers( TEAM_HIDERS ) > 1 ) then
			_PrintMessage( userid, 3, "You are spectating. Please wait until the round ends." );
		end

		_PlayerChangeTeam( userid, TEAM_SPECTATOR );
		_PlayerSpectatorStart( userid, 1 );

	elseif ( _PlayerInfo( userid, "team" ) == TEAM_SPECTATOR ) then

		onShowTeam( userid );

	elseif ( _PlayerInfo( userid, "team" ) == TEAM_SEEKERS ) then

		_PrintMessage( userid, 3, "You have spawned as a Seeker." );
		_EntFire( userid, "Alpha", 255, 0 );
		_PlayerGod ( userid, true );
		_PlayerSetSprint( userid, false );

		AddTimer( 0.01, 1, _PlayerGiveSWEP, userid, "weapons/hideandseek/weapon_supercatcher.lua" );
		AddTimer( 0.02, 1, _PlayerGiveSWEP, userid, "weapons/hideandseek/weapon_catcher.lua" );

	else

		_PrintMessage( userid, 3, "You have spawned as a Hider. Hide for as long as possible." );
		_PrintMessage( userid, 3, "You have been granted invisibility for 30 seconds. Use this time to hide!" );
		_EntSetMaterial( userid, "models/props_c17/fisheyelens" );
		_EntFire( userid, "Alpha", 0, 0 );
		_PlayerGod ( userid, true );
		_PlayerSetSprint( userid, true );
		AddTimer( 30, 1, MakeHiderVisible, userid );

	end
	
	DrawScores( userid );

end


function eventPlayerActive( name, userid, steamid )

	DrawScores( userid );

end

function GiveDefaultItems( userid )

	if ( _PlayerInfo( userid, "team" ) == TEAM_SEEKERS ) then
		_PlayerGiveItem( userid, "weapon_physcannon" );
	end

end

function onShowTeam( userid )

	if HSIsFound[userid] then 
		_PrintMessage( userid, 3, "Please wait for the next round before you change your team." );
		return;
	end

	_PlayerOption( userid, "TeamSelect", 99999 );

	_GModRect_Start( "gmod/white" );
	_GModRect_SetPos( 0, 0.2, 0.3, 0.27 );
	_GModRect_SetColor( 255, 255, 255, 75 );
	_GModRect_SetTime( 99999, 0.3, 0.3 );
	_GModRect_SetDelay( 0.5 );
	_GModRect_Send( userid, 0 );

	_GModText_Start( "ChatFont" );
	_GModText_SetPos( 0.016, 0.3 );
	_GModText_SetColor( 255, 255, 255, 255 );
	_GModText_SetTime( 99999, 0.3, 0.3 );
	_GModText_SetText( "Select your team:\n\n1. Hiders\n2. Seekers\n\n(Press 3 to close)" );
	_GModText_SetDelay( 0.5 );	
	_GModText_Send( userid, 1 );

	_GModText_Start( "ImpactMassive" );
	_GModText_SetPos( 0.016, 0.12 );
	_GModText_SetColor( 255, 0, 0, 205 );
	_GModText_SetTime( 99999, 1, 1 );
	_GModText_SetText( "Hide and Seek" );
	_GModText_SetDelay( 0.5 );
	_GModText_AllowOffscreen( true );
	_GModText_Send( userid, 2 );

	_GModText_Start( "ImpactMassive" );
	_GModText_SetPos( 0.016, 0.22  );
	_GModText_SetColor( 255, 0, 0, 205 );
	_GModText_SetDelay( 0 );
	_GModText_SendAnimate( userid, 2, 1.5, 0.7 );

end

function onShowHelp( userid )

	_GModText_Start( "HudHintTextLarge" );
	_GModText_SetPos( -1, 0.6 );
	_GModText_SetColor( 255, 255, 255, 255 );
	_GModText_SetTime( 5, 0.3, 0.3 );
	_GModText_SetText( "Hide and Seek Help\n\nHiders - stay hidden for as long as possible.\n Seekers - find Hiders by shooting them with the Catcher Gun.\n\nGamemode made by fayte\ngm_hideandseek_outland map made by MrSteak63\ngm_hideandseek_yard map made by TrickyClock" );
	_GModText_SetDelay( 0 );	
	_GModText_Send( userid, 8 );

end

function PickDefaultSpawnTeam( userid )

	_PlayerChangeTeam( userid, TEAM_SPECTATOR );
	return true; 

end