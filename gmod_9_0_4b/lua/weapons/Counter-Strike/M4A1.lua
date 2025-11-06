
-- Called when the weapon is created.

	function onInit( )
	
	_SWEPSetSound( MyIndex, "single_shot", "Weapon_m4a1.Single" );

	end



	

-- Called every frame

	function onThink( )

	end

	

	

-- When the player presses left mouse button

	function onPrimaryAttack( )	

	end

	

-- When the player presses right mouse button

	function onSecondaryAttack( )		
  
	end



	function Deploy( )
	end
	
	function Holster( )
	end
	

-- When player presses reload. Returning false means DONT RELOAD. Although this will hitch on the client.

	function onReload( )

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

		return 3;	

	end

	

	function getWeaponSlotPos()

		return 5;	

	end

	

	function getFiresUnderwater()

		return false;

	end

	

	function getReloadsSingly()

		return false;

	end

	



	

	-- Primary Attack

	

	function getDamage()

		return 18;

	end

	

	function getPrimaryShotDelay()

		return 0.09;

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

		return 0.001;

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

		return "models/weapons/v_rif_m4a1.mdl";

	end

	

	function getWorldModel( )

		return "models/weapons/w_rif_m4a1.mdl";

	end

	

	function getClassName() 

		return "weapon_scripted";

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

		return "M4A1";

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
		return "d_m4a1";
	end
