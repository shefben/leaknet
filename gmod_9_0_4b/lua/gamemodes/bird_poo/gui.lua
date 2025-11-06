--[[

GmodText
------------------------------------
0 .. _MaxPlayers() : Player Name
19: Press 5 to auto select team
20: Press 4 to select random bird
21: Team Blue Name
22: Team Blue Score
23: Team Yellow Name
24: Team Yellow Score
50: You got shat on by X
51: You shat on X
52: You were killed by X
53: You killed X
54: Menu
55: Version
56: Player Run Count

GmodRect
------------------------------------
0: Intro Text
1: Intro Logo 
2: White background
3: Select Team 1
4: Select Team 2
5: White background
6: Choose Bird Seagull
7: Choose Bird Crow
8: Choose Bird Pigeon
9: Score board
75+: HUD Poo

]]

function UpdatePlayerLabels()

	for i=1, _MaxPlayers() do

		if ( _PlayerInfo( i, "connected" ) and PlayerData[i] ~= nil ) then

			_GModText_Start( "BrandingSmall" );
			 _GModText_SetColor( 255, 255, 255, 255 );
			 _GModText_SetTime( 999, 0, 5 );
			 _GModText_SetEntityOffset( vector3( 0, 0, 16 ) );
			 _GModText_SetEntity( PlayerData[i].Bird );
			 _GModText_SetText( _PlayerInfo( i, "name" ) );
			_GModText_Send( 0, i );

		else

			_GModText_Hide( 0, i, 0, 0 );

		end

	end

end

function UndrawRunLeft(PlayerID)
	_GModText_Hide( PlayerID, 56 );

	local bid = 10
	for i = 0, MAX_RUN-1, 5 do
		_GModRect_Hide( PlayerID, bid );
		bid = bid + 1
	end
end

function DrawRunLeft( PlayerID )

	_GModText_Start( "Default" );
	_GModText_SetPos( 0.05, 0.5 );
	_GModText_SetTime( 99999, 0, 0 );
	_GModText_SetColor( 255, 255, 255, 255 );
	_GModText_SetText( "Sprint: " );
	_GModText_Send( PlayerID, 56 );

	local x_pos = 0.02
	local bid = 10

	for i = 0, MAX_RUN-1, 5 do

		_GModRect_Start( "gmod/white" );
		_GModRect_SetPos( x_pos, 0.55, 0.01, 0.01 );

		if (i >= PlayerData[PlayerID].RunLeft) then
			 _GModRect_SetColor( 192, 192, 192, 255 );
		else
			if (PlayerData[PlayerID].RunLeft > 35) then
				_GModRect_SetColor( 0, 255, 0, 255 );
			elseif (PlayerData[PlayerID].RunLeft > 10) then
				_GModRect_SetColor( 255, 255, 0, 255 );
			else
				_GModRect_SetColor( 255, 0, 0, 255 );
			end
		end	

		_GModRect_SetTime( 99999, 0, 0 );
		_GModRect_Send( PlayerID, bid );

		bid = bid + 1
		x_pos = x_pos + 0.011
	end
end

