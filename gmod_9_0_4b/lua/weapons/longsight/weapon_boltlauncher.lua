
-- These variables are passed by the engine
doublezoom = 1
MyIndex			=	0; -- Weapon's entity index.
Owner			= 	0; -- The player that owns this weapon
CurrentTime		=	0; -- The current game time


-- Called when the weapon is created.
	
	function onInit( )
		
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_galil.Single" );
		local doublezoom = 0
		doublezoom = 1
	end

	
			function getHUDMaterial( )

		return "VGUI/gfx/VGUI/awp.vmt";

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
	   --]vecpos = vecAdd( vecpos, vector3( 0, 0, -20 ) );
		local iBolt = _EntCreate( "crossbow_bolt" );
		
		if (iBolt > 0) then
			
			_EntSetKeyValue( iBolt, "damage", "500");
			_EntSetPos( iBolt,_PlayerGetShootPos( Owner ) );
			_EntSetAng( iBolt, _PlayerGetShootAng( Owner ) );
			_EntSetOwner( iBolt, Owner );
			local vVelocity = vecMul( _PlayerGetShootAng( Owner ), vector3(3000, 3000, 3000) );
			_EntSetVelocity( iBolt, vVelocity );
			_EntSetAng(iBolt, _PlayerGetShootAng( Owner ))
				
			_EntSpawn( iBolt );
			_SWEPUseAmmo( MyIndex, 0, 1 );
			--local vVelocity = vecMul( _PlayerGetShootAng( Owner ), vector3(3900, 3900, 3900) );
			--_EntSetVelocity( iBolt, vVelocity );
		end
	--	_EffectInit();
		
		--	_EffectSetEnt( Owner );
		--	_EffectSetOrigin( hitpos );
		--	_EffectSetStart( vecpos );
		--	_EffectSetScale( 15 );
		--	_EffectSetMagnitude( 3);
		
		--_EffectDispatch( "FadingLineTeam" );
		
		--_EffectSetMagnitude( 4 );
		--_EffectSetScale( 9 );
		--_EffectDispatch( "FadingLineTeam" );
		
			
	end
	
	
	-- Secondary gives ammo
	function zoomin()
		end
		function zoomout()
			end
	function onSecondaryAttack( )
				 		if doublezoom == 1 then
				 					 
		 doublezoom = 0
		 			_GModRect_Start("overlays/scope_lens");
			 _GModRect_SetPos( 0, 0, 1, 1);
			 _GModRect_SetColor(255, 255, 255, 255);
			 _GModRect_SetTime(999, 0, 5);
			_GModRect_Send(Owner, 64);		
		
			_GModRect_Start( "weapons/scopes/scope2" );
			 _GModRect_SetPos( 0, 0, 1, 1);
			 _GModRect_SetColor( 255, 255, 255, 255 );
			 _GModRect_SetTime(999, 0, 5);
			_GModRect_Send(Owner, 72);
			_PlayerSetFOV(Owner, 2.0, 0.5)

		  else
		  
		  _GModRect_Hide(Owner, 64, 0, 0);
			_GModRect_Hide(Owner, 72, 0, 0);
			_PlayerStopZooming(Owner)
		  doublezoom = 1
end


--		 if doublezoom == 1 then doublezoom = 0 elseif doublezoom == 0 then doublezoom = 1 end
	-- Nothing! Absolutely NOTHING! Hahahahahah! Stupid! You so stupiiiid!	
	end
	
			function Holster( )
					  	 _GModRect_Hide(Owner, 64, 0, 0);
			_GModRect_Hide(Owner, 72, 0, 0);
			_PlayerStopZooming(Owner)
		  doublezoom = 1
	end
	function onReload( )
					  _GModRect_Hide(Owner, 64, 0, 0);
			_GModRect_Hide(Owner, 72, 0, 0);
			_PlayerStopZooming(Owner)
		  doublezoom = 1
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
		return 1;	
	end
	
	function getWeaponSlotPos()
		return 1;	
	end
	
	function getFiresUnderwater()
		return true;
	end
	
	function getReloadsSingly()
		return false;
	end
	
	function getDamage()
		return 0;
	end
	
	function getPrimaryShotDelay()
		return 1.25;
	end
	
	function getSecondaryShotDelay()
		return .5;
	end
	
	function getPrimaryIsAutomatic()
		return true;
	end
	
	function getSecondaryIsAutomatic()
		return true;
	end
	
	function getBulletSpread()
		return vector3( 0.0155, 0.0155, 0.0155 );
	end
	
	function getViewKick()
		return vector3( -0.0, 0.0, 0.0);
	end
	
	function getViewKickRandom()
		return vector3( 0.5, 0.5, 0.5 );
	end

	function getViewModel( )
		return "models/weapons/v_snip_awp.mdl";
	end
	
	function getWorldModel( )
		return "models/weapons/w_snip_awp.mdl";
	end
	
	function getClassName()
		return "weapon_boltsniper";
	end

	function getPrimaryAmmoType()
		return "357";
	end
		
	function getSecondaryAmmoType()
		return "357";
	end
	
	-- return -1 if it doesn't use clips
	function getMaxClipPrimary()
		return 15;
	end
	
	function getMaxClipSecondary()
		return 1;
	end
	
	-- ammo in gun by default
	function getDefClipPrimary()
		return 15;
	end
	
	function getDefClipSecondary()
		return 1;
	end

	-- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade
	function getAnimPrefix()
		return "smg";
	end

	function getPrintName()
		return "Bolt Sniper";
	end
	
	
	-- 0 = Don't override, shoot bullets, make sound and flash
	-- 1 = Don't shoot bullets but do make flash/sounds
	-- 2 = Only play animations
	-- 3 = Don't do anything
	
	function getPrimaryScriptOverride()
		return 1;
	end

	function getSecondaryScriptOverride()
		return 3;
	end

