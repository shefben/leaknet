

-- These variables are passed by the engine
doublezoom = 1
MyIndex			=	0; -- Weapon's entity index.
Owner			= 	0; -- The player that owns this weapon
CurrentTime		=	0; -- The current game time


-- Called when the weapon is created.
	
	function onInit( )
		local doubleshot = 0
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_g3sg1.Single" );
		doublezoom = 1
	end

	
-- Called every frame

	function onThink( )
				_GModText_Hide(i, 25, 0)	
					   local vecpos2 = _PlayerGetShootPos( Owner );
	   local plyang2 = _PlayerGetShootAng( Owner );
		_TraceLine( vecpos2, plyang2, 10000, Owner );
	  	   if ( _TraceHit() == false )  then
	   
	   hitpos = vecAdd( vecpos2, vecMul( plyang, vector3(8000,8000,8000) ) )
	   
	   else
	 --  local hitpos2 = _TraceEndPos();
	--	local thelength12 = vecLength( vecSub( vecpos2, hitpos2 ) )
	--	local thelength2 = thelength12 / 30
		--					 _GModText_Start( "HudHintTextLarge" );
		-- _GModText_SetPos( 1, -.4, .0 ); -- x, y
		-- _GModText_SetColor( 25, 255, 25, 255 );
		-- _GModText_SetTime( 2, 0, 0 );
		-- _GModText_SetText( "Length: " .. thelength2 );
		-- _GModText_SetDelay( 0 );
					--_GModText_Send( Owner, 25 );
	if ( _TraceHitNonWorld() ) then
		_PlaySoundPlayer( Owner, "weapons/grenade/tick1.wav" )
		
			 --  _ScreenText( Owner, "Fire!", 1.75, -.45, 15, 15, 255, 255, 1, 1, 1, 0, 5 )
			--   _ScreenText( Owner, "Length: " .. thelength2 .. " feet", 1.75, -.50, 255, 15, 15, 255, 1, 1, 1, 0, 5 )
		end
		end
								 		
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
						 		if doublezoom == 1 then
				 					 
				 					 		 			_GModRect_Start("overlays/scope_lens");
			 _GModRect_SetPos( 0, 0, 1, 1);
			 _GModRect_SetColor(255, 255, 255, 255);
			 _GModRect_SetTime(999, 0, 5);
			_GModRect_Send(Owner, 66);		
		
			_GModRect_Start( "weapons/scopes/scope2" );
			 _GModRect_SetPos( 0, 0, 1, 1);
			 _GModRect_SetColor( 255, 255, 255, 255 );
			 _GModRect_SetTime(999, 0, 5);
			_GModRect_Send(Owner, 71);
			_PlayerSetFOV(Owner, 2.0, 0.5)
		 doublezoom = 0

		  else
		  
		  	 _GModRect_Hide(Owner, 66, 0, 0);
			_GModRect_Hide(Owner, 71, 0, 0);
			_PlayerStopZooming(Owner)
		  doublezoom = 1
		  end
	-- Nothing! Absolutely NOTHING! Hahahahahah! Stupid! You so stupiiiid!	
	end
		function Holster( )
					  	 _GModRect_Hide(Owner, 66, 0, 0);
			_GModRect_Hide(Owner, 71, 0, 0);
			_PlayerStopZooming(Owner)
		  doublezoom = 1
	end
	
	function onReload( )
					  	 _GModRect_Hide(Owner, 66, 0, 0);
			_GModRect_Hide(Owner, 71, 0, 0);
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
		return 5;	
	end
	
	function getFiresUnderwater()
		return true;
	end
	
	function getReloadsSingly()
		return false;
	end
	
	function getDamage()
		return 150;
	end
	
	function getPrimaryShotDelay()
		return 4.0;
	end
	
	function getSecondaryShotDelay()
		return 0.5;
	end
			function getHUDMaterial( )

		return "VGUI/gfx/VGUI/g3sg1.vmt";

	end
	function getPrimaryIsAutomatic()
		return false;
	end
	
	function getSecondaryIsAutomatic()
		return false;
	end
	
	function getBulletSpread()
		return vector3( 0.0, 0.0, 0.0 );
	end
	
	function getViewKick()
		return vector3( -0.0, 0.0, 0.0);
	end
	
	function getViewKickRandom()
		return vector3( 5, 5, 5 );
	end

	function getViewModel( )
		return "models/weapons/v_snip_g3sg1.mdl";
	end
	
	function getWorldModel( )
		return "models/weapons/w_snip_g3sg1.mdl";
	end
	
	function getClassName()
		return "weapon_tracedsniper";
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
		return "Smart Sniper";
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