function DrawScores( PlayerID )
	-- Blue Team
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.05, 0.06 );
	 _GModText_SetColor( 40, 70, 140, 255 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "The Birds: " );
	_GModText_Send( PlayerID, 21 );

	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.18, 0.06 );
	 _GModText_SetColor( 40, 70, 140, 255 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( _TeamScore(TEAM_BIRDS) );
	_GModText_Send( PlayerID, 22 );


	-- Yellow Team
	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.05, 0.09 );
	 _GModText_SetColor( 210, 160, 0, 255 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( "The People: " );
	_GModText_Send( PlayerID, 23 );

	_GModText_Start( "Default" );
	 _GModText_SetPos( 0.18, 0.09 );
	 _GModText_SetColor( 210, 160, 0, 255 );
	 _GModText_SetTime( 99999, 0, 0 );
	 _GModText_SetText( _TeamScore(TEAM_HUMANS) );
	_GModText_Send( PlayerID, 24 );


	-- Background Rect
	_GModRect_Start( "gmod/bird_poo/scores" );
	 _GModRect_SetPos( 0.02, 0.02, 0.20, 0.24 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	 _GModRect_SetTime( 99999, 0, 0 );
	_GModRect_Send( PlayerID, 9 );
end

function DrawIntro( PlayerID )

	_GModRect_Start( "gmod/bird_poo/text" );
	 _GModRect_SetPos( 0.1, 0.1, 0.6, 0.3 );
	 _GModRect_SetColor( 255, 255, 255, 0 );
	 _GModRect_SetTime( 4.0, 0, 0.5 );
	_GModRect_Send( PlayerID, 0 );

	_GModRect_Start( "" );
	 _GModRect_SetPos( 0.1, 0.1, 0.6, 0.3 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	_GModRect_SendAnimate( PlayerID, 0, 2, 0.3 );

	_GModRect_Start( "gmod/bird_poo/title" );
	 _GModRect_SetPos( 0.5, 0.5, 0.5, 0.5 );
	 _GModRect_SetColor( 255, 255, 255, 0 );
	 _GModRect_SetTime( 4.0, 0, 0.5 );
	_GModRect_Send( PlayerID, 1 );

	_GModRect_Start( "" );
	 _GModRect_SetPos( 0.5, 0.5, 0.5, 0.5 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	_GModRect_SendAnimate( PlayerID, 1, 2, 0.3 );

	_GModText_Start( "ImpactMassive" );
	 _GModText_SetPos( -1, 0.8 );
	 _GModText_SetColor( 0, 0, 0, 255 );
	 _GModText_SetTime( 4.0, 0, 0.5 );
	 _GModText_SetText( VERSION_TEXT );
	 _GModText_SetDelay( 1 );
	_GModText_Send( PlayerID, 55 );

	DrawScores( PlayerID )

end

function ChooseBirdType( playerid, num, seconds )

	_GModRect_Hide( playerid, 5, 0.5 );
	_GModRect_Hide( playerid, 6, 0.5 );
	_GModRect_Hide( playerid, 7, 0.5 );
	_GModRect_Hide( playerid, 8, 0.5 );

	_GModText_Hide( playerid, 20, 0.3 );

	local oldBirdType = PlayerData[playerid].BirdType

	if (num == 1) then	PlayerData[playerid].BirdType = BT_SEAGULL	
	elseif (num == 2) then	PlayerData[playerid].BirdType = BT_CROW	
	elseif (num == 3) then	PlayerData[playerid].BirdType = BT_PIGEON	
	else
		PlayerData[playerid].BirdType = math.floor( math.random() * 3 ) + 1
	end

	-- catch just in case!
	if (	PlayerData[playerid].BirdType ~= BT_SEAGULL and 
		PlayerData[playerid].BirdType ~= BT_CROW and
		PlayerData[playerid].BirdType ~= BT_PIGEON) then

		PlayerData[playerid].BirdType = BT_SEAGULL
	end

	-- nothing to change!
	if (PlayerData[playerid].BirdType == oldBirdType and _PlayerInfo( playerid, "team" ) == TEAM_BIRDS) then
		return
	end

	if (PlayerData[playerid].BirdType == BT_SEAGULL) then		_GModRect_Hide( playerid, 6, 1.5 );
	elseif (PlayerData[playerid].BirdType == BT_CROW) then		_GModRect_Hide( playerid, 7, 1.5 );
	elseif (PlayerData[playerid].BirdType == BT_PIGEON) then	_GModRect_Hide( playerid, 8, 1.5 );
	end

	_PlaySoundPlayer( playerid, "ambient/creatures/seagull_idle1.wav" );
	_PlayerChangeTeam( playerid, TEAM_BIRDS );
	AddTimer( 2, 1, _EntSpawn, playerid );
	_PlayerSilentKill( playerid );

end

function onChangeBirdType(playerid)
	_GModRect_Hide( playerid, 2, 0.0 );
	_GModRect_Hide( playerid, 3, 0.0 );
	_GModRect_Hide( playerid, 4, 0.0 );
	_GModText_Hide( playerid, 19, 0.0 );

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0, 0, 1, 1 );
	 _GModRect_SetColor( 255, 255, 255, 50 );
	 _GModRect_SetTime( 99999, 0.5, 3 );
	_GModRect_Send( playerid, 5 );

	_GModRect_Start( "gmod/bird_poo/seagull" );
	 _GModRect_SetPos( 0.0, 0.25, 0.30, 0.30 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	 _GModRect_SetTime( 99999, 0.5, 3 );
	_GModRect_Send( playerid, 6 );

	_GModRect_Start( "gmod/bird_poo/crow" );
	 _GModRect_SetPos( 0.35, 0.25, 0.30, 0.30 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	 _GModRect_SetTime( 99999, 0.5, 3 );
	_GModRect_Send( playerid, 7 );

	_GModRect_Start( "gmod/bird_poo/pigeon" );
	 _GModRect_SetPos( 0.70, 0.25, 0.30, 0.30 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	 _GModRect_SetTime( 99999, 0.5, 3 );
	_GModRect_Send( playerid, 8 );

	_GModText_Start( "Default" );
	 _GModText_SetPos( -1, 0.8 );
	 _GModText_SetColor( 0, 0, 0, 255 );
	 _GModText_SetTime( 99999, 1, 1 );
	 _GModText_SetText( "Or press '4' to randomly select a bird!" );
	 _GModText_SetDelay( 1 );
	_GModText_Send( playerid, 20 );

	-- I know this 99999 stuff sucks.. but so do I.
	_PlayerOption( playerid, "ChooseBirdType", 99999 );
end


function ChooseTeam( playerid, num, seconds )

	_GModRect_Hide( playerid, 2, 1.0 );
	_GModRect_Hide( playerid, 3, 0.5 );
	_GModRect_Hide( playerid, 4, 0.5 );

	_GModText_Hide( playerid, 19, 0.3 );

	if (num == 1) then

		onChangeBirdType(playerid)

		return;
	end

	if (num == 2) then

		if ( _PlayerInfo( playerid, "team" ) == TEAM_HUMANS ) then return; end;

		_PlaySoundPlayer( playerid, "vo/npc/male01/okimready01.wav" );
		_PlayerChangeTeam( playerid, TEAM_HUMANS );
		AddTimer( 2, 1, _EntSpawn, playerid );

		_PlayerSilentKill( playerid );
		_GModRect_Hide( playerid, 4, 1.5 );


		return;
	end

	-- anything else is auto choose team

	if ( _TeamNumPlayers( TEAM_HUMANS ) > _TeamNumPlayers( TEAM_BIRDS ) ) then

		ChooseTeam( playerid, 1, 0 );

	else

		ChooseTeam( playerid, 2, 0 );

	end

end


-- These are called by the players in game using the F1 - F4 keys

function onShowHelp ( userid )
	_GModText_Start( "Default" );
	 _GModText_SetPos( -1, 0.3 );
	 _GModText_SetColor( 255, 255, 255, 255 );
	 _GModText_SetTime( 4, 0.2, 1 );
	 _GModText_SetText( "The object of Bird Poo is to control your\nbird to poop on the humans! Duh!\n\nPress F3 for the menu!" );
	_GModText_Send( userid, 50 );

end

function onShowTeam ( userid )

	_GModRect_Hide( userid, 5, 0 );
	_GModRect_Hide( userid, 6, 0 );
	_GModRect_Hide( userid, 7, 0 );
	_GModRect_Hide( userid, 8, 0 );
	_GModText_Hide( userid, 20, 0 );	

	ClearPlayerData( userid, 0 )

	_GModRect_Start( "gmod/white" );
	 _GModRect_SetPos( 0, 0, 1, 1 );
	 _GModRect_SetColor( 255, 255, 255, 50 );
	 _GModRect_SetTime( 99999, 0.5, 3 );
	_GModRect_Send( userid, 2 );

	_GModRect_Start( "gmod/bird_poo/team1" );
	 _GModRect_SetPos( 0.10, 0.25, 0.35, 0.35 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	 _GModRect_SetTime( 99999, 0.5, 3 );
	_GModRect_Send( userid, 3 );

	_GModRect_Start( "gmod/bird_poo/team2" );
	 _GModRect_SetPos( 0.55, 0.25, 0.35, 0.35 );
	 _GModRect_SetColor( 255, 255, 255, 255 );
	 _GModRect_SetTime( 99999, 0.5, 3 );
	_GModRect_Send( userid, 4 );

	-- Options
	_GModText_Start( "Default" );
	 _GModText_SetPos( -1, 0.8 );
	 _GModText_SetColor( 0, 0, 0, 255 );
	 _GModText_SetTime( 99999, 1, 1 );
	 _GModText_SetText( "Or press '5' to automatically join the best team." );
	 _GModText_SetDelay( 1 );
	_GModText_Send( userid, 19 );

	-- I know this 99999 stuff sucks.. but so do I.
	_PlayerOption( userid, "ChooseTeam", 99999 );

end

function onShowSpare1 ( userid )
	cc_menu(userid)
end

