
function eventPlayerKilled ( killed, attacker, weapon )

	if (PlayerData[killed] ~= nil) then
		ClearPlayerData( killed, 1 )
	end

	DrawScores(0)

end

function eventPlayerDisconnect ( name, userid, address, steamid, reason )

	ClearPlayerData( userid, 0 )

end

function SetSpectatorMode ( userid )

	_PlayerSpectatorStart( userid, 4 );
	_PlayerSpectatorTarget( userid, PlayerData[userid].Bird );

	UpdatePlayerLabels();
end

function eventPlayerSpawn ( userid )

	-- If they're a spectator show them the team choice menu
	if ( _PlayerInfo( userid, "team" ) == TEAM_SPECTATOR ) then

		DrawIntro( userid );

		AddTimer( 4.5, 1, onShowTeam, userid );

		_EntSetPos( userid, vector3( 0, 0, 0) );
	end

	_PlayerSetDrawTeamCircle( userid, false )
	_PlayerSetSprint( userid, false )

	ClearPlayerData(userid, 0)

	UndrawRunLeft(userid)

	if (_PlayerInfo(userid, "team") == TEAM_BIRDS) then

		local vPos = _PlayerGetShootPos( userid );

		local iBird = 0
		local sModel = "models/seagull.mdl"
		local sType = "npc_seagull"

		if (PlayerData[userid].BirdType == BT_SEAGULL) then	

			sType = "npc_seagull"	
			sModel = "models/seagull.mdl"

		elseif (PlayerData[userid].BirdType == BT_CROW) then	

			sType = "npc_crow"	
			sModel = "models/crow.mdl"

		elseif (PlayerData[userid].BirdType == BT_PIGEON) then

			sType = "npc_pigeon"	
			sModel = "models/pigeon.mdl"

		else	
			_Msg("Unknown birdType!\n");
			
			sType = "npc_seagull"	
			sModel = "models/seagull.mdl"
			PlayerData[userid].BirdType = BT_SEAGULL
		end

		iBird = _EntCreate( sType );

		if (iBird == 0) then
			_Msg("********************************************\n")
			_Msg("************ Unable to create entity(!) ****\n")
			_Msg("********************************************\n")
			_PlayerChangeTeam( playerid, TEAM_SPECTATOR );

			return
		end

		_EntSetModel(iBird, sModel)
		_EntSetMoveType( iBird, MOVETYPE_FLYGRAVITY)
		_EntSetMoveCollide( iBird, MOVECOLLIDE_FLY_BOUNCE)
		_EntSetCollisionGroup( iBird, COLLISION_GROUP_PLAYER_MOVEMENT)
		_EntSetSolid( iBird, SOLID_BBOX)

		_EntSetKeyValue( iBird, "model", sModel );
		_EntSetKeyValue( iBird, "health", "1" );
		--_EntSetKeyValue( iBird, "spawnflags", "272" );
		--_EntSetKeyValue( iBird, "spawnflags", "912" ); 
									-- Efficient(16) + 
									-- Wait for Script(128) + 
									-- Long Visibility(256) +
									-- Fade Corpse(512)

		_EntSetPos( iBird, vPos );

		_EntSpawn( iBird );

		_EntSetActivity( iBird, ACT_FLY );

		PlayerData[userid].Bird = iBird

		-- We need to time this because PlayerSpawn is called while they're still spawning
		AddTimer( 0.1, 1, SetSpectatorMode, userid );

		PlayerData[userid].UpdateTimer = AddTimer( 0.1, 0, DoUpdateTarget, userid );

		DrawRunLeft( userid )
	end			
end

function eventNPCKilled ( killerid, killed )

	local key = 0
	for key in PlayerData do

		if (_PlayerInfo( key, "connected" ) and _PlayerInfo(key, "team") == TEAM_BIRDS ) then

			if (PlayerData[key].Bird == killed) then

				if (_PlayerInfo( killerid, "connected" )) then

					_GModText_Start( "ImpactMassive" );
					 _GModText_SetColor( 40, 70, 140, 255 );
					 _GModText_SetTime( 2, 0, 0 );
					 _GModText_SetPos( -1, 0.2 );
					 _GModText_SetText( "You were killed by " .. _PlayerInfo( killerid, "name" ) .. "!" );
					_GModText_Send( key, 52 );

					_GModText_Start( "ImpactMassive" );
					 _GModText_SetColor( 210, 160, 0, 255 );
					 _GModText_SetTime( 2, 0, 0 );
					 _GModText_SetPos( -1, 0.2 );
					 _GModText_SetText( "You killed " .. _PlayerInfo( key, "name" ) .. "!" );
					_GModText_Send( killerid, 53 );

					_TeamAddScore(_PlayerInfo( killerid, "team" ), 1)
					_PlayerAddScore(killerid, 1)
				end

				_PlayerAddDeath(key, 1)
				ClearPlayerData(key, 1)
				DrawScores(0)

				break;
			end
		end
	end
end

