

-- These variables are passed by the engine
doublezoom = 1
MyIndex			=	0; -- Weapon's entity index.
Owner			= 	0; -- The player that owns this weapon
CurrentTime		=	0; -- The current game time


-- Called when the weapon is created.
	
	function onInit( )
		local doublezoom = 1
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_m4a1.Silenced" );
		
	end

	
-- Called every frame

	function onThink( )
			
	end
	
	
-- 

	function onPrimaryAttack( )
		
		if ( _PlayerInfo( Owner, "alive" ) == false ) then return; end
				
		-- Make the player fly backwards
		 --local vPlayerVel = vecMul( _PlayerGetShootAng( Owner ), vector3(-2000, -2000, -2000) );
		-- _EntSetVelocity( Owner, vPlayerVel );
		
		
		-- Send the laser line effect
		
		
	   local vecpos = _PlayerGetShootPos( Owner );
	   local plyang = _PlayerGetShootAng( Owner );
	 
	   _TraceLine( vecpos, plyang, 10000, Owner );
	   
	   local hitpos = _TraceEndPos();
	 
	   if ( _TraceHit() == false )  then
	   
	   hitpos = vecAdd( vecpos, vecMul( plyang, vector3(8000,8000,8000) ) )
	   
	   end
	 
	   --local mycoolvar = string.format("%.2f",_TraceFraction)
	   --_Msg(mycoolvar)
	 --  vecpos = vecAdd( vecpos, vector3( 0, 0, -20 ) );

		--_EffectInit();
		
		--	_EffectSetEnt( Owner );
		--	_EffectSetOrigin( hitpos );
		--	_EffectSetStart( vecpos );
		--	_EffectSetScale( 15 );
	--		_EffectSetMagnitude( 3);
		
	--	_EffectDispatch( "FadingLineTeam" );
		
	--	_EffectSetMagnitude( 4 );
	--	_EffectSetScale( 9 );
	--	_EffectDispatch( "FadingLineTeam" );
		
			
	end
	
	
	-- Secondary gives ammo
	
	function onSecondaryAttack( )	
		-- Something now	
						 		if doublezoom == 1 then
				 					 
				 					 		 			_GModRect_Start("overlays/scope_lens");
			 _GModRect_SetPos( 0, 0, 1, 1);
			 _GModRect_SetColor(255, 255, 255, 255);
			 _GModRect_SetTime(999, 0, 5);
			_GModRect_Send(Owner, 65);		
		
			_GModRect_Start( "weapons/scopes/scope2" );
			 _GModRect_SetPos( 0, 0, 1, 1);
			 _GModRect_SetColor( 255, 255, 255, 255 );
			 _GModRect_SetTime(999, 0, 5);
			_GModRect_Send(Owner, 70);
			_PlayerSetFOV(Owner, 2.0, 0.5)
		 doublezoom = 0

		  else
		  
		  	 _GModRect_Hide(Owner, 65, 0, 0);
			_GModRect_Hide(Owner, 70, 0, 0);
			_PlayerStopZooming(Owner)
		  doublezoom = 1
		  end
		
	-- Nothing! Absolutely NOTHING! Hahahahahah! Stupid! You so stupiiiid!	
	end
	
	
			function Holster( )
					  	 _GModRect_Hide(Owner, 70, 0, 0);
			_GModRect_Hide(Owner, 65, 0, 0);
			_PlayerStopZooming(Owner)
		  doublezoom = 1
	end
	function onReload( )
					  _GModRect_Hide(Owner, 70, 0, 0);
			_GModRect_Hide(Owner, 65, 0, 0);
			_PlayerStopZooming(Owner)
		  doublezoom = 1
		return true;
			
	end
	
	
-- Weapon settings.
-- These are only accessed once when setting the weapon up
		function getHUDMaterial( )

		return "VGUI/gfx/VGUI/scout.vmt";

	end
	
	function getWeaponSwapHands()
		return true;	
	end
	
	function getWeaponFOV()
		return 74;	
	end
	
	function getWeaponSlot()
		return 1;	
	end
	
	function getWeaponSlotPos()
		return 2;	
	end
	
	function getFiresUnderwater()
		return true;
	end
	
	function getReloadsSingly()
		return false;
	end
	
	function getDamage()
		return 120;
	end
	
	function getPrimaryShotDelay()
		return 1.75;
	end
	
	function getSecondaryShotDelay()
		return 0.5;
	end
	
	function getPrimaryIsAutomatic()
		return true;
	end
	
	function getSecondaryIsAutomatic()
		return true;
	end
	
	function getBulletSpread()
		return vector3( 0.015, 0.015, 0.015 );
	end
	
	function getViewKick()
		return vector3( -0.0, 0.0, 0.0);
	end
	
	function getViewKickRandom()
		return vector3( 5, 5, 5 );
	end

	function getViewModel( )
		return "models/weapons/v_snip_scout.mdl";
	end
	
	function getWorldModel( )
		return "models/weapons/w_snip_scout.mdl";
	end
	
	function getClassName()
		return "weapon_normalsniper";
	end

	function getPrimaryAmmoType()
		return "357";
	end
		
	function getSecondaryAmmoType()
		return "357";
	end
	
	-- return -1 if it doesn't use clips
	function getMaxClipPrimary()
		return 8;
	end
	
	function getMaxClipSecondary()
		return -1;
	end
	
	-- ammo in gun by default
	function getDefClipPrimary()
		return 8;
	end
	
	function getDefClipSecondary()
		return -1;
	end

	-- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade
	function getAnimPrefix()
		return "smg";
	end

	function getPrintName()
		return "Silenced Sniper";
	end
	
	
	-- 0 = Don't override, shoot bullets, make sound and flash
	-- 1 = Don't shoot bullets but do make flash/sounds
	-- 2 = Only play animations
	-- 3 = Don't do anything
	
	function getPrimaryScriptOverride()
		return 0;
	end

	function getSecondaryScriptOverride()
		return 3;
	end

