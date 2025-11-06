

-- Called when the weapon is created.

	function onInit( )
		_SWEPSetSound( MyIndex, "single_shot", "Weapon_ump45.Single" );
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

		return 6;	

	end

	

	function getFiresUnderwater()

		return true;

	end

	

	function getReloadsSingly()

		return false;

	end

	

	function getDamage()

		return 12.5;

	end

	

	function getPrimaryShotDelay()

		return 0.12;

	end

	
	function getPrimaryIsAutomatic()

		return true;

	end

		

	function getBulletSpread()

		return vector3( 0.01, 0.01, 0.01 );

	end

	

	function getViewKick()

		return vector3( -1.0, 0.0, 0.0);

	end

	

	function getViewKickRandom()

		return vector3( 0.6, 0.6, 0.5 );

	end

	


	function getViewModel( )
		return "models/weapons/v_smg_ump45.mdl";
	end

	function getWorldModel( )
		return "models/weapons/w_smg_ump45.mdl";
	end

	

	function getClassName()

		return "weapon_ump45";

	end



	function getPrimaryAmmoType()

		return "pistol";

	end

		

	function getSecondaryAmmoType()

		return "pistol";

	end

	

	-- return -1 if it doesn't use clips

	function getMaxClipPrimary()

		return 30;

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

		return "ump45";

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
		return "d_ump45";
	end

