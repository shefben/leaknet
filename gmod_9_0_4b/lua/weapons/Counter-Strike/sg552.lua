


bZoomed = false;






-- Called when the weapon is created.

	function onInit( )
	
	_SWEPSetSound( MyIndex, "single_shot", "Weapon_sg552.Single" );

	end



	

-- Called every frame

	function onThink( )

	end

	

	

-- When the player presses left mouse button

	function onPrimaryAttack( )	

	end



-- When the player presses right mouse button

     function onSecondaryAttack( )          
          
	ToggleZoom();

     end
     

     function ToggleZoom ( )
          
          
          if (bZoomed ~= true) then
               
               _PlayerSetFOV ( Owner, 40, 0.1 );
               
               _GModRect_Start( "gmod/scope-refract" );
                _GModRect_SetPos( 0, 0, 1, 1 );
                _GModRect_SetColor( 0, 0, 0, 255 );
                _GModRect_SetTime( 9999, 0.2, 0.1 );
               _GModRect_Send( Owner, 124 );     
               
               _GModRect_Start( "gmod/scope" );
                _GModRect_SetPos( 0, 0, 1, 1 );
                _GModRect_SetColor( 0, 0, 0, 255 );
                _GModRect_SetTime( 9999, 0.1, 0.1 );
               _GModRect_Send( Owner, 125 );     
               
               bZoomed = true;
               
               -- Update the variables after changing the zoom to update the bullet spread
               _SWEPUpdateVariables( MyIndex );
               
          else
          
               EndZoom();
               
          end
          
          
          
     end


     function EndZoom ( )
          
          _PlayerStopZooming( Owner );
          _GModRect_Hide( Owner, 125, 0.1, 0.0 );     
          _GModRect_Hide( Owner, 124, 0.0, 0.0 );     
          bZoomed = false;
          _SWEPUpdateVariables( MyIndex );
          
     end
     

     function Deploy( )
          EndZoom();
     end
     
     function Holster( )
          EndZoom();
     end
     
     function onReload( )
          EndZoom();
          return true;
     end
	



-- These are only accessed once when setting up the weapon

	

	function getWeaponSwapHands()

		return true;	

	end

	

	function getWeaponFOV()

		return 70;	

	end

	

	function getWeaponSlot()

		return 5;	

	end

	

	function getWeaponSlotPos()

		return 4;	

	end

	

	function getFiresUnderwater()

		return false;

	end

	

	function getReloadsSingly()

		return false;

	end

	



	

	-- Primary Attack

	

	function getDamage()

		return 25;

	end

	

	function getPrimaryShotDelay()

		return 0.10;

	end

		

	function getPrimaryIsAutomatic()

		return true;

	end

			

	function getBulletSpread()

		return vector3( 0.01, 0.01, 0.01 );

	end

	

	function getViewKick()

		return vector3( 0.5, 0.0, 0.0);

	end

	

	function getViewKickRandom()

		return vector3( 0.5, 0.5, 0.2 );

	end

		

	function getNumShotsPrimary()

		return 1;

	end

	

	function getPrimaryAmmoType()

		return "pistol";

	end

	

	-- Secondary attack

	

	function getDamageSecondary()

		return 150;

	end

	

	function getSecondaryShotDelay()

		return 0.8;

	end

	

	function getSecondaryIsAutomatic()

		return true;

	end

	

	function getBulletSpreadSecondary()

		return vector3( 0.001, 0.001, 0.001 );

	end

	

	function getViewKickSecondary()

		return vector3( 0.5, 0.0, 0.0);

	end

	

	function getViewKickRandomSecondary()

		return vector3( 0.5, 0.5, 0.2 );

	end

	

	function getNumShotsSecondary()

		return 15;

	end

	

	function getSecondaryAmmoType()

		return "buckshot";

	end

	

	



	function getViewModel( )

		return "models/weapons/v_rif_sg552.mdl";

	end

	

	function getWorldModel( )

		return "models/weapons/w_rif_sg552.mdl";

	end

	

	function getClassName() 

		return "weapon_sg552";

	end



	function getHUDMaterial( )

		return "gmod/SWEP/default";

	end





		



	

	function getMaxClipPrimary() -- return -1 if it doesn't use clips

		return 30;

	end

	

	function getMaxClipSecondary() -- return -1 if it doesn't use clips

		return 5;

	end

	

	function getDefClipPrimary() -- ammo in gun by default

		return 128;

	end

	

	function getDefClipSecondary()

		return 1;

	end



	

	function getAnimPrefix() -- How the player holds the weapon: pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade

		return "smg";

	end



	function getPrintName()

		return "sg552";

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


	function getDeathIcon( )
		return "d_sg552";
	end
