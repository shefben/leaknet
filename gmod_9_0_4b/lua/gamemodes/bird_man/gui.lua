
	function FormatTime( diff )
		local time = string.format("%.3f", diff)
		
		return time;
	end
	
	function FormatDist( distance )
		local time = string.format("%.3f", distance)
		
		return time;
	end

	function showDistance(player)
	
		if (_PlayerInfo(player, "connected") == false ) then
			HaltTimer( PlayerInfo[player].Timer )
			return
		end

		if (PlayerInfo[player].IsFlying == 0) then
			HaltTimer( PlayerInfo[player].Timer )
			return
		end

		local StartPos = PlayerInfo[player].StartPos
		local EndPos = _PlayerGetShootPos( player )
		local Diff = vecSub(StartPos, EndPos)
		local Distance = vecLength(Diff)
		Distance = Distance / 30
		local TimeDiff = _CurTime() - PlayerInfo[player].Time
		
		_GModText_Hide( player,1 );
		_GModText_Hide( player,10 );

		_GModText_Start( "Default" );
		_GModText_SetColor( 255, 0, 0, 255 );
		_GModText_SetTime( 1, 0, 0 );
		_GModText_SetPos( -1, 0.3 )
		_GModText_SetText( FormatDist(Distance) .. " feet!");
		_GModText_Send( player, 21 );
		
		_GModText_Start( "Default" );
		_GModText_SetColor( 255, 0, 0, 255 );
		_GModText_SetTime( 1, 0, 0 );
		_GModText_SetPos( -1, 0.33 )
		_GModText_SetText( FormatTime(Distance) .. " seconds!");
		_GModText_Send( player, 10 );
	end
	
	function teamColor(Team)
		if (Team == TEAM_RED)	then		_GModRect_SetColor( 255, 192, 192, 150 );
		elseif (Team == TEAM_BLUE) then		_GModRect_SetColor( 192, 192, 255, 150 );
		elseif (Team == TEAM_GREEN) then	_GModRect_SetColor( 192, 255, 192, 150 );
		elseif (Team == TEAM_YELLOW) then	_GModRect_SetColor( 255, 255, 192, 150 );
		else					_GModRect_SetColor( 192, 192, 192, 150 );
		end
	end
	
	function DrawStat(player, offset, name, units, stat, id)
		
		_GModRect_Start( "gmod/white" );
			_GModRect_SetPos( 0.035, offset, 0.18, 0.10 );
			teamColor(stat.Team)
			_GModRect_SetTime( 99999, 0, 0 );
			_GModRect_SetDelay( 0 );
		_GModRect_Send( player, id );
	
		_GModText_Start( "Default" );
			_GModText_SetColor( 0, 0, 0, 255 );
			_GModText_SetTime( 99999, 0, 0 );
			_GModText_SetPos( 0.05, offset + 0.01 )
			_GModText_SetText( name )
		_GModText_Send( player, id+1 );
		
		_GModText_Start( "Default" );
			_GModText_SetColor( 0, 0, 0, 255 );
			_GModText_SetTime( 99999, 0, 0 );
			_GModText_SetPos( 0.05, offset + 0.04 )
			if (units == 0) then
				_GModText_SetText( FormatDist(stat.Value) .. " feet")
			else
				_GModText_SetText( FormatTime(stat.Value) .. " s")
			end
		_GModText_Send( player, id+2 );
		
		_GModText_Start( "Default" );
			_GModText_SetColor( 0, 0, 0, 255 );
			_GModText_SetTime( 99999, 0, 0 );
			_GModText_SetPos( 0.05, offset + 0.07 )
			_GModText_SetText( "by " .. stat.Player)
		_GModText_Send( player, id+3 );
	
	end
	
	function DrawStats(player)
	
		DrawStat( player, 0.24, "Longest Distance", 0, Stats.LongestDistance, 22)
		DrawStat( player, 0.36, "Longest Time", 1, Stats.LongestTime, 26)
		DrawStat( player, 0.48, "Shortest Distance", 0, Stats.ShortestDistance, 30)
		DrawStat( player, 0.60, "Shortest Time", 1, Stats.ShortestTime, 34)
		
	end
	
	function onShowHelp ( userid )
		_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 1.0, 0.69, 0.7, 0.35 );
		 _GModRect_SetColor( 0, 0, 0, 150 );
		 _GModRect_SetTime( 10, 0.5, 1 );
		 _GModRect_SetDelay( 0 );
		_GModRect_Send( userid, 4 );

		_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 0.3, 0.69, 0.7, 0.35 );
		 _GModRect_SetColor( 0, 0, 0, 150 );
		 _GModRect_SetDelay( 1 );
		_GModRect_SendAnimate(userid, 4, 0.5, 0.5);

		_GModText_Start( "Default" );
		 _GModText_AllowOffscreen( true );
		 _GModText_SetPos( 1.0, 0.7 );
		 _GModText_SetColor( 255, 255, 255, 0 );
		 _GModText_SetTime( 10, 0.5, 1 );
		 _GModRect_SetDelay( 1 );
		 _GModText_SetText( 
		 	
		 	"The object of Bird Man is to create a flying machine to get\n" .. 
		 	"the longest time and distance travelled. Select a team, and join in.\n"  ..
		 	"When the walls are removed, each team should take a turn in flying their\n"
		 )
		_GModText_Send( userid, 50 );

		_GModText_Start( "Default" );
		 _GModText_AllowOffscreen( true );
		 _GModText_SetPos( 0.32, 0.7 );
		 _GModText_SetColor( 255, 200, 0, 205 );
		 _GModText_SetDelay( 1.0 );
		_GModText_SendAnimate( userid, 50, 0.5, 0.5 );

		_GModText_Start( "Default" );
		 _GModText_AllowOffscreen( true );
		 _GModText_SetPos( 1.0, 0.76 );
		 _GModText_SetColor( 255, 255, 255, 0 );
		 _GModText_SetTime( 10, 0.5, 1 );
		 _GModText_SetText( 
		 	"machine. Points are awarded for shortest time and distance as well as\n" ..
		 	"longest time and distance.\n" ..
		 	"\n" .. 
		 	"For more information on the bird man competition, see:\n" ..
		 	"           http://www.birdman.org.uk\n"  
		 	
		 );
		_GModText_Send( userid, 51 );

		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.32, 0.76 );
		 _GModText_SetColor( 255, 200, 0, 205 );
		 _GModText_SetDelay( 1.0 );
		 _GModText_AllowOffscreen( true );
		_GModText_SendAnimate( userid, 51, 0.5, 0.5 );
	end

	function onShowTeam ( userid )

		_PlayerOption( userid, "ChooseTeam", 99999 );

		-- Title
		_GModText_Start( "ImpactMassive" );
		 _GModText_SetPos( -1, 0.1 );
		 _GModText_SetColor( 255, 200, 0, 205 );
		 _GModText_SetTime( 99999, 1.5, 0 );
		 _GModText_SetText( "Bird Man" );
		 _GModText_SetDelay( 1.5 );
		 _GModText_AllowOffscreen( true );
		_GModText_Send( userid, 0 );

		_GModText_Start( "ImpactMassive" );
		 _GModText_SetPos( -1, 0.3 );
		 _GModText_SetColor( 255, 200, 0, 205 );
		 _GModText_SetDelay( 1.0 );
		_GModText_SendAnimate( userid, 0, 1.5, 0.7 );


		-- credit line
		_GModText_Start( "HudHintTextLarge" );
		 _GModText_SetPos( 0.6, 0.37 );
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 99999, 0.5, 1.5 );
		 _GModText_SetText( "by AndyVincent" );
		 _GModText_SetDelay( 1.0 );
		_GModText_Send( userid, 1 );


		-- background
		_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 0.0, 0.0, 1, 1 );
		 _GModRect_SetColor( 255, 255, 255, 255 );
		 _GModRect_SetTime( 99999, 0.5, 2 );
		 _GModRect_SetDelay( 1 );
		_GModRect_Send( userid, 0 );

		_GModRect_Start( "gmod/white" );
		 _GModRect_SetPos( 0.3, 0.3, 0.7, 0.35 );
		 _GModRect_SetColor( 0, 0, 0, 150 );
		 _GModRect_SetDelay( 1 );
		_GModRect_SendAnimate(userid, 0, 0.5, 0.5);

		-- Options
		_GModText_Start("Default");
		 _GModText_SetPos(0.32, 1);
		 _GModText_SetColor(255, 255, 255, 255) ;
		 _GModText_SetTime(99999, 3, 1.0);
		 _GModText_SetText("Choose your team:\n\n\n1. Blue Team\n2. Yellow Team\n3. Green Team\n4. Red Team\n\n5. Auto");
		 _GModText_SetDelay(1.5);
		_GModText_Send(userid, 2);

		_GModText_Start("Default");
		 _GModText_SetPos(0.32, 0.4);
		 _GModText_SetColor(255, 255, 255, 255);
		 _GModText_SetDelay(1.5);
		_GModText_SendAnimate(userid, 2, 1.0, 0.7);
	end

	function ChooseTeam( playerid, num, seconds )		
		_GModText_Hide( playerid, 0, 0.5 );
		_GModText_Hide( playerid, 1, 0.5 );
		_GModText_Hide( playerid, 2, 0.5 );

		_GModRect_Hide( playerid, 0, 0.9 );

		-- if they are by theirself then allow them to change
		if (_PlayerInfo(playerid, "team") ~= TEAM_SPECTATOR) then
			if (_TeamNumPlayers(_PlayerInfo(playerid,"team")) == 1) then
				if (num > 0) and (num < 5) then
					autoJoinTeam(playerid, (num+1));
				else
					autoJoinTeam(playerid, 0);
				end
				return;
			end
		end


		if (num == 1) then
			if (_PlayerInfo(playerid, "team") == TEAM_BLUE) then return; end;

			if (_PlayerInfo(playerid, "team") == TEAM_SPECTATOR) then
				if (_TeamNumPlayers(TEAM_BLUE) > _TeamNumPlayers(TEAM_YELLOW))
					or (_TeamNumPlayers(TEAM_BLUE) > _TeamNumPlayers(TEAM_GREEN))
					or (_TeamNumPlayers(TEAM_BLUE) > _TeamNumPlayers(TEAM_RED))
				then
					autoJoinTeam(playerid, 0);
					return;
				end;
			else
				if (_TeamNumPlayers(TEAM_BLUE) >= _TeamNumPlayers(_PlayerInfo(playerid,"team")))
				then
					_PrintMessage(playerid, 4, "Teams would be too uneven if you switched.");
					return;
				end
			end

			_PlayerChangeTeam(playerid, TEAM_BLUE);
			_PlayerRespawn(playerid);

			return;
		end

		if (num == 2) then

			if (_PlayerInfo(playerid, "team") == TEAM_YELLOW) then return; end;

			if (_PlayerInfo(playerid, "team") == TEAM_SPECTATOR) then
				if (_TeamNumPlayers(TEAM_YELLOW) > _TeamNumPlayers(TEAM_BLUE))
					or (_TeamNumPlayers(TEAM_YELLOW) > _TeamNumPlayers(TEAM_GREEN))
					or (_TeamNumPlayers(TEAM_YELLOW) > _TeamNumPlayers(TEAM_RED))
				then
					autoJoinTeam(playerid, 0);
					return;
				end;
			else
				if (_TeamNumPlayers(TEAM_YELLOW) >= _TeamNumPlayers(_PlayerInfo(playerid,"team")))
				then
					_PrintMessage(playerid, 4, "Teams would be too uneven if you switched.");
					return;
				end
			end

			_PlayerChangeTeam(playerid, TEAM_YELLOW);
			_PlayerRespawn(playerid);

			return;	
		end

		if (num == 3) then
			if (_PlayerInfo(playerid, "team") == TEAM_GREEN) then return; end;

			if (_PlayerInfo(playerid, "team") == TEAM_SPECTATOR) then
				if (_TeamNumPlayers(TEAM_GREEN) > _TeamNumPlayers(TEAM_YELLOW))
					or (_TeamNumPlayers(TEAM_GREEN) > _TeamNumPlayers(TEAM_BLUE))
					or (_TeamNumPlayers(TEAM_GREEN) > _TeamNumPlayers(TEAM_RED))
				then
					autoJoinTeam(playerid, 0);
					return;
				end;
			else
				if (_TeamNumPlayers(TEAM_GREEN) >= _TeamNumPlayers(_PlayerInfo(playerid,"team")))
				then
					_PrintMessage(playerid, 4, "Teams would be too uneven if you switched.");
					return;
				end
			end

			_PlayerChangeTeam(playerid, TEAM_GREEN);
			_PlayerRespawn(playerid);

			return;
		end

		if (num == 4) then
			if (_PlayerInfo(playerid, "team") == TEAM_RED) then return; end;

			if (_PlayerInfo(playerid, "team") == TEAM_SPECTATOR) then
				if (_TeamNumPlayers(TEAM_RED) > _TeamNumPlayers(TEAM_YELLOW))
					or (_TeamNumPlayers(TEAM_RED) > _TeamNumPlayers(TEAM_BLUE))
					or (_TeamNumPlayers(TEAM_RED) > _TeamNumPlayers(TEAM_GREEN))
				then
					autoJoinTeam(playerid, 0);
					return;
				end;
			else
				if (_TeamNumPlayers(TEAM_RED) >= _TeamNumPlayers(_PlayerInfo(playerid,"team")))
				then
					_PrintMessage(playerid, 4, "Teams would be too uneven if you switched.");
					return;
				end
			end

			_PlayerChangeTeam(playerid, TEAM_RED);
			_PlayerRespawn(playerid);

			return;
		end;


		autoJoinTeam(playerid, 0);
	end

	function UpdateScores( UserID )

		-- Blue Team
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.05, 0.08 );
		 _GModText_SetColor( 50, 150, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Blue Team: " );
		_GModText_Send( UserID, 13 );

		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.18, 0.08 );
		 _GModText_SetColor( 50, 150, 255, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( _TeamScore(TEAM_BLUE) );
		_GModText_Send( UserID, 14 );


		-- Yellow Team
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.05, 0.11 );
		 _GModText_SetColor( 255, 200, 0, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Yellow Team: " );
		_GModText_Send( UserID, 15 );

		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.18, 0.11 );
		 _GModText_SetColor( 255, 200, 0, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( _TeamScore(TEAM_YELLOW) );
		_GModText_Send( UserID, 16 );

		-- Green Team
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.05, 0.14 );
		 _GModText_SetColor( 50, 255, 150, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Green Team: " );
		_GModText_Send( UserID, 17 );

		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.18, 0.14 );
		 _GModText_SetColor( 50, 255, 150, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( _TeamScore(TEAM_GREEN) );
		_GModText_Send( UserID, 18 );

		-- RedTeam
		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.05, 0.17 );
		 _GModText_SetColor( 255, 100, 100, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "Red Team: " );
		_GModText_Send( UserID, 19 );

		_GModText_Start( "DefaultShadow" );
		 _GModText_SetPos( 0.18, 0.17 );
		 _GModText_SetColor( 255, 100, 100, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( _TeamScore(TEAM_RED) );
		_GModText_Send( UserID, 20 );

		-- Background
		_GModRect_Start( "gmod/white" );
			_GModRect_SetPos( 0.035, 0.06, 0.18, 0.16 );
			_GModRect_SetColor( 0, 0, 0, 150 );
			_GModRect_SetTime( 99999, 0, 0 );
			_GModRect_SetDelay( 0 );
		_GModRect_Send( UserID, 1 );

	end
