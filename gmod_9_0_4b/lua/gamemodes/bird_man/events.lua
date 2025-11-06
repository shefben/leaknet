

	function eventPlayerKilled ( killed, attacker, weapon )

		_PlayerAddDeath( killed, 1 )

		if ( killed == attacker ) then
			_PlayerAddScore( killed, -1 )
		elseif ( attacker > 0 ) then
			_PlayerAddScore( attacker, 1 )
			_PlayerAllowDecalPaint( attacker ); 
		else
			_PlayerAddScore( killed, -1 )
		end	

	end

	function eventPlayerSpawn ( userid )
	
		if (_PlayerInfo(userid, "team") == TEAM_SPECTATOR) then
			onShowTeam(userid);
			
		end
		
		_PlayerSetDrawTeamCircle(userid, true);
		
		PlayerInfo[userid] = {}
		PlayerInfo[userid].StartPos = vector3(0,0,0)
		PlayerInfo[userid].IsFlying = 0
		
		DrawStats( userid )
		UpdateScores( userid )
	end
	
	function StartPoint( Activator, Caller, NewCP )

		if (_PlayerInfo(Activator, "connected") == false ) then
			return
		end
		
		if (PlayerInfo[Activator].IsFlying == 1) then
			return
		end
		
		PlayerInfo[Activator].StartPos = _PlayerGetShootPos( Activator )
		PlayerInfo[Activator].IsFlying = 1
		PlayerInfo[Activator].Time = _CurTime()
		PlayerInfo[Activator].Timer = AddTimer( 0.1, 0, showDistance, Activator );
	end
	
	function TouchDown( Activator, Caller, NewCP )

		if (_PlayerInfo(Activator, "connected") == false ) then
			return
		end
		
		if (PlayerInfo[Activator].IsFlying == 0) then
			return
		end
		
		local StartPos = PlayerInfo[Activator].StartPos
		local EndPos = _PlayerGetShootPos( Activator )
		local Diff = vecSub(StartPos, EndPos)
		local Distance = vecLength(Diff)
		Distance = Distance / 30
		local TimeDiff = _CurTime() - PlayerInfo[Activator].Time
		
		_GModText_Hide( player, 21 );
		_GModText_Hide( player, 10 );

		_GModText_Start( "Default" );
		_GModText_SetColor( 0, 255, 0, 255 );
		_GModText_SetTime( 5, 0, 0.3 );
		_GModText_SetPos( -1, 0.3 )
		_GModText_SetText( "You flew " .. FormatDist(Distance) .. " feet for " .. FormatTime(TimeDiff) .. " seconds!");
		_GModText_Send( player, 1 );
		
		local c1 = 0
		local c2 = 0
		local c3 = 0
		local c4 = 0
		if (Distance > Stats.LongestDistance.Value or Stats.LongestDistance.Value == 0) then
			Stats.LongestDistance.Value = Distance
			Stats.LongestDistance.Player = _PlayerInfo(Activator, "name")
			Stats.LongestDistance.Team = _PlayerInfo(Activator, "team")
			c1 = 1
		end
		if (Distance < Stats.ShortestDistance.Value or Stats.ShortestDistance.Value == 0) then
			Stats.ShortestDistance.Value = Distance
			Stats.ShortestDistance.Player = _PlayerInfo(Activator, "name")
			Stats.ShortestDistance.Team = _PlayerInfo(Activator, "team")
			c2 = 1
		end
		
		if (TimeDiff > Stats.LongestTime.Value or Stats.LongestTime.Value == 0) then
			Stats.LongestTime.Value = TimeDiff
			Stats.LongestTime.Player = _PlayerInfo(Activator, "name")
			Stats.LongestTime.Team = _PlayerInfo(Activator, "team")
			c3 = 1
		end
		if (TimeDiff < Stats.ShortestTime.Value or Stats.ShortestTime.Value == 0) then
			Stats.ShortestTime.Value = TimeDiff
			Stats.ShortestTime.Player = _PlayerInfo(Activator, "name")
			Stats.ShortestTime.Team = _PlayerInfo(Activator, "team")
			c4 = 1
		end
		
		if (c1 or c2 or c3 or c4) then
			_TeamAddScore( _PlayerInfo(Activator, "team"), 1 )
			UpdateScores(0)
		end
		
		DrawStats(0)
		
		PlayerInfo[Activator].IsFlying = 0
		HaltTimer( PlayerInfo[Activator].Timer )
	end
	

	-- Thanks g33k for letting me use this code:
	function autoJoinTeam(playerid, wantTeam)
		-- do I really need this much code for an auto-join?

		local t0p, t1p, t2p, t3mp = -1, -1, -1, -1;
		local plyrCount = 0;

		plyrCount = _TeamNumPlayers(TEAM_BLUE);
		plyrCount = plyrCount + _TeamNumPlayers(TEAM_YELLOW);
		plyrCount = plyrCount + _TeamNumPlayers(TEAM_GREEN);
		plyrCount = plyrCount + _TeamNumPlayers(TEAM_RED);

		-- this is used to make sure no one joins a team by theirself if there's already a team with 1 person
		if (_TeamNumPlayers(TEAM_BLUE) == 0) then t0p = TEAM_BLUE;
		elseif (_TeamNumPlayers(TEAM_BLUE) == 1) then
			if (_PlayerInfo(playerid, "team") ~= TEAM_BLUE) then
				t1p = TEAM_BLUE;
			end
		elseif (_TeamNumPlayers(TEAM_BLUE) == 2) then t2p = TEAM_BLUE;
		elseif (_TeamNumPlayers(TEAM_BLUE) >= 3) then t3mp = TEAM_BLUE; end;

		if (_TeamNumPlayers(TEAM_YELLOW) == 0) then t0p = TEAM_YELLOW;
		elseif (_TeamNumPlayers(TEAM_YELLOW) == 1) then
			if (_PlayerInfo(playerid, "team") ~= TEAM_YELLOW) then
				t1p = TEAM_YELLOW;
			end
		elseif (_TeamNumPlayers(TEAM_YELLOW) == 2) then t2p = TEAM_YELLOW;
		elseif (_TeamNumPlayers(TEAM_YELLOW) >= 3) then t3mp = TEAM_YELLOW; end;

		if (_TeamNumPlayers(TEAM_GREEN) == 0) then t0p = TEAM_GREEN;
		elseif (_TeamNumPlayers(TEAM_GREEN) == 1) then
			if (_PlayerInfo(playerid, "team") ~= TEAM_GREEN) then
				t1p = TEAM_GREEN;
			end
		elseif (_TeamNumPlayers(TEAM_GREEN) == 2) then t2p = TEAM_GREEN;
		elseif (_TeamNumPlayers(TEAM_GREEN) >= 3) then t3mp = TEAM_GREEN; end;

		if (_TeamNumPlayers(TEAM_RED) == 0) then t0p = TEAM_RED;
		elseif (_TeamNumPlayers(TEAM_RED) == 1) then
			if (_PlayerInfo(playerid, "team") ~= TEAM_RED) then
				t1p = TEAM_RED;
			end
		elseif (_TeamNumPlayers(TEAM_RED) == 2) then t2p = TEAM_RED;
		elseif (_TeamNumPlayers(TEAM_RED) >= 3) then t3mp = TEAM_RED; end;

		-- make sure there's at least one team with less than 3 people
		if (plyrCount < 9) and ((t0p > 0) or (t1p > 0) or (t2p > 0)) then
			if (t1p > 0) then
				-- there's a 1 person team. Make the player join this team
				--_Msg("there's a 1 person team. Make the player join this team\n");
				if (wantTeam > 0) and (_TeamNumPlayers(wantTeam) == 1) then
					-- If the wanted team has 1 person then let them join it
					_PlayerChangeTeam(playerid, wantTeam);
					_PlayerRespawn(playerid);
				else
					_PlayerChangeTeam(playerid, t1p);
					_PlayerRespawn(playerid);
				end

			elseif (t3mp > 0) then
				-- there's a 3 or more person team, switch one person and make them join a new team
				--_Msg("there's a 3 or more person team, switch one person and make them join a new team\n");
				if (wantTeam > 0) and (_TeamNumPlayers(wantTeam) == 0) then
					-- If the wanted team has 0 people then let them join it
					_PlayerChangeTeam(playerid, wantTeam);
					_PlayerRespawn(playerid);

					local rTeamMember = randomTeamMember(t3mp);
					if (rTeamMember > 0) then
						_PlayerChangeTeam(rTeamMember, wantTeam);
						_PlayerRespawn(rTeamMember);
						_PrintMessage(rTeamMember, 4, "You were auto-switched to even teams");
					end
				else
					_PlayerChangeTeam(playerid, t0p);
					_PlayerRespawn(playerid);

					local rTeamMember = randomTeamMember(t3mp);
					if (rTeamMember > 0) then
						_PlayerChangeTeam(rTeamMember, t0p);
						_PlayerRespawn(rTeamMember);
						_PrintMessage(rTeamMember, 4, "You were auto-switched to even teams");
					end
				end

			elseif (t2p > 0) then
				-- there's a 2 person team (and no 1 person team) make them join as the 3rd
				--_Msg("there's a 2 person team (and no 1 person team) make them join as the 3rd\n");
				if (wantTeam > 0) and (_TeamNumPlayers(wantTeam) == 2) then
					-- If the wanted team has 2 people then let them join it
					_PlayerChangeTeam(playerid, wantTeam);
					_PlayerRespawn(playerid);
				else
					_PlayerChangeTeam(playerid, t2p);
					_PlayerRespawn(playerid);
				end		
			else
				-- must be the only player playing, switch them to a 0 person team
				--_Msg(" must be the only player playing, switch them to a 0 person team\n");
				if (wantTeam > 0) and (_TeamNumPlayers(wantTeam) == 0) then
					-- If the wanted team has 0 people then let them join it
					_PlayerChangeTeam(playerid, wantTeam);
					_PlayerRespawn(playerid);
				else
					-- there's no 1 or 2 person team. they have to join a empty team
					_PlayerChangeTeam(playerid, t0p);
					_PlayerRespawn(playerid);
				end
			end

		else
			-- Find out which team has the least amount of people
			if (_TeamNumPlayers(TEAM_BLUE) <= _TeamNumPlayers(TEAM_YELLOW))
				and (_TeamNumPlayers(TEAM_BLUE) <= _TeamNumPlayers(TEAM_GREEN))
				and (_TeamNumPlayers(TEAM_BLUE) <= _TeamNumPlayers(TEAM_RED))
			then
				-- blue has the least amount of players
				_PlayerChangeTeam(playerid, TEAM_BLUE);
				_PlayerRespawn(playerid);
			elseif (_TeamNumPlayers(TEAM_YELLOW) <= _TeamNumPlayers(TEAM_BLUE))
				and (_TeamNumPlayers(TEAM_YELLOW) <= _TeamNumPlayers(TEAM_GREEN))
				and (_TeamNumPlayers(TEAM_YELLOW) <= _TeamNumPlayers(TEAM_RED))
			then
				-- yellow has the least amount of players
				_PlayerChangeTeam(playerid, TEAM_YELLOW);
				_PlayerRespawn(playerid);
			elseif (_TeamNumPlayers(TEAM_GREEN) <= _TeamNumPlayers(TEAM_BLUE))
				and (_TeamNumPlayers(TEAM_GREEN) <= _TeamNumPlayers(TEAM_YELLOW))
				and (_TeamNumPlayers(TEAM_GREEN) <= _TeamNumPlayers(TEAM_RED))
			then
				-- green has the least amount of players
				_PlayerChangeTeam(playerid, TEAM_GREEN);
				_PlayerRespawn(playerid);
			else
				-- none of the other teams had the least amount so that only leaves red
				_PlayerChangeTeam(playerid, TEAM_RED);
				_PlayerRespawn(playerid);
			end
		end
	end
