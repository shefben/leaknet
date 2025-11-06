----------------------------------------------------------------------------------
-- Longsight final open source build. Yep.
---------------------
-- This is probably one of the messiest, sloppiest, most disorganized scripts you
-- will ever see. Why? Because I coded it, and I'm not going to clean it up for
-- you. There are parts that are useless, parts that shouldn't be there, and
-- better ways to do some of this, but the code works, and this is how I did it.
---------------------
-- Anyways, don't steal my shit. It will be a bit obvious if you do. VERY obvious.
-- MrSteak63(mrsteak63@gmail.com) http://www.beavasoft.com/
----------------------------------------------------------------------------------

fIntermissionEnd = 0
bEndGame = false

_OpenScript( "Events.lua" ) -- Use the default (empty) events


--  Called every frame from: CHL2MPRules::Think( void ) -----------------------


   -- Here's where Garry fixes MrSteak63's dumbass scripting.
   -- Don't SEND TEXTS/RECTS EVERY FUCKING FRAME JESUS CHRIST YOU SUCK
    function DrawUpdateScores( player )
    
    
 		if showtherecords == 1 then
 		
		_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 0.7, 0.3, 1, 0.1 ); -- start, up, end, down
		 _GModRect_SetColor( 0, 0, 0, 175 );
		 _GModRect_SetTime( 99999, 0, 0 );
		 _GModRect_Send( 0, 0 );
		 _GModRect_SetPos( 0.7, 0.2975, 1, 0.005 ); -- start, up, end, down
		 _GModRect_SetColor( 255, 255, 255, 255 );
		 _GModRect_SetTime( 99999, 0, 0 );
		 _GModRect_Send( player, 1 );
		 
		 _GModText_Start( "Default" );
		 _GModText_SetPos( 0.72, 0.3 ); -- x, y
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Longest Shot: " .. longestdist .. " feet" );
		 _GModText_SetDelay( 0 );
		_GModText_Send( player, 2 );
					-----
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.72, 0.325 ); -- x, y
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "By: " .. longestperson );
		 _GModText_SetDelay( 0 );
					_GModText_Send( player, 3 );
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.72, 0.35 ); -- x, y
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Shortest Kill: " .. shortestdist .. " feet" );
		 _GModText_SetDelay( 0 );
		_GModText_Send( player, 4 );
		
		_GModText_SetPos( 0.72, 0.375 ); -- x, y
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Person Shot: " .. shortestperson );
		 _GModText_SetDelay( 0 );
		_GModText_Send( player, 5 );

		end
					 
    
    end

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
	
		_PlayerGiveSWEP ( playerid, "weapons/longsight/weapon_tracedsniper.lua" )
		_PlayerGiveSWEP ( playerid, "weapons/longsight/weapon_pistol.lua" )
		_PlayerGiveSWEP ( playerid, "weapons/longsight/weapon_normalsniper.lua" )
		_PlayerGiveSWEP ( playerid, "weapons/longsight/weapon_boltlauncher.lua" )
		_PlayerGiveAmmo( playerid, 1000, "357", false );
		
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

	          function cc_thebest(fromplayer)
	          				_ScreenText( fromplayer, "The best sniper is " .. longestperson .. " with " .. longestdist .. " feet", -1, -.35, 255, 0, 0, 255, 0.1, 1.0, 3, 1, 4 )
	          	--_ScreenText( fromplayer, "The worst sniper is " .. shortestperson .. " who got shot at " .. shortestdist .. " feet. HAHAHAHAHA!", -1, -.35, 255, 0, 0, 200, 0.1, 1.0, 3, 1, 3 )
	          	end
	          	function cc_worst (fromplayer)
	          			          	_ScreenText( fromplayer, "The worst sniper is " .. shortestperson .. " who got shot at " .. shortestdist .. " feet. HAHAHAHAHA!", -1, -.27, 255, 0, 0, 255, 0.1, 1.0, 3, 1, 3 )
	          		end
	          		function cc_therecords(fromplayer)
	          			local hisid = _PlayerInfo( fromplayer, "networkid" )
	          			if hisid == "UNKNOWN" then
	          						_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 0.7, 0.3, 1, 0.1 ); -- start, up, end, down
		 _GModRect_SetColor( 0, 0, 0, 175 );
		 _GModRect_SetTime( 99999, 0, 0 );
	          				showtherecords = 1
		 for i = 1, _MaxPlayers() do
		 	_Msg("Sending the stats to all players, don't worry about the errors if there are any.\n")
		_GModRect_Send( i, 0 );
		end
		end
		end
		function cc_therecords3()
	          						_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 0.7, 0.3, 1, 0.1 ); -- start, up, end, down
		 _GModRect_SetColor( 0, 0, 0, 175 );
		 _GModRect_SetTime( 99999, 0, 0 );
	          				showtherecords = 1
		 for i = 1, _MaxPlayers() do
		 	_Msg("Sending the stats to all players, don't worry about the errors if there are any.\n")
		_GModRect_Send( i, 0 );
	          				end
	          			end
  	     		function cc_therecords2(fromplayer)	        --FUCKING HELL REMMEBER THIS YOU BITCH
	          		local hisid = _PlayerInfo( fromplayer, "networkid" )
	          		if hisid == "UNKNOWN" then
	          			showtherecords = 0
						for i = 1, _MaxPlayers() do
							_Msg("Removing the stats, ignore the errors if there are any.\n")
						_GModRect_Hide( i, 0, 0.9 );
						_GModRect_Hide( i, 1, 0.9 );
						_GModText_Hide(i, 2, 0.9)
						_GModText_Hide(i, 3, 0.9)
						_GModText_Hide(i, 4, 0.9)
						_GModText_Hide(i, 5, 0.9)
						end
	          		end
	          	end
	          		--function cc_workit()
			--end
	          		CONCOMMAND("theworst", cc_worst)
	          		CONCOMMAND("thebest", cc_thebest)
	          		CONCOMMAND("showrecords", cc_therecords)
					CONCOMMAND("hiderecords", cc_therecords2)	
					--CONCOMMAND("makeitwork", cc_workit)
	function gamerulesStartMap ()
		
		dotherec = 0
		showtherecords = 0
		shortestdist = 2500
		shortestperson = "Nobody"
		longestdist = 0
		longestperson = "Nobody"
		showtherecords = 1
		
		-- Anything to imitialize?	
	
		bEndGame = false
		fIntermissionEnd = 0
		PlayerFreezeAll( false )
			
	end

	function PickDefaultSpawnTeam( userid )
		return false; -- return false tells it to just do the default action. (random teams)
	end

