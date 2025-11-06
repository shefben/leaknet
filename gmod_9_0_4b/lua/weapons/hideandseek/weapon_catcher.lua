--##############################
--## Hide and Seek
--## fayte, gdhughes@gmail.com
--##############################

MyIndex					= 0;
Owner 					= 0;
CurrentTime 				= 0;

function onInit()
	_SWEPSetSound(MyIndex, "single_shot", "Weapon_G3SG1.Single")
end
	
function onPrimaryAttack()
	_RunString("ShootPrimaryCatcher(" .. Owner .. ", " .. MyIndex .. ")")
end
	
function onSecondaryAttack() return false; end
function getPrintName() return "Catcher Gun"; end
function getClassName() return "catchergun"; end
function getPrimaryAmmoType() return "pistol"; end
function getSecondaryAmmoType() return "pistol"; end
function getViewModel() return "models/weapons/v_rif_sg552.mdl"; end
function getWorldModel() return "models/weapons/w_rif_sg552.mdl"; end
function getAnimPrefix() return "pistol"; end
function getWeaponSwapHands() return true; end
function getWeaponFOV() return 80; end
function getWeaponSlot() return 1; end
function getWeaponSlotPos() return 1; end
function getFiresUnderwater() return true; end
function getReloadsSingly() return false; end
function getDamage() return 0; end
function getPrimaryShotDelay() return 1; end
function getSecondaryShotDelay() return 1; end
function getPrimaryIsAutomatic() return false; end
function getSecondaryIsAutomatic() return false; end
function getMaxClipPrimary() return 1; end
function getMaxClipSecondary() return 0; end
function getDefClipPrimary() return 1; end
function getDefClipSecondary() return 0; end
function getPrimaryScriptOverride() return 1; end
function getSecondaryScriptOverride() return 3; end
function getBulletSpread() return vector3(0.0, 0.0, 0.0); end
function getViewKick() return vector3(0, 0.0, 0.0); end
function getViewKickRandom() return vector3(0.0, 0.0, 0.0); end
