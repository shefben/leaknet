----------------------------------------------------------------------------------
-- 9/10ths Lua script
-- By Garry Newman
----------------------------------------------------------------------------------

-- Settings

NINET_ROUND_LENGTH = 5 * 60; -- in seconds




fIntermissionEnd = 0
bEndGame = false

ItemCount = {} -- This stores the number of props in each team's triggers
ItemCount[1] = 0;
ItemCount[2] = 0;
iRoundTimer = -1;
fRoundEnd = -1;
RespawnableProps = {}; -- a table to hold all the props so we can respawn them at the start of the round





--  Called every frame from: CHL2MPRules::Think( void ) -----------------------


	function gamerulesThink ()
	
		-- gameover is true when the game has ended and everyone
		-- is looking at the scoreboard blaming lag for their score
		
		if ( bEndGame ) then
			
			if ( fIntermissionEnd < _CurTime() ) then
				
				_StartNextLevel()
				
			end
						
			return
			
		end;
			
		
		local TimeLimit = _GetConVar_Float( "mp_timelimit" ) * 60 -- Minutes to seconds!
		local FragLimit = _GetConVar_Float( "mp_fraglimit" )
		
		if (FragLimit > 0) then -- We have a fraglimit!
		
			if ( _GetRule( "Teamplay" ) ) then
				
				local NumTeams = _TeamCount();
				
				for i=0, NumTeams do
					
					if ( _TeamScore(i) >= FragLimit ) then
					
						StartIntermission()
					
					end
					
				end
		
			else -- not teamplay
		
				for i=0, _MaxPlayers() do
				
					if ( _PlayerInfo( i, "connected" ) and  _PlayerInfo( i, "kills" ) >= FragLimit ) then
						
						StartIntermission()
						
					end
				
				end
		
			end
			
		end
	 
	 

		if ( TimeLimit > 0 ) then
			
			if ( TimeLimit < _CurTime() ) then
			
				StartIntermission()
				
			end;
			
		end;
		
		
	
	end --gamerulesThink
	
	
	
	
	
-- Give the players the default weapons --

	function GiveDefaultItems( playerid )
		
		if ( _PlayerInfo( playerid, "team" ) == TEAM_SPECTATOR ) then return; end
		
		_PlayerGiveItem( playerid, "weapon_crowbar" )
		--_PlayerGiveItem( playerid, "weapon_pistol" )
		_PlayerGiveItem( playerid, "weapon_physcannon" )
				
	end
	

--  The current map has ended, show the scoreboard ----------------------------

	
	function StartIntermission ()
	

		bEndGame = true;
		
		fIntermissionEnd = _CurTime() + _GetConVar_Float( "mp_chattime" )
		
		-- Loop through all players
		for i=0, _MaxPlayers() do
		
			_PlayerShowScoreboard( i )
			_PlayerFreeze( i, true )
		
		end
	
	end
	
	
	
