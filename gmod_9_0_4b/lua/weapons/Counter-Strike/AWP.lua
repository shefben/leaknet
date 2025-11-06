


bZoomed = false;


-- Called when the weapon is created.

     

     function onInit( )
          _SWEPSetSound( MyIndex, "single_shot", "Weapon_AWP.Single" );
          
          _ForceFileConsistency( "gmod/scope-refract.vmf" );
          _ForceFileConsistency( "gmod/scope.vmf" );
     end


     function onThink( )
     end


     function onPrimaryAttack( )
     end

     
     function onSecondaryAttack( )          
          
          ToggleZoom();
          
     end

     
     function ToggleZoom ( )
          
          
          if (bZoomed ~= true) then
               
               _PlayerSetFOV ( Owner, 20, 0.1 );
               
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

     

     

     

-- Weapon settings.

-- These are only accessed once when setting the weapon up

     

     function getWeaponSwapHands()
          return true;     
     end

     

     function getWeaponFOV()

          return 74;     

     end

     

     function getWeaponSlot()

          return 5;     

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

          return 400;

     end

     

     function getPrimaryShotDelay()

          return 1;

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

          if (bZoomed) then
               return vector3( 0.0, 0.0, 0.0 );
          else
               return vector3( 0.3, 0.4, 0.4 );
          end

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
          return "models/weapons/v_snip_awp.mdl";
     end

     function getWorldModel( )
          return "models/weapons/w_snip_awp.mdl";
     end

     

     function getClassName()

          return "weapon_awp";

     end



     function getPrimaryAmmoType()

          return "357";

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

          return 64

     end

     

     function getDefClipSecondary()

          return -1;

     end



     -- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade

     function getAnimPrefix()

          return "rpg";

     end



     function getPrintName()

          return "AWP";

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
		return "d_awp";
	end