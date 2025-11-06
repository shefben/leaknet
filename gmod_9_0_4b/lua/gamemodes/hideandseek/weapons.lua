--##############################
--## Hide and Seek
--## fayte, gdhughes@gmail.com
--##############################

function ShootPrimaryCatcher( ownerid, index ) 

	local vecpos = _PlayerGetShootPos( ownerid );
  	local plyang = _PlayerGetShootAng( ownerid );
  	_TraceLine( vecpos, plyang, 4096, ownerid );
	
	if ( not _PlayerInfo( ownerid, "alive" ) ) then return; end
	if ( not _TraceHit() ) then return; end
	if ( _EntGetType( _TraceGetEnt() ) ~= "player" ) then return; end
	if ( _PlayerInfo( _TraceGetEnt(), "team" ) ~= TEAM_HIDERS ) then return; end
	if ( _PlayerInfo( ownerid, "team" ) ~= TEAM_SEEKERS ) then return; end
	
	HSNumFound = HSNumFound + 1
	HSIsFound[ _TraceGetEnt() ] = true

	_PrintMessage( _TraceGetEnt(), 4, "You were found!" );
	_PrintMessageAll( 3, _PlayerInfo(_TraceGetEnt(), "name") .. " was found by " .. _PlayerInfo(ownerid, "name") .. "!" );

	_TeamAddScore( TEAM_SEEKERS, 8 );

	_PlayerSilentKill( _TraceGetEnt(), 1, true );
	
	DrawScores( 0 );
	
end

function ShootPrimarySuperCatcher( ownerid, index ) 

	if ( not _PlayerInfo( ownerid, "alive" ) ) then return; end
	if ( _PlayerInfo( ownerid, "team" ) ~= TEAM_SEEKERS ) then return; end

	local vecpos = _PlayerGetShootPos( ownerid );
  	local plyang = _PlayerGetShootAng( ownerid );
  	_TraceLine( vecpos, plyang, 4096, ownerid );

	local hitpos = _TraceEndPos();

	if ( not _TraceHit() ) then 
		local hitpos = vecAdd( vecpos, vecMul( plyang, vector3(4096,4096,4096) ) );
	end
	
	local newvel = vecMul( plyang, vector3(-800, -800, -800) ); 
	_EntSetVelocity( ownerid, newvel );

	_EffectInit(); 	
	_EffectSetEnt( ownerid ); 
	_EffectSetOrigin( hitpos ); 	
	_EffectSetStart( vecpos ); 
	_EffectSetScale( 15 ); 	
	_EffectSetMagnitude( 3); 
	_EffectDispatch( "FadingLineTeam" );
	_EffectSetMagnitude( 4 ); 
	_EffectSetScale( 9 ); 	
	_EffectDispatch( "FadingLineTeam" );

	if ( _EntGetType( _TraceGetEnt() ) ~= "player" ) then return; end
	if ( _PlayerInfo( _TraceGetEnt(), "team" ) ~= TEAM_HIDERS ) then return; end

	HSNumFound = HSNumFound + 1
	HSIsFound[ _TraceGetEnt() ] = true

	_PrintMessage( _TraceGetEnt(), 4, "You were found!" );
	_PrintMessageAll( 3, _PlayerInfo(_TraceGetEnt(), "name") .. " was found by " .. _PlayerInfo(ownerid, "name") .. " with the SuperCatcher 6000!" );

	_TeamAddScore( TEAM_SEEKERS, 10 );
	_PlayerSilentKill( _TraceGetEnt(), 1, true );
	
	DrawScores( 0 );

end