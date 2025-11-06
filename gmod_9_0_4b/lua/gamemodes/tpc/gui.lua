



	-- Rect index



	-- Text index
	-- 0  : Select Team
	-- 1  : Money
	-- 2  : Buy Menu
	-- 3  : Buy Menu
	-- 10 : Intro Text



	function DrawIntro( PlayerID )

		_GModText_Start( "ImpactMassive" );
		 _GModText_SetPos( 0.6, 0.2 );
		 _GModText_SetColor( 50, 80, 250, 255 );
		 _GModText_SetTime( 5, 2, 2 );
		 _GModText_SetText( "Tactical Police Cops!" );
		_GModText_Send( PlayerID, 10 );	

	end



		

	function DrawTeamChoose( PlayerID )
		
		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.01, 0.33 );
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetTime( 99999, 2, 1.0 );
		 _GModText_SetText( "Choose your team:\n\n\n1. Team Liberation\n2. Team Freedom\n\n5. Auto" );
		_GModText_Send( PlayerID, 0 );

		_GModText_Start( "Default" );
		 _GModText_SetPos( 0.06, 0.33 );
		 _GModText_SetColor( 255, 255, 255, 255 );
		 _GModText_SetDelay( 1.5 );
		_GModText_SendAnimate( PlayerID, 0, 1.0, 0.7 );		

	end
	
	-- called on spawn and any time their cash changes
	function DrawCash( PlayerID )
		
		_GModText_Start( "Default" );
		 _GModText_SetPos( -1.0, -0.05 );
		 _GModText_SetColor( 50, 255, 50, 255 );
		 _GModText_SetTime( 99999, 0, 0 );
		 _GModText_SetText( "$" .. PlayerInfo[PlayerID].Money );
		_GModText_Send( PlayerID, 1 );	

	end
	
	function HideTeamMenu( PlayerID )
		
		_GModText_Hide( PlayerID, 0 );
		
	end

	function HideBuyMenu( PlayerID )
		
		_GModText_Hide( PlayerID, 2 );
		_GModText_Hide( PlayerID, 3 );
		
	end

	
	function DrawPowerUps( PlayerID )
		
		_GModText_Start( "TargetID" );
		 _GModText_SetPos( 0.1, -1 );
		 _GModText_SetColor( 50, 255, 50, 255 );
		 _GModText_SetTime( 10, 0, 0 );
		 _GModText_SetText( "1: Full Health\n2. Light Armour\n3. Heavy Armour\n\n4. Give Money" );
		_GModText_Send( PlayerID, 2 );	
		
		_GModText_Start( "TargetID" );
		 _GModText_SetPos( 0.3, -1 );
		 _GModText_SetColor( 255, 200, 50, 255 );
		 _GModText_SetTime( 10, 0, 0 );
		 _GModText_SetText( "$100\n$200\n$350\n\n.." );
		_GModText_Send( PlayerID, 3 );	

	end
	
	function DrawWeaponMenu( PlayerID )
		
		_GModText_Start( "TargetID" );
		 _GModText_SetPos( 0.1, -1 );
		 _GModText_SetColor( 50, 255, 50, 255 );
		 _GModText_SetTime( 10, 0, 0 );
		 _GModText_SetText( "1: USP Pistol\n2. Desert Eagle\n3. Shotgun\n4. Mac10\n5. MP5\n6. P90\n7. AWP" );
		_GModText_Send( PlayerID, 2 );	
		
		_GModText_Start( "TargetID" );
		 _GModText_SetPos( 0.3, -1 );
		 _GModText_SetColor( 255, 200, 50, 255 );
		 _GModText_SetTime( 10, 0, 0 );
		 _GModText_SetText( "$30\n$100\n$250\n$270\n$300\n$300\n$450" );
		_GModText_Send( PlayerID, 3 );	

	end
