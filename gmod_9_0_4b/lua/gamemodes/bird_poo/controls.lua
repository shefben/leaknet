
DECAL_POOP1 = 89
DECAL_POOP2 = 90
DECAL_POOP3 = 91
DECAL_POOP4 = 92

function PlayerShatOn(iThisPlayer, iOtherPlayer)

	_GModText_Start( "ImpactMassive" );
	 _GModText_SetColor( 210, 160, 0, 255 );
	 _GModText_SetTime( 2, 0, 0 );
	 _GModText_SetPos( -1, 0.2 );
	 _GModText_SetText( "You got shat on by " .. _PlayerInfo( iThisPlayer, "name" ) );
	_GModText_Send( iOtherPlayer, 50 );

	_GModText_Start( "ImpactMassive" );
	 _GModText_SetColor( 40, 70, 140, 255 );
	 _GModText_SetTime( 2, 0, 0 );
	 _GModText_SetPos( -1, 0.2 );
	 _GModText_SetText( "You shat on " .. _PlayerInfo( iOtherPlayer, "name" ) );
	_GModText_Send( iThisPlayer, 51 );

	_EntEmitSound( PlayerData[iThisPlayer].Bird, "ambient/creatures/seagull_idle3.wav" );

	AddHUDPoop(iOtherPlayer)

	if (_PlayerInfo( iThisPlayer, "team" ) == _PlayerInfo( iOtherPlayer, "team" )) then

		_EntEmitSound( iOtherPlayer, "ambient/creatures/seagull_idle3.wav" );

		_PlayerAddScore(iThisPlayer, 3)

	else
		_EntEmitSound( iOtherPlayer, "vo/npc/male01/uhoh.wav" );

		_TeamAddScore(_PlayerInfo( iThisPlayer, "team" ), 1)
		_PlayerAddScore(iThisPlayer, 1)

		local iHealth = _PlayerInfo( iOtherPlayer, "health" );

		iHealth = iHealth - 2

		if (iHealth <= 0) then
			_PlayerAddDeath(iOtherPlayer, 1)
			_PlayerKill( iOtherPlayer )
			AddTimer( 3, 1, _EntSpawn, iOtherPlayer );
		else
			_PlayerSetHealth( iOtherPlayer, iHealth );
		end

	end
	
	DrawScores(0)

end

function DoFire(iPlayer)

	if ( _PlayerIsKeyDown( iPlayer, IN_ATTACK ) == false ) then

		if (PlayerData[iPlayer].IsFiring ~= 0) then

			local vPos = _EntGetPos( PlayerData[iPlayer].Bird );
			local vAng = _EntGetUpVector( iPlayer )

			vAng.z = -vAng.z

			_TraceLine( vPos, vAng, 2048, PlayerData[iPlayer].Bird );

			if (DRAW_POOP_LINE ~= 0) then
				_EffectInit();
					_EffectSetEnt( iPlayer );
					_EffectSetOrigin( _TraceEndPos() );
					_EffectSetStart( vPos );
					_EffectSetScale( 20 );
					_EffectSetMagnitude( 1.0 );
				_EffectDispatch( "FadingLineTeam" );
			end

			if (_TraceHit()) then

				_MakeDecal( math.random(DECAL_POOP1, DECAL_POOP4) )

				if (_TraceHitWorld() == false) then

					local hitEnt = _TraceGetEnt()

					if (hitEnt ~= iPlayer) then
						if (_PlayerInfo( hitEnt, "connected" ) ) then

							PlayerShatOn(iPlayer, hitEnt)
						
						else

							local found = 0
							local key = 0

							for key in PlayerData do
								if (_PlayerInfo( key, "connected" ) and _PlayerInfo(key, "team") == TEAM_BIRDS ) then
									if (PlayerData[key].Bird == hitEnt) then

										PlayerShatOn(iPlayer, key)

										found = 1
										break;
									end
								end
							end

							if (found == 0) then

								-- hit another entity type: damage it?

							end
							
						end
					end
				end 
			end	
		end

		PlayerData[iPlayer].IsFiring = 0
		return; 
	end;

	PlayerData[iPlayer].IsFiring = 1