--  Called right before the new map starts ------------------------------------

	          
	function gamerulesStartMap ()
	
		-- Anything to imitialize?	
		bEndGame = false
		fIntermissionEnd = 0


		RoundRestart();
		
		-- Give the props time to settle before we capture their
		-- models, location and positions.
		AddTimer( 3, 1, StorePropList, 0 );
			
	end
	
	
	function StorePropList( )
		
		local ents = _EntitiesFindByClass( "prop_physics" );
		table.foreach( ents, AddRespawnableProp );
		
	end
	
	function AddRespawnableProp( idx, key )
		
		RespawnableProps[ idx ] = {};
		RespawnableProps[ idx ].model = _EntGetModel( key );
		RespawnableProps[ idx ].pos = _EntGetPos( key );
		RespawnableProps[ idx ].ang = _EntGetAngAngle( key );
		
	end
		
	function PropRespawn( idx, key )
			
		local iEnt = _EntCreate( "prop_physics" );
		
			_EntSetModel( iEnt, key.model ); -- the model will already be precached
			_EntSetPos( iEnt, key.pos );
			_EntSetAngAngle( iEnt, key.ang );
			
		_EntSpawn( iEnt );
		
	end
	
	function PropDelete( idx, key )
		
		_EntRemove( key );
		
	end
	
	function RespawnStoredProps()
		
		-- Print a list of respawn props
		--tprint( RespawnableProps );
		
		if ( table.getn(RespawnableProps) == 0 ) then return; end
		
		-- Remove all current props
		local CurrentEnts = _EntitiesFindByClass( "prop_physics" );
		table.foreach( CurrentEnts, PropDelete );
		
		-- Spawn all the new props
		table.foreach( RespawnableProps, PropRespawn );
		
	end
	
	
	function RoundRestart( )
	
		RespawnStoredProps();
	
		-- Set the default team names
		_TeamSetName( TEAM_BLUE, "Blue Team" );
		_TeamSetName( TEAM_YELLOW, "Yellow Team" );
			
		-- Unfreeze everyone
		PlayerFreezeAll( false );
		
		-- respawn all players
		PlayerSpawnAll( );
		
		fRoundEnd = _CurTime() + NINET_ROUND_LENGTH;		
		if (iRoundTimer > -1) then HaltTimer( iRoundTimer ) end;
		iRoundTimer = AddTimer( 1, NINET_ROUND_LENGTH, doRoundTimer );
		
		-- Reset indivdual scores to 0
		for i=1, _MaxPlayers() do
			_PlayerSetScore( i, 0 )
		end

	end
	
	-- called every second
	function doRoundTimer( )
		
		local iTimeLeft = fRoundEnd - _CurTime();
		if (iTimeLeft<0) then iTimeLeft = 0; end;
		
		local iMinutes = iTimeLeft / 60.0;
		iMinutes = math.floor( iMinutes );
		
		local iSeconds = math.mod( iTimeLeft, 60 );
		iSeconds = math.floor( iSeconds );
		if (iSeconds < 10) then iSeconds = "0" .. iSeconds; end; -- Proper way to do this?
		
		local TimerText = iMinutes .. ":" .. iSeconds;
		
		
		_GModRect_Start( "gmod/gm_910/time" );
		 _GModRect_SetPos( 0.02, 0.16, 0.20, 0.12 );
		 _GModRect_SetColor( 255, 255, 255, 255 );
		 _GModRect_SetTime( 99999, 0, 0 );
		_GModRect_Send( userid, 20 );
		
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.15, 0.195 );
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( TimerText );
		_GModText_Send( 0, 20 );
		
		
		if (iTimeLeft < 10 and iTimeLeft > 0 ) then
			
			_GModText_Start( "DefaultShadow" );
			 _GModText_SetPos( 0.15, 0.195 );
			 _GModText_SetColor( 255, 0, 0, 255 );
			 _GModText_SetTime( 0, 0, 1 );
			 _GModText_SetText( TimerText );
			_GModText_Send( 0, 21 );
			
			_PlaySound( "ambient/alarms/klaxon1.wav" );
			
		end
		
		-- Game Over!
		if (iTimeLeft == 0) then
			
			
			_PlaySound("ambient/voices/playground_memory.wav");
			
			AddTimer( 1, 1, PlayerFreezeAll, true );
			
			-- show winner.. 

			-- todo: use textures for these to make it cooler
			_GModText_Start( "ImpactMassive" );
			 _GModText_SetPos( -1, -1 );
			 _GModText_SetTime( 6, 0, 1 );
			 
			 if ( ItemCount[ 1 ] > ItemCount[ 2 ] ) then
			 
			 _GModText_SetColor( 50, 90, 255, 255 );
			 _GModText_SetText( "Blue Team Wins!" );
			 _TeamAddScore( TEAM_BLUE, 1 )
			 
			 elseif ( ItemCount[ 1 ] < ItemCount[ 2 ] ) then
		
			 _GModText_SetColor( 255, 500, 2, 255 );
			 _GModText_SetText( "Yellow Team Wins!" );
			 _TeamAddScore( TEAM_YELLOW, 1 )	
			 	
			 else
			 	
			 _GModText_SetColor( 200, 200, 200, 255 );
			 _GModText_SetText( "Everyone Wins!" );	
			 _TeamAddScore( TEAM_BLUE, 1 )
			 _TeamAddScore( TEAM_YELLOW, 1 )	
			 	
			 end
			 
			_GModText_Send( 0, 31 );
			
		_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 0, 0, 1, 1 );
		 _GModRect_SetColor( 0, 0, 0, 100 );
		 _GModRect_SetTime( 6, 0.1, 1 );
		_GModRect_Send( 0, 30 );


			-- Schedule a round restart
			AddTimer( 7, 1, RoundRestart );

			-- Stop the round timer..
			--HaltTimer( iRoundTimer );
			--iRoundTimer = -1;
			
			
		end
		
		
		
	end


	function UpdateScores( UserID )

		-- Blue Team
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.05, 0.08 );
		 _GModText_SetColor( 50, 150, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Blue Team: " );
		_GModText_Send( UserID, 0 );
		
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.18, 0.08 );
		 _GModText_SetColor( 50, 150, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( ItemCount[ 1 ] );
		_GModText_Send( UserID, 1 );
		
		
		-- Yellow Team
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.05, 0.11 );
		 _GModText_SetColor( 255, 200, 0, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Yellow Team: " );
		_GModText_Send( UserID, 2 );
		
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.18, 0.11 );
		 _GModText_SetColor( 255, 200, 0, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( ItemCount[ 2 ] );
		_GModText_Send( UserID, 3 );
		
		
		-- Background Rect
		_GModRect_Start( "gmod/gm_910/scores" );
		 _GModRect_SetPos( 0.02, 0.02, 0.20, 0.24 );
		 _GModRect_SetColor( 255, 255, 255, 255 );
		 _GModRect_SetTime( 99999, 0, 0 );
		_GModRect_Send( UserID, 0 );
	
	end
	
	function ScoreChanged( iTeam, bIncreased )
	
		_GModText_Start( "Default" );
		_GModText_SetTime( 0.1, 0.1, 1 );
		
		if (bIncreased) then
			_GModText_SetColor( 255, 255, 255, 255 );
		else
			_GModText_SetColor( 255, 50, 20, 255 );
		end
	
		if (iTeam == 1) then
		
		--	_GModText_SetPos( 0.05, 0.08 );
		--	_GModText_SetText( "Blue Team: " );
		--	_GModText_Send( 0, 10 );
			
			_GModText_SetPos( 0.18, 0.08 );
			_GModText_SetText( ItemCount[ 1 ] );
			_GModText_Send( 0, 11 );
		
		else
		
			_GModText_SetPos( 0.05, 0.11 );
			_GModText_SetText( "Yellow Team: " );
			_GModText_Send( 0, 12 );
			
			_GModText_SetPos( 0.18, 0.11 );
			_GModText_SetText( ItemCount[ 2 ] );
			_GModText_Send( 0, 13 );
	
		end
	
	end
	
	function DrawIntroScreen( userid )
	
		-- Intro Logo
		_GModRect_Start( "gmod/gm_910/910" );
		 _GModRect_SetPos( 0.325, 0.3, 0.35, 0.4 );
		 _GModRect_SetColor( 255, 255, 255, 255 );
		 _GModRect_SetTime( 3.0, 0.0, 1.5 );
		_GModRect_Send( userid, 1 );
		
		-- Make them see the scores too (for the first time)
		UpdateScores( userid );
	
	end


-- This is very important, it will crash if a player doesn't have a model! --
	function PlayerSpawnChooseModel ( playerid )	
	
		if ( _PlayerInfo( playerid, "model" ) == DEFAULT_PLAYER_MODEL ) then
					
			-- The player doesn't have a preferred model, set it randomly
			if ( _PlayerPreferredModel( playerid ) == "" ) then
	
				_PlayerSetModel( playerid, _PlayerGetRandomAllowedModel() )
						
			-- The player has a preferred model, use that!		
			else
			
				_PlayerSetModel( playerid, _PlayerPreferredModel( playerid ) )
			
			end
			
		end
		
	end

-- Events --



	function eventPlayerKilled ( killed, attacker, weapon )
		
		
		_PlayerAddDeath( killed, 1 )
		
		
		-- Player killed himself, what a top hat
		if ( killed == attacker ) then
			
			_PlayerAddScore( killed, -1 )
			
		-- Was killed by another player!
		elseif ( attacker > 0 ) then
		
			_PlayerAddScore( attacker, 1 )
			_PlayerAllowDecalPaint( attacker ); -- Let the player spraypaint the body

		-- The soft sod was killed by the world
		else
		
			_PlayerAddScore( killed, -1 )
			
		end	
		
		
	end


	function eventPlayerSpawn ( userid )
		
		-- If they're a spectator show them the team choice menu
		if ( _PlayerInfo( userid, "team" ) == TEAM_SPECTATOR ) then
			
			DrawIntroScreen( userid );
			AddTimer( 4.5, 1, onShowTeam, userid );
			_EntSetPos( userid, vector3( 0, 0, 0) );

		end
		
		_PlayerSetDrawTeamCircle( userid, true )

	end
	
	
	function PickDefaultSpawnTeam( userid )
		
		_PlayerChangeTeam( userid, TEAM_SPECTATOR );
		return true;
		
	end
	
	
	
	
	-- The syntax for these functions should nearly always be the same
	-- 
	-- Activator 	- The entity that initially caused this chain of output events.
	-- Caller		- The entity that fired this particular output.
	-- The third parameter depends on which output the gmod_runfunction entity called
	

	function Reward( iPlayer )
		
		_PlayerAddScore( iPlayer, 1 );
		_PlaySoundPlayer( iPlayer, "hl1/fvox/bell.wav" );
		
	end
	
	function Punish( iPlayer )
		
		_PlayerAddScore( iPlayer, -1 );
		_PlaySoundPlayer( iPlayer, "hl1/fvox/buzz.wav" );
		
	end
	
	function onEntityTouch( Activator, Caller, Team )
			
		ItemCount[ Team ] = ItemCount[ Team ] + 1;
		UpdateScores(0);
		ScoreChanged( Team, true );
		
		-- Get the player that shot the entity..
		local iPlayer = _EntityGetPhysicsAttacker( Activator );
		
		if ( IsPlayer(iPlayer) == false ) then return; end;
			
		if ( _PlayerInfo( iPlayer, "team" ) == TEAM_BLUE ) then		
			if (Team == 1) then Reward(iPlayer);
			else Punish(iPlayer); end
		elseif ( _PlayerInfo( iPlayer, "team" ) == TEAM_YELLOW ) then
			if (Team == 2) then Reward(iPlayer);
			else Punish(iPlayer); end
		end	
	
	end
	
	
	function onEntityUntouch( Activator, Caller, Team )
			
		ItemCount[ Team ] = ItemCount[ Team ] - 1;
		UpdateScores(0);
		ScoreChanged( Team, false );
		
		local iPlayer = _EntityGetPhysicsAttacker( Activator );
		
		local iPlayer = _EntityGetPhysicsAttacker( Activator );
		
		if ( IsPlayer(iPlayer) == false ) then return; end;
			
		if ( _PlayerInfo( iPlayer, "team" ) == TEAM_BLUE ) then		
			if (Team == 1) then Punish(iPlayer);end
		elseif ( _PlayerInfo( iPlayer, "team" ) == TEAM_YELLOW ) then
			if (Team == 2) then Punish(iPlayer);end
		end
	
	end
	
	
	function ChooseTeam( playerid, num, seconds )
		
		_GModRect_Hide( playerid, 2, 1.0 );
		_GModRect_Hide( playerid, 3, 0.5 );
		_GModRect_Hide( playerid, 4, 0.5 );
		
		_GModText_Hide( playerid, 19, 0.3 );
		
		if (num == 1) then
			
			if ( _PlayerInfo( playerid, "team" ) == TEAM_BLUE ) then return; end;
			
			_PlaySoundPlayer( playerid, "hl1/fvox/activated.wav" );
			_PlayerChangeTeam( playerid, TEAM_BLUE );
			
			AddTimer( 2, 1, _EntSpawn, playerid );
			--_PlayerSilentKill( playerid );
			
			_GModRect_Start( "gmod/gm_910/team1" );
			 _GModRect_SetPos( 0.05, 0.15, 0.56, 0.75 );
			 _GModRect_SetColor( 255, 255, 255, 255 );
			_GModRect_SendAnimate( playerid, 3, 1.5, 0.8 );
			
			_GModRect_Hide( playerid, 3, 1.5 );
			
			return;
		end
		
		if (num == 2) then
			
			if ( _PlayerInfo( playerid, "team" ) == TEAM_YELLOW ) then return; end;
			
			_PlaySoundPlayer( playerid, "hl1/fvox/activated.wav" );
			_PlayerChangeTeam( playerid, TEAM_YELLOW );
			AddTimer( 2, 1, _EntSpawn, playerid );

			_GModRect_Start( "gmod/gm_910/team1" );
			 _GModRect_SetPos( 0.40, 0.15, 0.56, 0.75 );
			 _GModRect_SetColor( 255, 255, 255, 255 );
			_GModRect_SendAnimate( playerid, 4, 1.5, 0.8 );
			
			_GModRect_Hide( playerid, 4, 1.5 );
			
			
			return;
		end
		
		-- anything else is auto choose team

		if ( _TeamNumPlayers( TEAM_YELLOW ) > _TeamNumPlayers( TEAM_BLUE ) ) then
			
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
		 _GModText_SetText( "The object of Nine Tenths is to have the most\nitems in your base at the end of the round." );
		_GModText_Send( userid, 50 );

	end
	
	function onShowTeam ( userid )

		_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 0, 0, 1, 1 );
		 _GModRect_SetColor( 255, 255, 255, 50 );
		 _GModRect_SetTime( 99999, 0.5, 3 );
		_GModRect_Send( userid, 2 );

		_GModRect_Start( "gmod/gm_910/team1" );
		 _GModRect_SetPos( 0.15, 0.25, 0.35, 0.42 );
		 _GModRect_SetColor( 255, 255, 255, 255 );
		 _GModRect_SetTime( 99999, 0.5, 3 );
		_GModRect_Send( userid, 3 );
		
		_GModRect_Start( "gmod/gm_910/team2" );
		 _GModRect_SetPos( 0.5, 0.25, 0.35, 0.42 );
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

	end
	
	function onShowSpare2 ( userid )

	end
	
_Msg("--------------------------------------------------------\n")
_Msg("-- gm_910 -------------------------  Nine Tenths  ------\n")
_Msg("--------------------------------------------------------\n")
