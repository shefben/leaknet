

-- Called when the weapon is created.
	
	function onInit( )
		
		_SWEPSetSound( MyIndex, "single_shot", "NPC_Sniper.FireBullet" );
		
	end

	
-- Called every frame

	function onThink( )
			
	end
	
	
-- 

	function onPrimaryAttack( )
		
		if ( _PlayerInfo( Owner, "connected" ) == false ) then return; end
		if ( _PlayerInfo( Owner, "alive" ) == false ) then return; end
				
		-- Make the player fly backwards
		 local vPlayerAng = _PlayerGetShootAng( Owner );
		 if (vPlayerAng == nil) then _Msg("Playerang was null!\n") return; end;
		 	
		 local vPlayerVel = vecMul( vPlayerAng, vector3(-2000, -2000, -2000) );
		 _EntSetVelocity( Owner, vPlayerVel );
		
		
		-- Send the laser line effect
		
		
	   local vecpos = _PlayerGetShootPos( Owner );
	   local plyang = _PlayerGetShootAng( Owner );
	 
	   _TraceLine( vecpos, plyang, 8000, Owner );
	   
	   local hitpos = _TraceEndPos();
	 
	   if ( _TraceHit() == false )  then
	   
	   hitpos = vecAdd( vecpos, vecMul( plyang, vector3(8000,8000,8000) ) )
	   
	   end
	 
	   
	   
	   vecpos = vecAdd( vecpos, vector3( 0, 0, -20 ) );

		_EffectInit();
		
			_EffectSetEnt( Owner );
			_EffectSetOrigin( hitpos );
			_EffectSetStart( vecpos );
			_EffectSetScale( 15 );
			_EffectSetMagnitude( 3);
		
		_EffectDispatch( "FadingLineTeam" );
		
		_EffectSetMagnitude( 4 );
		_EffectSetScale( 9 );
		_EffectDispatch( "FadingLineTeam" );
		
			
	end
	
	
	-- Secondary gives ammo
	
	function onSecondaryAttack( )		
		
		
	   local vecpos = _PlayerGetShootPos( Owner );
	   local plyang = _PlayerGetShootAng( Owner );
	 
	   _TraceLine( vecpos, plyang, 8000, Owner );
	   
	   vecpos = vecAdd( vecpos, vector3( 0, 0, -20 ) );
	   
	   local hitpos = _TraceEndPos();
	 
	   if ( _TraceHit() == false )  then
	   
	   hitpos = vecAdd( vecpos, vecMul( plyang, vector3(8000,8000,8000) ) )
	   
	   end
	 
	   

		_EffectInit();
		
			_EffectSetEnt( 255 ); -- green
			
			_EffectSetOrigin( hitpos );
			_EffectSetStart( vecpos );
			_EffectSetScale( 20 );
			_EffectSetMagnitude( 0.5 );
		
		_EffectDispatch( "FadingLine" );
		
		if ( _TraceHitNonWorld() ) then
		
			local iHealth = _PlayerInfo( _TraceGetEnt(), "health" );
			
			if (iHealth == 149) then

				_EntEmitSound( _TraceGetEnt(), "hl1/fvox/medical_repaired.wav" );
			
			end
			
			iHealth = iHealth + 1;
			
			if (iHealth > 150) then return; end
			
			_PlayerSetHealth( _TraceGetEnt(), iHealth );
			local Pitch = (iHealth / 150.0) * 0.50 + 0.80;
			_EntEmitSoundEx( _TraceGetEnt(), "items/medshot4.wav", 1.0, Pitch );
		
		end

					
	end
	
	
	function onReload( )
			
		return true;
			
	end
	
	
	
-- Weapon settings.
-- These are only accessed once when setting the weapon up
	
	function getWeaponSwapHands()
		return false;	
	end
	
	function getWeaponFOV()
		return 74;	
	end
	
	function getWeaponSlot()
		return 1;	
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
		return 120;
	end
	
	function getPrimaryShotDelay()
		return 0.5;
	end
	
	function getSecondaryShotDelay()
		return 0.1;
	end
	
	function getPrimaryIsAutomatic()
		return true;
	end
	
	function getSecondaryIsAutomatic()
		return true;
	end
	
	function getBulletSpread()
		return vector3( 0.0, 0.0, 0.0 );
	end
	
	function getViewKick()
		return vector3( -0.0, 0.0, 0.0);
	end
	
	function getViewKickRandom()
		return vector3( 0.5, 0.5, 0.5 );
	end
	
	
	function getBulletSpreadSecondary()
		return vector3( 0.001, 0.001, 0.001 );
	end
	
	function getViewKickSecondary()
		return vector3( 0.0, 0.0, 0.0);
	end
	
	function getViewKickRandomSecondary()
		return vector3( 0.0, 0.0, 0.0 );
	end
	

	function getViewModel( )
		return "models/weapons/v_rif_famas.mdl";
	end
	
	function getWorldModel( )
		return "models/weapons/w_rif_famas.mdl";
	end
	
	function getClassName()
		return "weapon_lasergun";
	end

	function getPrimaryAmmoType()
		return "357";
	end
		
	function getSecondaryAmmoType()
		return "357";
	end
	
	-- return -1 if it doesn't use clips
	function getMaxClipPrimary()
		return 500;
	end
	
	function getMaxClipSecondary()
		return 10;
	end
	
	-- ammo in gun by default
	function getDefClipPrimary()
		return 500;
	end
	
	function getDefClipSecondary()
		return 10;
	end

	-- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade
	function getAnimPrefix()
		return "smg";
	end

	function getPrintName()
		return "Laser Nonce";
	end
	
	function getHUDMaterial( )
		return "gmod/SWEP/weapon_laserdance";
	end
	
	
	-- 0 = Don't override, shoot bullets, make sound and flash
	-- 1 = Don't shoot bullets but do make flash/sounds
	-- 2 = Only play animations
	-- 3 = Don't do anything
	
	function getPrimaryScriptOverride()
		return 0;
	end

	function getSecondaryScriptOverride()
		return 2;
	end

