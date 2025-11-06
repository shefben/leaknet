
function cc_onShowTeam ( fromplayer, args )

	onShowTeam( fromplayer )

end
CONCOMMAND( "changeteam", cc_onShowTeam )


function cc_changebirdtype ( fromplayer, args )

	ClearPlayerData( fromplayer, 0 )

	onChangeBirdType(fromplayer)

end
CONCOMMAND( "changebirdtype", cc_changebirdtype )

function domenu( playerid, num, seconds )
	_GModText_Hide( playerid, 54, 0.25, 0 );

	if ( num == 1 ) then 		cc_onShowTeam(playerid)
	elseif ( num == 2 ) then	cc_changebirdtype(playerid)
	end
end

function cc_menu ( fromplayer, args )

	_GModText_Start( "ImpactMassive" );
	 _GModText_SetPos( -1, -1 );
	 _GModText_SetColor( 0, 0, 0, 255 );
	 _GModText_SetTime( 99999, 0.25, 1 );
	 _GModText_SetText( "Menu:\n1. Change Team\n2. Change Bird Type\n3. Cancel" );
	 _GModText_SetDelay( 1 );
	_GModText_Send( fromplayer, 54 );

	_PlayerOption( fromplayer, "domenu", 99999 );
end
CONCOMMAND( "menu", cc_menu )

function cc_draw_line_on( fromplayer, args )		DRAW_POOP_LINE = 1	end
function cc_draw_line_off( fromplayer, args )		DRAW_POOP_LINE = 0	end
CONCOMMAND("draw_poop_line_on", cc_draw_line_on)
CONCOMMAND("draw_poop_line_off", cc_draw_line_off)


