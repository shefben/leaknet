

-- Called when the weapon is created.

     function onInit( )
          _SWEPSetSound( MyIndex, "single_shot", "Weapon_m249.Single" );
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

          return 4;     

     end

     

     function getWeaponSlotPos()

          return 4;     

     end

     

     function getFiresUnderwater()

          return true;

     end

     

     function getReloadsSingly()

          return false;

     end

     

     function getDamage()

          return 20;

     end

     

     function getPrimaryShotDelay()

          return 0.05;

     end

     
     function getPrimaryIsAutomatic()

          return true;

     end

          

     function getBulletSpread()

          return vector3( 0.01, 0.01, 0.01 );

     end

     

     function getViewKick()

          return vector3( -1.3, -0.6, 0.2);

     end

     

     function getViewKickRandom()

          return vector3( -0.5, 2.3, 1.4 );

     end

     


     function getViewModel( )
          return "models/weapons/v_mach_m249para.mdl";
     end

     function getWorldModel( )
          return "models/weapons/w_mach_m249para.mdl";
     end

     

     function getClassName()

          return "weapon_para";

     end



     function getPrimaryAmmoType()

          return "pistol";

     end

          

     function getSecondaryAmmoType()

          return "pistol";

     end

     

     -- return -1 if it doesn't use clips

     function getMaxClipPrimary()

          return 200;

     end

     

     function getMaxClipSecondary()

          return -1;

     end

     

     -- ammo in gun by default

     function getDefClipPrimary()

          return 128;

     end

     

     function getDefClipSecondary()

          return -1;

     end



     -- pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade

     function getAnimPrefix()

          return "smg";

     end



     function getPrintName()

          return "m249";

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
		return "d_m249";
	end