--##############################
--## Hide and Seek
--## fayte, gdhughes@gmail.com
--##############################

function DrawScores( userid )

	_GModText_Start( "ChatFont" );
	_GModText_SetPos( 0.0165, 0.0165 );
	_GModText_SetColor( 255, 255, 255, 255 );
	_GModText_SetTime( 99999, 0, 0 );
	_GModText_SetText( "Hide and Seek     Round " .. HSRoundNum .. "     Press F2 for team selection     Hiders: " .. _TeamScore( TEAM_HIDERS ) .. " points     Seekers: " .. _TeamScore( TEAM_SEEKERS ) .. " points" );
	_GModText_SetDelay( 0.01 );
	_GModText_Send( userid, 5 );

end

function MakeHiderVisible( userid ) 

	if ( _PlayerInfo( userid, "team" ) ~= TEAM_HIDERS ) then return; end
	if ( _PlayerInfo( userid, "alive" ) ~= true ) then return; end

	_EntFire( userid, "Alpha", 30, 0 );

	_PrintMessage( userid, 3, "You are now visible - all Seekers can see you." );

	_PlayerGiveSWEP( userid, "weapons/hideandseek/weapon_melongun.lua" );
	_PlayerGiveSWEP( userid, "weapons/hideandseek/weapon_babygun.lua" );
	_PlayerGiveItem( userid, "weapon_physcannon" );

	AddTimer( _TeamNumPlayers( TEAM_HIDERS ) * HIDER_SCORE_MODIFIER, 1, IncreaseHiderScore, userid );

end

function IncreaseHiderScore( userid ) 

	if ( _PlayerInfo( userid, "team" ) ~= TEAM_HIDERS ) then return; end
	if ( _PlayerInfo( userid, "alive" ) ~= true ) then return; end

	if ( _TeamNumPlayers( TEAM_SEEKERS ) > 0 ) then _TeamAddScore( 2, 1 ); end

	AddTimer( _TeamNumPlayers( TEAM_HIDERS ) * HIDER_SCORE_MODIFIER, 1, IncreaseHiderScore, userid );
	
	DrawScores( 0 );

end

function TeamSelect( userid, team, seconds )

	_GModRect_Hide( userid, 0, 1.0, 0 );
	_GModText_Hide( userid, 1, 0.5, 0 );
	_GModText_Hide( userid, 2, 0.5, 0 );

	if ( HSIsFound[userid] ) then
		_PrintMessage( userid, 3, "Please wait for the next round before you change your team.");
		return;
	end

	if ( team == 1 ) then

		if ( _PlayerInfo( userid, "team" ) == TEAM_HIDERS ) then return; end

		_PlayerChangeTeam( userid, TEAM_HIDERS );
		_EntSetKeyValue( userid, "rendermode", 1 );
		_EntSpawn( userid );

	elseif ( team == 2 ) then

		if ( _PlayerInfo( userid, "team" ) == TEAM_SEEKERS ) then return; end

		_PlayerChangeTeam( userid, TEAM_SEEKERS );
		_EntSetKeyValue( userid, "rendermode", 1 );
		_EntSpawn( userid );

	end

end