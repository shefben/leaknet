----------------------------------------------------------------------------------

-- This is an example gameplay override script

----------------------------------------------------------------------------------



_Msg("--------------------------------------------------------\n")

_Msg("-- gm_laserdance ---------------------------------------\n")

_Msg("--------------------------------------------------------\n")





iPlayerLastPosition = {}

fNextTrailLine = 0;



	function gamerulesThink ()





		

		if (fNextTrailLine > _CurTime() ) then return; end;

			

		for i=0, _MaxPlayers() do



			-- remove this 'false and' to enable player trails.

			if ( _PlayerInfo( i, "alive" ) ) then

				

				_EffectInit();

				

					_EffectSetEnt( i ); 

					_EffectSetOrigin( vecAdd( _EntGetPos( i ), vector3( 0, 0, 30) )  );

					_EffectSetStart( vecAdd( iPlayerLastPosition[i], vector3( 0, 0, 30) ) );

					_EffectSetScale( 20 );

					_EffectSetMagnitude( 10 );

				

				_EffectDispatch( "FadingLineTeamSolid" );

				

				iPlayerLastPosition[i] = _EntGetPos( i );

				

			end



		end

		

		

		fNextTrailLine = _CurTime() + 0.10;



		



	end 

	

	

	

	function GiveDefaultItems( playerid )

		

		

		-- spectators don't get any weapons

		if ( _PlayerInfo( playerid, "team" ) == TEAM_SPECTATOR ) then return; end;

		

		_PlayerGiveSWEP( playerid, "weapons/gm_laserdance/weapon_laser.lua" );

		_PlayerGiveAmmo( playerid, 1000, "357", false );

		

	end

		

	

--  Called right before the new map starts ------------------------------------



	          

	function gamerulesStartMap ()

	

		PlayerFreezeAll( false )

		



		_TeamSetName( TEAM_BLUE, "Blue Team" )

		_TeamSetName( TEAM_GREEN, "Green Team" )

		

		-- Precache the models 



		_EntPrecacheModel( "models/player/combine_super_soldier.mdl" );

		_EntPrecacheModel( "models/player/stalker.mdl" );

			

	end

	







	function PlayerSpawnChooseModel ( playerid )	

	

	

		if ( _PlayerInfo( playerid, "team" ) == TEAM_BLUE )  then

			

			_PlayerSetModel( playerid, "models/player/combine_super_soldier.mdl" )



		else

		

			_PlayerSetModel( playerid, "models/player/stalker.mdl" )



		end

				

	end

	

	

	function PickDefaultSpawnTeam( userid )

		

		_PlayerChangeTeam( userid, TEAM_SPECTATOR );

		_Msg("LUA!: Changing team!\n");

		return true;

		

	end



	

-- Event overrides



	

	function eventPlayerKilled ( killed, attacker, weapon )

		

		_PlayerAddDeath( killed, 1 )

		_PlayerAddScore( attacker, 1 )

		-- Annoying "killing spree" stuff?

		

	end

	

	

	function canPlayerHaveItem( playerid, itemname )

			

		if ( itemname == "weapon_lasergun" ) then return true; end

		

		return false

		

	end

	

	

	

	function eventPlayerSpawn ( userid )

		

		-- If they're a spectator show them the team choice menu

		if ( _PlayerInfo( userid, "team" ) == TEAM_SPECTATOR ) then

			

			onShowTeam( userid );

			

		end

		

		

		iPlayerLastPosition[userid] = _EntGetPos( userid );

		

		_PlayerSetDrawTeamCircle( userid, true )



	end

	

	