end

function DoAltFire(i)
	if ( _PlayerIsKeyDown( i, IN_ATTACK2 ) == false ) then 
		if (PlayerData[i].IsAltFiring == 1) then
			_EntSetAng(i, PlayerData[i].Ang)
			PlayerData[i].IsAltFiring = 0
		end
		return; 
	end

	if (PlayerData[i].IsAltFiring == 0) then
		local vAng = _PlayerGetShootAng( i )
		PlayerData[i].Ang = vector3(vAng.x, vAng.y, vAng.z)

		local vAng2 = _EntGetUpVector( i )
		vAng2.z = -vAng2.z
		_EntSetAng(i, vAng2)

		PlayerData[i].IsAltFiring = 1
	end
end

function DoSpeed(i)
	if ( _PlayerIsKeyDown( i, IN_SPEED ) == false ) then 
		PlayerData[i].IsSpeedFiring = 0
		return; 
	end

	if (PlayerData[i].IsSpeedFiring == 0) then
		if (PlayerData[i].RunLeft < MAX_RUN) then
			return
		end
	end

	PlayerData[i].IsSpeedFiring = 1
end

function DoUpdateTarget( i )

	if (i == nil or PlayerData[i] == nil) then
		return
	end

	if (_PlayerInfo(i, "connected") ) then

		if (_PlayerInfo(i, "team") == TEAM_BIRDS) then

			if (PlayerData[i].IsSpeedFiring == 0) then
				PlayerData[i].RunLeft = PlayerData[i].RunLeft + 1

				if (PlayerData[i].RunLeft >= MAX_RUN) then
					PlayerData[i].RunLeft = MAX_RUN
				else
					DrawRunLeft(i)
				end
			else
				PlayerData[i].RunLeft = PlayerData[i].RunLeft - 1

				if (PlayerData[i].RunLeft < 0) then
					PlayerData[i].RunLeft = 0
					PlayerData[i].IsSpeedFiring = 0
				else
					DrawRunLeft(i)
				end
			end

			local vAimPos = _PlayerGetShootAng( i );

			if (PlayerData[i].IsAltFiring ~= 0) then
				vAimPos = PlayerData[i].Ang
			end

			local vVel = vector3(NORMAL_SPEED,NORMAL_SPEED,NORMAL_SPEED)

			_EntSetAng(PlayerData[i].Bird, vAimPos)

			if ( _PlayerIsKeyDown( i, IN_DUCK ) ) then

				if (PlayerData[i].IsSpeedFiring == 0) then
					vVel = vecMul(vAimPos, vector3(WALK_SPEED,WALK_SPEED,0))
					_EntSetActivity( PlayerData[i].Bird, ACT_WALK );
				else
					vVel = vecMul(vAimPos, vector3(FAST_WALK_SPEED,FAST_WALK_SPEED,0))
					_EntSetActivity( PlayerData[i].Bird, ACT_RUN );
				end
			else
				if (PlayerData[i].IsSpeedFiring == 0) then
					vVel = vecMul(vAimPos, vector3(NORMAL_SPEED,NORMAL_SPEED,NORMAL_SPEED))
					_EntSetActivity( PlayerData[i].Bird, ACT_FLY );
				else
					vVel = vecMul(vAimPos, vector3(FAST_SPEED,FAST_SPEED,FAST_SPEED))
					_EntSetActivity( PlayerData[i].Bird, ACT_FLY );
				end
			end

			_EntSetVelocity(PlayerData[i].Bird, vVel)
		end
	end
end

function DoControls()
	for i=1, _MaxPlayers() do
		if ( _PlayerInfo( i, "connected" ) and PlayerData[i] ~= nil ) then

			if (_PlayerInfo(i, "alive") and _PlayerInfo(i, "team") == TEAM_BIRDS) then

				DoFire(i)
				DoAltFire(i)
				DoSpeed(i)

			end

		end
	end
end

