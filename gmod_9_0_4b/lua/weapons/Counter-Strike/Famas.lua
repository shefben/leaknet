

-- Called when the weapon is created.

	function onInit( )
	
	_SWEPSetSound( MyIndex, "single_shot", "Weapon_famas.Single" );

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

		return false;	

	end

	

	function getWeaponFOV()

		return 70;	

	end

	

	function getWeaponSlot()

		return 3;	

	end

	

	function getWeaponSlotPos()

		return 3;	

	end

	

	function getFiresUnderwater()

		return false;

	end

	

	function getReloadsSingly()

		return false;

	end

	



	

	-- Primary Attack

	

	function getDamage()

		return 20;

	end

	

	function getPrimaryShotDelay()

		return 0.1;

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

		return 70;

	end

	

	function getSecondaryShotDelay()

		return 0.35;

	end

	

	function getSecondaryIsAutomatic()

		return false;

	end

	

	function getBulletSpreadSecondary()

		return vector3( 0.4, 0.4, 0.4 );

	end

	

	function getViewKickSecondary()

		return vector3( 0.0, 0.0, 3.0);

	end

	

	function getViewKickRandomSecondary()

		return vector3( -0.1, 0.1, 0.2 );

	end

	

	function getNumShotsSecondary()

		return 4;

	end

	

	function getSecondaryAmmoType()

		return "buckshot";

	end

	

	



	function getViewModel( )

		return "models/weapons/v_rif_famas.mdl";

	end

	

	function getWorldModel( )

		return "models/weapons/w_rif_famas.mdl";

	end

	

	function getClassName() 

		return "weapon_famas";

	end



	function getHUDMaterial( )

		return "gmod/SWEP/default";

	end





		



	

	function getMaxClipPrimary() -- return -1 if it doesn't use clips

		return 25;

	end

	

	function getMaxClipSecondary() -- return -1 if it doesn't use clips

		return 500000;

	end

	

	function getDefClipPrimary() -- ammo in gun by default

		return 128;

	end

	

	function getDefClipSecondary()

		return 128;

	end



	

	function getAnimPrefix() -- How the player holds the weapon: pistol, smg, ar2, shotgun, rpg, phys, crossbow, melee, slam, grenade

		return "smg";

	end



	function getPrintName()

		return "Famas";

	end

	

	

	-- 0 = Don't override, shoot bullets, make sound and flash

	-- 1 = Don't shoot bullets but do make flash/sounds

	-- 2 = Only play animations

	-- 3 = Don't do anything

	

	function getPrimaryScriptOverride()

		return 0;

	end



	function getSecondaryScriptOverride()

		return 0;

	end


	function getDeathIcon( )
		return "d_famas";
	end