-- These are called by the players in game using the F1 - F4 keys

	

	function onShowHelp ( userid )

		

		_GModText_Start( "Default" );

		 _GModText_SetPos( -1, 0.3 );

		 _GModText_SetColor( 255, 255, 255, 255 );

		 _GModText_SetTime( 4, 0.2, 1 );

		 _GModText_SetText( "The object of Laser Dance is to kill the other team.\n\nGive teammates health by pressing the right mouse button." );

		_GModText_Send( userid, 50 );



	end

	

	function onShowTeam ( userid )

		

		_PlayerOption( userid, "ChooseTeam", 99999 );

		

		-- Big yellow title

		_GModText_Start( "ImpactMassive" );

		 _GModText_SetPos( 0.9, 0.34 );

		 _GModText_SetColor( 255, 200, 0, 205 );

		 _GModText_SetTime( 99999, 1.5, 0 );

		 _GModText_SetText( "gm_laserdance" );

		 _GModText_SetDelay( 1.5 );

		 _GModText_AllowOffscreen( true ); -- Let the text hang off the screen

		_GModText_Send( userid, 0 );

		

		_GModText_Start( "ImpactMassive" );

		 _GModText_SetPos( 0.6, 0.34 );

		 _GModText_SetColor( 255, 200, 0, 205 );

		 _GModText_SetDelay( 1.0 );

		_GModText_SendAnimate( userid, 0, 1.5, 0.7 );

				

		-- tiny credit line

		_GModText_Start( "HudHintTextLarge" );

		 _GModText_SetPos( 0.79, 0.43 );

		 _GModText_SetColor( 255, 255, 255, 255 );

		 _GModText_SetTime( 99999, 0.5, 1.5 );

		 _GModText_SetText( "by Garry Newman" );

		 _GModText_SetDelay( 1.0 );

		_GModText_Send( userid, 1 );

		

		-- background black square (white is changed black using SetColor)

		_GModRect_Start( "gmod/white" );

		 _GModRect_SetPos( 0.0, 0.3, 1, 0.2 );

		 _GModRect_SetColor( 255, 255, 255, 255 );

		 _GModRect_SetTime( 99999, 0.5, 2 );

		 _GModRect_SetDelay( 1 );

		_GModRect_Send( userid, 0 );

		

		_GModRect_Start( "gmod/white" );

		 _GModRect_SetPos( 0.0, 0.3, 1, 0.2 );

		 _GModRect_SetColor( 0, 0, 0, 150 );

		 _GModRect_SetDelay( 1 );

		_GModRect_SendAnimate( userid, 0, 0.5, 0.5);

		

		-- top line

		_GModRect_Start( "gmod/white" );

		 _GModRect_SetPos( 0.0, 0.3, 0, 0.002 );

		 _GModRect_SetColor( 0, 0, 255, 255 );

		 _GModRect_SetTime( 99999, 0.5, 2 );

		_GModRect_Send( userid, 1 );

		

		_GModRect_Start( "gmod/white" );

		 _GModRect_SetPos( 0.0, 0.3, 1, 0.002 );

		 _GModRect_SetColor( 0, 0, 0, 255 );

		_GModRect_SendAnimate( userid, 1, 0.7, 0.2 );

		

		-- bottom line

		_GModRect_Start( "gmod/white" );

		 _GModRect_SetPos( 1.0, 0.4998, 0, 0.002 );

		 _GModRect_SetColor( 0, 0, 255, 255 );

		 _GModRect_SetTime( 99999, 0.5, 2 );

		_GModRect_Send( userid, 3 );

		

		_GModRect_Start( "gmod/white" );

		 _GModRect_SetPos( 0.0, 0.4998, 1, 0.002 );

		 _GModRect_SetColor( 0, 0, 0, 255 );

		_GModRect_SendAnimate( userid, 3, 0.7, 0.3 );

		

		

		-- shadow gradient

		_GModRect_Start( "gmod/white-grad-down" );

		 _GModRect_SetPos( 0.0, 0.4998, 1, 0.00 );

		 _GModRect_SetColor( 0, 0, 0, 100 );

		 _GModRect_SetTime( 99999, 0.5, 2 );

		_GModRect_Send( userid, 4 );

		

		_GModRect_Start( "gmod/white-grad-down" );

		 _GModRect_SetPos( 0.0, 0.4998, 1, 0.05 );

		 _GModRect_SetColor( 0, 0, 0, 100 );

		 _GModRect_SetTime( 99999, 0.5, 2 );

		_GModRect_SendAnimate( userid, 4, 2, 0.5 );

		

		-- Options

		_GModText_Start( "Default" );

		 _GModText_SetPos( 0.01, 0.33 );

		 _GModText_SetColor( 255, 255, 255, 255 );

		 _GModText_SetTime( 99999, 3, 1.0 );

		 _GModText_SetText( "Choose your team:\n\n\n1. Blue Team\n2. Yellow Team\n\n5. Auto" );

		 _GModText_SetDelay( 1.5 );

		_GModText_Send( userid, 2 );

		

		_GModText_Start( "Default" );

		 _GModText_SetPos( 0.06, 0.33 );

		 _GModText_SetColor( 255, 255, 255, 255 );

		 _GModText_SetDelay( 1.5 );

		_GModText_SendAnimate( userid, 2, 1.0, 0.7 );	



		



	end

	

	function onShowSpare1 ( userid )



	end

	

	function onShowSpare2 ( userid )



	end

	

	

-- Player option callbacks (Handles input from player options)

	

	function ChooseTeam( playerid, num, seconds )

		

		_GModText_Hide( playerid, 0, 0.5 );

		_GModText_Hide( playerid, 1, 0.5 );

		_GModText_Hide( playerid, 2, 0.5 );

		

		_GModRect_Hide( playerid, 0, 0.9 );

		_GModRect_Hide( playerid, 1, 0.9 );

		_GModRect_Hide( playerid, 2, 0.9 );

		_GModRect_Hide( playerid, 3, 0.9 );

		_GModRect_Hide( playerid, 4, 0.9 );

		

		if (num == 1) then

			

			if ( _PlayerInfo( playerid, "team" ) == TEAM_BLUE ) then return; end;

			

			_PlayerChangeTeam( playerid, TEAM_BLUE );

			_PlayerHolsterWeapon( playerid );

			_EntSpawn( playerid );

			

			return;

			

		end

		

		if (num == 2) then

			

			if ( _PlayerInfo( playerid, "team" ) == TEAM_YELLOW ) then return; end;

			

			_PlayerChangeTeam( playerid, TEAM_YELLOW );

			_PlayerHolsterWeapon( playerid )

			_EntSpawn( playerid );

			

			return;

			

		end

		

		-- anything else is auto choose team



		if ( _TeamNumPlayers( TEAM_YELLOW ) > _TeamNumPlayers( TEAM_BLUE ) ) then

			

			ChooseTeam( playerid, 1, 0 );

			

		else

		

			ChooseTeam( playerid, 2, 0 );

		

		end

		

	end