-- Events --



	function eventPlayerKilled ( killed, attacker, weapon )

		local hispos = _EntGetPos(killed)
		local mypos = _EntGetPos(attacker)
		local thelength1 = vecLength( vecSub( hispos, mypos ) )
		local thelength = thelength1 / 30

		_PlayerAddDeath( killed, 1 )
		
		-- garry: HEY LOOK - IM ROUNDING THE NUMBERS SO IT ISN'T 80 DECIMAL PLACES
		
		thelength1 = math.floor( thelength1 * 100 );
		thelength1 = thelength1 / 100;
		
		thelength = math.floor( thelength * 100 );
		thelength = thelength / 100;
		
		-- :garry
		
		-- Player killed himself, what a top hat
		if ( killed == attacker ) then
			
			_PlayerAddScore( killed, -1 )
			
		-- Was killed by another player!
		elseif ( attacker > 0 ) then
		
			_PlayerAddScore( attacker, 1 )
			_PlayerAllowDecalPaint( attacker ); -- Let the player spraypaint the body
			_ScreenText( killed, _PlayerInfo( attacker, "name" ) .. " killed you from " .. thelength .. " feet", -1, -.75, 255, 0, 0, 200, 0.1, 1.0, 3, 1, 0 )
			_ScreenText( attacker, "You killed " .. _PlayerInfo( killed, "name" ) .. " from " .. thelength .. " feet", -1, -.75, 255, 0, 0, 200, 0.1, 1.0, 3, 1, 0 )
			if thelength > longestdist then
				longestdist = thelength
				longestperson = _PlayerInfo( attacker, "name" )
				for i = 1, _MaxPlayers() do
					_ScreenText( i, _PlayerInfo( attacker, "name" ) .. " got the new longest shot with  " .. thelength .. " feet", -1, -.5, 255, 0, 0, 255, 0.1, 1.0, 3, 1, 5 )
				end
			end
					if thelength < shortestdist then
				shortestdist = thelength
				shortestperson = _PlayerInfo( killed, "name" )
				for i = 1, _MaxPlayers() do
					_ScreenText( i, _PlayerInfo( killed, "name" ) .. " got shot at the shortest range of  " .. thelength .. " feet. HAHAHAHAHAH!", -0.68, 0, 255, 0, 0, 255, 0.1, 1.0, 3, 1, 5 )
					end
				end
		-- The soft sod was killed by the world
		else
		
			_PlayerAddScore( killed, -1 )
			
		end	
		
		-- Update the score INSTEAD OF SENDING IT EVERY SINGLE COCK SUCKING FRAME (garry)
		
		DrawUpdateScores( 0 );
		
	end


	function eventPlayerSpawn ( userid )
		
		-- If we're in teamplay mode switch team circles on for everyone
		if dotherec == 0 then
			showtherecords = 1
			dotherec = 1
			end
		_PlayerSetDrawTeamCircle( userid, _GetRule( "Teamplay" ) )
		
		-- send him the latest stats
		DrawUpdateScores( userid );

	end
	
	
	
-- These are called by the players in game using the F1 - F4 keys
	
	function onShowHelp ( userid )
		_GModText_Start( "Default" );
		 _GModText_SetPos( -1, 0.3 );
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 10, 0.2, 1 );
		 _GModText_SetText( "Longsight Release 1\n\nMain Version 2.6\n\n\n\nKill other snipers. Try to get the longest kill, and not be the shortest kill.\n\nThere are 3 rifles, and 1 pistol.\n\n\nmrsteak63@gmail.com" );
		_GModText_Send( userid, 8 );
	end
	
	function onShowTeam ( userid )

	end
	
	function onShowSpare1 ( userid )

	end
	
	function onShowSpare2 ( userid )

	end
	
	function eventPlayerConnect ( name, address, steamid ) -- BECAUSE I'M BETTER THAN YOU AND THIS IS BUGGED AND I CANT BE ASSED TO FIX IT
	if steamid == "STEAM_0:1:2166040" then
		for i = 1, _MaxPlayers() do
		_PlaySoundPlayer(i, "/vo/npc/male01/runforyourlife01.wav")
		end
			--_PrintMessageAll( 1, "MrSteak63 is joining. Give me a pope hat!" );
		_GModText_Start( "Default" );
		 _GModText_SetPos( -1, 0.125 );
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 5, 0.2, 1 );
		 _GModText_SetText( "MrSteak63 is joining. Give me a pope hat!" );
		_GModText_Send( 0, 25 );
		end
	end
