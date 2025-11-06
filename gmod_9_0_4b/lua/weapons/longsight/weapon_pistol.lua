
-- These variables are passed by the engine

MyIndex			=	0; -- Weapon's entity index.
Owner			= 	0; -- The player that owns this weapon
CurrentTime		=	0; -- The current game time


-- Called when the weapon is created.
	
	function onInit( )
		
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_fiveseven.Single" );
		
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
	   vecpos = vecAdd( vecpos, vector3( 0, 0, -20 ) );

		--_EffectInit();
		
		--	_EffectSetEnt( Owner );
		--	_EffectSetOrigin( hitpos );
		--	_EffectSetStart( vecpos );
		--	_EffectSetScale( 15 );
		--	_EffectSetMagnitude( 3);
		
	--	_EffectDispatch( "FadingLineTeam" );
		
	--	_EffectSetMagnitude( 4 );
	--	_EffectSetScale( 9 );
		--_EffectDispatch( "FadingLineTeam" );
		
			
	end
	
	
	-- Secondary gives ammo
	
	function onSecondaryAttack( )		
		
		
	-- Nothing! Absolutely NOTHING! Hahahahahah! Stupid! You so stupiiiid!	
	end
	
	
	function onReload( )
			
		return true;
			
	end
	
	
	
-- Weapon settings.
-- These are only accessed once when setting the weapon up
	
	function getWeaponSwapHands()
		return true;	
	end
	
	function getWeaponFOV()
		return 74;	
	end
	
	function getWeaponSlot()
		return 2;	
	end
	
	function getWeaponSlotPos()
		return 5;	
	end
	
	function getFiresUnderwater()
		return true;
	end
	
	function getReloadsSingly()
		return false;
	end
	
	function getDamage()
		return 60;
	end
	
	function getPrimaryShotDelay()
		return 0.125;
	end
	
	function getSecondaryShotDelay()
		return 0.1;
	end
	
	function getPrimaryIsAutomatic()
		return false;
	end
	
	function getSecondaryIsAutomatic()
		return false;
	end
	
	function getBulletSpread()
		return vector3( 0.05, 0.05, 0.05 );
	end
	
	function getViewKick()
		return vector3( -0.0, 0.0, 0.0);
	end
	
	function getViewKickRandom()
		return vector3( 1, 1, 1 );
	end

	function getViewModel( )
		return "models/weapons/v_pist_p228.mdl";
	end
	
	function getWorldModel( )
		return "models/weapons/w_pist_p228.mdl";
	end
	
	function getClassName()
		return "weapon_mypistol";
	end

	function getPrimaryAmmoType()
		return "357";
	end
				function getHUDMaterial( )

		return "VGUI/gfx/VGUI/p228.vmt";

	end
	function getSecondaryAmmoType()
		return "357";
	end
	
	-- return -1 if it doesn't use clips
	function getMaxClipPrimary()
		return 10;
	end
	
	function getMaxClipSecondary()
		return -1;
	end
	
	-- ammo in gun by default
	function getDefClipPrimary()
		return 10;
	end
	
	function getDefClipSecondary()
		return 0;
	end

	-- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade
	function getAnimPrefix()
		return "pistol";
	end

	function getPrintName()
		return "Secondary Handgun";
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

